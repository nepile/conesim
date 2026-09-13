#include "core/Configuration.hpp"

#include <fstream>
#include <sstream>
#include <stdexcept>
#include <algorithm>
#include <cctype>
#include <cmath>
#include <filesystem>

namespace conesim {

ObjectFactory& ObjectFactory::instance() {
    static ObjectFactory factory;
    return factory;
}

void ObjectFactory::registerType(const std::string& className, DefaultCreator creator) {
    defaultRegistry[className] = std::move(creator);
}

void ObjectFactory::registerType(const std::string& className, ConfiguredCreator creator) {
    configuredRegistry[className] = std::move(creator);
}

bool ObjectFactory::hasType(const std::string& className) const {
    return defaultRegistry.find(className) != defaultRegistry.end() ||
           configuredRegistry.find(className) != configuredRegistry.end();
}

std::unordered_map<std::string, std::string> Configuration::properties;
int Configuration::runIndex = 0;
bool Configuration::isInitialized = false;

Configuration::Configuration(): namespaceName(""), secondaryNamespace("") {}

Configuration::Configuration(const std::string& namespaceName): namespaceName(namespaceName), secondaryNamespace("") {}

void Configuration::setRunIndex(int index) {
    runIndex = index;
}

int Configuration::getRunIndex() {
    return runIndex;
}

void Configuration::setNamespace(const std::string& name) {
    oldNamespaces.push(namespaceName);
    namespaceName = name;
}

void Configuration::restoreNamespace() {
    if (oldNamespaces.empty()) {
        throw std::runtime_error("No previous namespace to restore");
    }
    namespaceName = oldNamespaces.top();
    oldNamespaces.pop();
}

void Configuration::setSecondaryNamespace(const std::string& name) {
    oldSecondaryNamespaces.push(secondaryNamespace);
    secondaryNamespace = name;
}

void Configuration::restoreSecondaryNamespace() {
    if (oldSecondaryNamespaces.empty()) {
        throw std::runtime_error("No previous secondary namespace to restore");
    }
    secondaryNamespace = oldSecondaryNamespaces.top();
    oldSecondaryNamespaces.pop();
}

std::string Configuration::getFullConfigurationName(const std::string& name, bool useSecondary) const {
    const std::string& ns = useSecondary ? secondaryNamespace : namespaceName;
    if (ns.empty()) {
        return name;
    }
    return ns + "." + name;
}

std::string Configuration::getPropertyNamesString(const std::string& name) const {
    if (!secondaryNamespace.empty()) {
        return "'" + secondaryNamespace + "." + name + "' nor '" +
               (namespaceName.empty() ? name : namespaceName + "." + name) + "'";
    } else if (!namespaceName.empty()) {
        return "'" + namespaceName + "." + name + "'";
    } else {
        return "'" + name + "'";
    }
}

std::string Configuration::getFullPropertyName(const std::string& name) const {
    if (!contains(name)) {
        return "";
    }

    std::string primary = getFullConfigurationName(name, false);
    auto it = properties.find(primary);
    if (it != properties.end() && !parseRunSetting(it->second).empty()) {
        return primary;
    }

    return getFullConfigurationName(name, true);
}

bool Configuration::contains(const std::string& name) const {
    try {
        std::string val = getConfiguration(name);
        return !val.empty();
    } catch (const std::exception&) {
        return false;
    }
}

std::string Configuration::parseRunSetting(const std::string& rawValue) {
    std::string val = rawValue;

    std::size_t commentPos = val.find('#');
    if (commentPos != std::string::npos) {
        val = val.substr(0, commentPos);
    }

    auto trimStart = std::find_if(val.begin(), val.end(), [](unsigned char ch) {
        return !std::isspace(ch);
    });
    auto trimEnd = std::find_if(val.rbegin(), val.rend(), [](unsigned char ch) {
        return !std::isspace(ch);
    }).base();

    if (trimStart >= trimEnd) {
        return "";
    }
    val = std::string(trimStart, trimEnd);

    if (runIndex < 0 || val.size() < 3 || val.front() != '[' || val.back() != ']') {
        return val;
    }

    std::string content = val.substr(1, val.size() - 2);
    std::vector<std::string> elements;
    std::stringstream ss(content);
    std::string item;

    while (std::getline(ss, item, ';')) {
        auto itemStart = std::find_if(item.begin(), item.end(), [](unsigned char ch) {
            return !std::isspace(ch);
        });
        auto itemEnd = std::find_if(item.rbegin(), item.rend(), [](unsigned char ch) {
            return !std::isspace(ch);
        }).base();

        if (itemStart < itemEnd) {
            elements.emplace_back(itemStart, itemEnd);
        } else {
            elements.emplace_back("");
        }
    }

    if (elements.empty()) {
        return "";
    }

    int targetIdx = runIndex % static_cast<int>(elements.size());
    return elements[targetIdx];
}

std::string Configuration::getConfiguration(const std::string& name) const {
    if (!isInitialized) {
        init("");
    }

    std::string fullName = getFullConfigurationName(name, false);
    auto it = properties.find(fullName);
    std::string value;

    if (it != properties.end()) {
        value = parseRunSetting(it->second);
    }

    if (value.empty() && !secondaryNamespace.empty()) {
        fullName = getFullConfigurationName(name, true);
        it = properties.find(fullName);
        if (it != properties.end()) {
            value = parseRunSetting(it->second);
        }
    }

    if (value.empty()) {
        throw std::runtime_error("Can't find setting " + getPropertyNamesString(name));
    }

    return value;
}

std::string Configuration::getConfiguration(const std::string& name, const std::string& defaultValue) const {
    try {
        return getConfiguration(name);
    } catch (const std::exception&) {
        return defaultValue;
    }
}

std::optional<std::string> Configuration::getOptionalConfiguration(const std::string& name) const {
    try {
        return getConfiguration(name);
    } catch (const std::exception&) {
        return std::nullopt;
    }
}

double Configuration::parseDoubleValue(const std::string& value, const std::string& settingName) const {
    if (value.empty()) {
        throw std::runtime_error("Empty numeric value for setting: " + settingName);
    }

    double multiplier = 1.0;
    std::string cleanVal = value;
    char lastChar = cleanVal.back();

    if (lastChar == 'k') {
        multiplier = 1e3;
        cleanVal.pop_back();
    } else if (lastChar == 'M') {
        multiplier = 1e6;
        cleanVal.pop_back();
    } else if (lastChar == 'G') {
        multiplier = 1e9;
        cleanVal.pop_back();
    }

    try {
        std::size_t idx = 0;
        double parsed = std::stod(cleanVal, &idx);
        if (idx != cleanVal.size()) {
            throw std::invalid_argument("Trailing characters");
        }
        return parsed * multiplier;
    } catch (const std::exception& e) {
        throw std::runtime_error("Invalid numeric setting '" + value + "' for '" + settingName + "'\n" + e.what());
    }
}

int Configuration::convertToInt(double doubleValue, const std::string& settingName) const {
    double intPart = 0.0;
    if (std::modf(doubleValue, &intPart) != 0.0) {
        throw std::runtime_error("Expected integer value for setting '" + settingName + "' got '" + std::to_string(doubleValue) + "'");
    }
    return static_cast<int>(doubleValue);
}

double Configuration::getDouble(const std::string& name) const {
    return parseDoubleValue(getConfiguration(name), name);
}

double Configuration::getDouble(const std::string& name, double defaultValue) const {
    try {
        return getDouble(name);
    } catch (const std::exception&) {
        return defaultValue;
    }
}

std::optional<double> Configuration::getOptionalDouble(const std::string& name) const {
    try {
        return getDouble(name);
    } catch (const std::exception&) {
        return std::nullopt;
    }
}

int Configuration::getInt(const std::string& name) const {
    return convertToInt(getDouble(name), name);
}

int Configuration::getInt(const std::string& name, int defaultValue) const {
    try {
        return getInt(name);
    } catch (const std::exception&) {
        return defaultValue;
    }
}

std::optional<int> Configuration::getOptionalInt(const std::string& name) const {
    try {
        return getInt(name);
    } catch (const std::exception&) {
        return std::nullopt;
    }
}

bool Configuration::getBoolean(const std::string& name) const {
    std::string val = getConfiguration(name);
    std::string lowerVal = val;
    std::transform(lowerVal.begin(), lowerVal.end(), lowerVal.begin(), [](unsigned char c) {
        return std::tolower(c);
    });

    if (lowerVal == "true" || lowerVal == "1") {
        return true;
    }
    if (lowerVal == "false" || lowerVal == "0") {
        return false;
    }

    throw std::runtime_error("Not a boolean value: '" + val + "' for setting " + name);
}

bool Configuration::getBoolean(const std::string& name, bool defaultValue) const {
    try {
        return getBoolean(name);
    } catch (const std::exception&) {
        return defaultValue;
    }
}

std::optional<bool> Configuration::getOptionalBoolean(const std::string& name) const {
    try {
        return getBoolean(name);
    } catch (const std::exception&) {
        return std::nullopt;
    }
}

std::vector<std::string> Configuration::getCsvSetting(const std::string& name) const {
    std::string csv = getConfiguration(name);
    std::vector<std::string> values;
    std::stringstream ss(csv);
    std::string token;

    while (std::getline(ss, token, ',')) {
        auto start = std::find_if(token.begin(), token.end(), [](unsigned char ch) {
            return !std::isspace(ch);
        });
        auto end = std::find_if(token.rbegin(), token.rend(), [](unsigned char ch) {
            return !std::isspace(ch);
        }).base();

        if (start < end) {
            values.emplace_back(start, end);
        } else {
            values.emplace_back("");
        }
    }

    return values;
}

std::vector<std::string> Configuration::getCsvSetting(const std::string& name, std::size_t expectedCount) const {
    std::vector<std::string> values = getCsvSetting(name);
    if (values.size() != expectedCount) {
        throw std::runtime_error("Read unexpected amount (" + std::to_string(values.size()) +
                                 ") of comma separated values for setting '" + name +
                                 "' (expected " + std::to_string(expectedCount) + ")");
    }
    return values;
}

std::vector<double> Configuration::getCsvDoubles(const std::string& name) const {
    std::vector<std::string> tokens = getCsvSetting(name);
    std::vector<double> results;
    results.reserve(tokens.size());

    for (const auto& token : tokens) {
        results.push_back(parseDoubleValue(token, name));
    }
    return results;
}

std::vector<double> Configuration::getCsvDoubles(const std::string& name, std::size_t expectedCount) const {
    std::vector<std::string> tokens = getCsvSetting(name, expectedCount);
    std::vector<double> results;
    results.reserve(tokens.size());

    for (const auto& token : tokens) {
        results.push_back(parseDoubleValue(token, name));
    }
    return results;
}

std::vector<int> Configuration::getCsvInts(const std::string& name) const {
    std::vector<double> doubleVals = getCsvDoubles(name);
    std::vector<int> results;
    results.reserve(doubleVals.size());

    for (double val : doubleVals) {
        results.push_back(convertToInt(val, name));
    }
    return results;
}

std::vector<int> Configuration::getCsvInts(const std::string& name, std::size_t expectedCount) const {
    std::vector<double> doubleVals = getCsvDoubles(name, expectedCount);
    std::vector<int> results;
    results.reserve(doubleVals.size());

    for (double val : doubleVals) {
        results.push_back(convertToInt(val, name));
    }
    return results;
}

void Configuration::assertValidRange(const std::vector<int>& range, const std::string& name) const {
    if (range.size() != 2) {
        throw std::runtime_error("Range setting " + getFullPropertyName(name) +
                                 " should contain only two comma separated integer values");
    }
    if (range[0] > range[1]) {
        throw std::runtime_error("Range setting's " + getFullPropertyName(name) +
                                 " first value should be smaller or equal to second value");
    }
}

std::string Configuration::valueFillString(const std::string& input) const {
    if (input.find(FILL_DELIMITER) == std::string::npos) {
        return input;
    }

    Configuration globalConfig;
    std::string result;
    std::size_t pos = 0;

    while (pos < input.size()) {
        std::size_t start = input.find(FILL_DELIMITER, pos);
        if (start == std::string::npos) {
            result += input.substr(pos);
            break;
        }

        result += input.substr(pos, start - pos);
        std::size_t keyStart = start + FILL_DELIMITER.size();
        std::size_t end = input.find(FILL_DELIMITER, keyStart);

        if (end == std::string::npos) {
            result += input.substr(start);
            break;
        }

        std::string settingKey = input.substr(keyStart, end - keyStart);
        result += globalConfig.getConfiguration(settingKey);
        pos = end + FILL_DELIMITER.size();
    }

    return result;
}

void Configuration::addSetting(const std::string& name, const std::string& value) {
    properties[name] = value;
}

void Configuration::addSettings(const std::string& propFile) {
    loadFile(propFile);
}

void Configuration::loadFile(const std::string& filename) {
    std::ifstream file(filename);

    if (!file.is_open()) {
        throw std::runtime_error("Failed to open configuration file: " + filename);
    }

    std::string line;

    while (std::getline(file, line)) {
        line.erase(line.begin(), std::find_if(line.begin(), line.end(), [](unsigned char ch) {
            return !std::isspace(ch);
        }));

        if (line.empty() || line[0] == '#') {
            continue;
        }

        const std::size_t delimiter = line.find('=');
        if (delimiter == std::string::npos) {
            continue;
        }

        std::string key = line.substr(0, delimiter);
        std::string value = line.substr(delimiter + 1);

        key.erase(key.begin(), std::find_if(key.begin(), key.end(), [](unsigned char ch) {
            return !std::isspace(ch);
        }));
        key.erase(std::find_if(key.rbegin(), key.rend(), [](unsigned char ch) {
            return !std::isspace(ch);
        }).base(), key.end());

        value.erase(value.begin(), std::find_if(value.begin(), value.end(), [](unsigned char ch) {
            return !std::isspace(ch);
        }));
        value.erase(std::find_if(value.rbegin(), value.rend(), [](unsigned char ch) {
            return !std::isspace(ch);
        }).base(), value.end());

        properties[key] = value;
    }
}

void Configuration::init(const std::string& propFile) {
    properties.clear();
    isInitialized = true;

    if (std::filesystem::exists(DEF_SETTINGS_FILE)) {
        loadFile(DEF_SETTINGS_FILE);
    }

    if (!propFile.empty()) {
        loadFile(propFile);
    }
}

}