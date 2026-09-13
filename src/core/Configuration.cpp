#include "core/Configuration.hpp"

#include <fstream>
#include <sstream>
#include <stdexcept>
#include <algorithm>
#include <cctype>
#include <cmath>

namespace conesim {

std::unordered_map<std::string, std::string> Configuration::properties;

Configuration::Configuration(): namespaceName(""), secondaryNamespace("") {}

Configuration::Configuration(const std::string& namespaceName): namespaceName(namespaceName), secondaryNamespace("") {}

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

std::string Configuration::getFullPropertyName(const std::string& name) const {
    std::string primary = getFullConfigurationName(name, false);
    if (properties.find(primary) != properties.end()) {
        return primary;
    }
    if (!secondaryNamespace.empty()) {
        std::string secondary = getFullConfigurationName(name, true);
        if (properties.find(secondary) != properties.end()) {
            return secondary;
        }
    }
    return "";
}

bool Configuration::contains(const std::string& name) const {
    try {
        std::string val = getConfiguration(name);
        return !val.empty();
    } catch (const std::exception&) {
        return false;
    }
}

std::string Configuration::getConfiguration(const std::string& name) const {
    std::string fullName = getFullConfigurationName(name, false);
    auto it = properties.find(fullName);

    if (it == properties.end() && !secondaryNamespace.empty()) {
        fullName = getFullConfigurationName(name, true);
        it = properties.find(fullName);
    }

    if (it == properties.end()) {
        throw std::runtime_error("Configuration not found: '" + name + "'");
    }

    return it->second;
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
    } catch (const std::exception&) {
        throw std::runtime_error("Invalid numeric setting '" + value + "' for '" + settingName + "'");
    }
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
    double dVal = getDouble(name);
    double intPart = 0.0;
    if (std::modf(dVal, &intPart) != 0.0) {
        throw std::runtime_error("Expected integer value for setting '" + name + "' got '" + std::to_string(dVal) + "'");
    }
    return static_cast<int>(dVal);
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

    loadFile("settings/default_settings.cfg");

    if (!propFile.empty()) {
        loadFile(propFile);
    }
}

}