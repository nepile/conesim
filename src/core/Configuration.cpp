/**
 * @file Configuration.cpp
 * @brief Implementation of runtime configuration management and dynamic object factories.
 * @details Handles multi-run parameter interpolation, namespacing, unit parsing,
 *          and configuration properties parsing for the conesim simulation engine.
 * @author Neville
 * @date September, 2026
 */

#include "core/Configuration.hpp"
#include "core/ConfigurationError.hpp"

#include <fstream>
#include <sstream>
#include <algorithm>
#include <cctype>
#include <cmath>
#include <filesystem>

namespace conesim {

// ============================================================================
// ObjectFactory Implementation
// ============================================================================

/**
 * @brief Retrieves the singleton instance of the ObjectFactory.
 * @return Reference to the thread-safe global ObjectFactory instance.
 */
ObjectFactory& ObjectFactory::instance() {
    static ObjectFactory factory;
    return factory;
}

/**
 * @brief Registers a default construction callback for a given type name.
 * @param className Identifier string representing the class type.
 * @param creator Callable factory returning a newly constructed instance.
 */
void ObjectFactory::registerType(const std::string& className, DefaultCreator creator) {
    defaultRegistry[className] = std::move(creator);
}

/**
 * @brief Registers a configuration-aware construction callback for a given type name.
 * @param className Identifier string representing the class type.
 * @param creator Callable factory accepting a Configuration reference.
 */
void ObjectFactory::registerType(const std::string& className, ConfiguredCreator creator) {
    configuredRegistry[className] = std::move(creator);
}

/**
 * @brief Verifies whether a given type identifier is registered.
 * @param className Name of the type to query.
 * @return true if registered under either default or configured registries; false otherwise.
 */
bool ObjectFactory::hasType(const std::string& className) const {
    return defaultRegistry.find(className) != defaultRegistry.end() ||
           configuredRegistry.find(className) != configuredRegistry.end();
}

// ============================================================================
// Configuration Static Members Initialization
// ============================================================================

std::unordered_map<std::string, std::string> Configuration::properties;
int Configuration::runIndex = 0;
bool Configuration::isInitialized = false;

std::ostream* Configuration::outStream = nullptr;
std::unique_ptr<std::ofstream> Configuration::fileStream = nullptr;
std::unordered_set<std::string> Configuration::writtenSettings;

// ============================================================================
// Configuration Constructors & Lifecycle
// ============================================================================

/**
 * @brief Constructs an empty Configuration context with default namespaces.
 */
Configuration::Configuration(): namespaceName(""), secondaryNamespace("") {}

/**
 * @brief Constructs a Configuration context bound to a primary namespace prefix.
 * @param namespaceName Scope prefix prepended to queried setting keys.
 */
Configuration::Configuration(const std::string& namespaceName): namespaceName(namespaceName), secondaryNamespace("") {}

/**
 * @brief Sets the global simulation execution run index.
 * @details Resets the record of written settings for tracking active values.
 * @param index Non-negative run index used for parameter array resolution.
 */
void Configuration::setRunIndex(int index) {
    runIndex = index;
    writtenSettings.clear();
}

/**
 * @brief Retrieves the current active simulation run index.
 * @return Integer representing current run index.
 */
int Configuration::getRunIndex() {
    return runIndex;
}

/**
 * @brief Flushes an accessed setting to the designated logging/output stream.
 * @param setting Formatted key-value string to output.
 */
void Configuration::outputSetting(const std::string& setting) {
    if (outStream != nullptr && writtenSettings.find(setting) == writtenSettings.end()) {
        if (writtenSettings.empty()) {
            *outStream << "# Settings for run " << (runIndex + 1) << "\n";
        }
        *outStream << setting << "\n";
        writtenSettings.insert(setting);
    }
}

// ============================================================================
// Namespace Management
// ============================================================================

/**
 * @brief Pushes current primary namespace to the history stack and activates a new one.
 * @param name New namespace prefix to adopt.
 */
void Configuration::setNamespace(const std::string& name) {
    oldNamespaces.push(namespaceName);
    namespaceName = name;
}

/**
 * @brief Restores previous primary namespace from the history stack.
 * @throws ConfigurationError If namespace stack is empty.
 */
void Configuration::restoreNamespace() {
    if (oldNamespaces.empty()) {
        throw ConfigurationError("No previous namespace to restore");
    }
    namespaceName = oldNamespaces.top();
    oldNamespaces.pop();
}

/**
 * @brief Pushes current fallback namespace to the history stack and activates a new one.
 * @param name New secondary namespace prefix.
 */
void Configuration::setSecondaryNamespace(const std::string& name) {
    oldSecondaryNamespaces.push(secondaryNamespace);
    secondaryNamespace = name;
}

/**
 * @brief Restores previous fallback secondary namespace from the history stack.
 * @throws ConfigurationError If secondary namespace stack is empty.
 */
void Configuration::restoreSecondaryNamespace() {
    if (oldSecondaryNamespaces.empty()) {
        throw ConfigurationError("No previous secondary namespace to restore");
    }
    secondaryNamespace = oldSecondaryNamespaces.top();
    oldSecondaryNamespaces.pop();
}

/**
 * @brief Computes fully qualified property identifier using selected namespace.
 * @param name Relative property key.
 * @param useSecondary Flag selecting secondary namespace instead of primary.
 * @return Fully qualified dot-separated key.
 */
std::string Configuration::getFullConfigurationName(const std::string& name, bool useSecondary) const {
    const std::string& ns = useSecondary ? secondaryNamespace : namespaceName;
    if (ns.empty()) {
        return name;
    }
    return ns + "." + name;
}

/**
 * @brief Formats readable candidates list for missing configuration error reporting.
 * @param name Base setting key.
 * @return Quoted diagnostic representation of keys searched.
 */
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

/**
 * @brief Resolves active fully qualified key that contains a valid configured value.
 * @param name Setting key.
 * @return Matched full key name, or empty string if not found.
 */
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

/**
 * @brief Checks existence and non-empty resolution of a given setting key.
 * @param name Base setting key name.
 * @return true if setting exists and is resolvable; false otherwise.
 */
bool Configuration::contains(const std::string& name) const {
    try {
        std::string val = getConfiguration(name);
        return !val.empty();
    } catch (const ConfigurationError&) {
        return false;
    }
}

// ============================================================================
// Value Parsing & Retrieval
// ============================================================================

/**
 * @brief Strips comments, trims whitespace, and resolves index arrays.
 * @details Evaluates array expressions of form `[opt0; opt1; ...]` using
 *          modulo arithmetic against `runIndex`.
 * @param rawValue Raw unparsed string value from properties table.
 * @return Sanitized string element for active run index.
 */
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

/**
 * @brief Resolves raw string setting value via primary and secondary namespaces.
 * @param name Setting key.
 * @return Active resolved string configuration value.
 * @throws ConfigurationError If property key cannot be found in configured registries.
 */
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
        throw ConfigurationError("Can't find setting " + getPropertyNamesString(name));
    }

    outputSetting(fullName + " = " + value);
    return value;
}

/**
 * @brief Retrieves setting with fallback default value if resolution fails.
 * @param name Setting key.
 * @param defaultValue Fallback string to return if missing.
 * @return Resolved configuration string or defaultValue.
 */
std::string Configuration::getConfiguration(const std::string& name, const std::string& defaultValue) const {
    try {
        return getConfiguration(name);
    } catch (const ConfigurationError&) {
        return defaultValue;
    }
}

/**
 * @brief Retrieves setting wrapped in std::optional.
 * @param name Setting key.
 * @return Optional containing resolved value or std::nullopt.
 */
std::optional<std::string> Configuration::getOptionalConfiguration(const std::string& name) const {
    try {
        return getConfiguration(name);
    } catch (const ConfigurationError&) {
        return std::nullopt;
    }
}

/**
 * @brief Parses numeric string containing scientific multipliers ('k', 'M', 'G').
 * @param value String containing numeric token.
 * @param settingName Key identifier used for diagnostic error context.
 * @return Scaled double floating-point value.
 * @throws ConfigurationError If value is empty or contains malformed characters.
 */
double Configuration::parseDoubleValue(const std::string& value, const std::string& settingName) const {
    if (value.empty()) {
        throw ConfigurationError("Empty numeric value for setting: " + settingName);
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
            throw ConfigurationError("Trailing characters in numeric value: " + cleanVal);
        }
        return parsed * multiplier;
    } catch (const std::exception& e) {
        throw ConfigurationError("Invalid numeric setting '" + value + "' for '" + settingName + "'", e);
    }
}

/**
 * @brief Enforces integer fidelity by verifying double contains zero fractional part.
 * @param doubleValue Floating-point number to cast.
 * @param settingName Context setting key for exception reporting.
 * @return Cast integer representation.
 * @throws ConfigurationError If fractional part is non-zero.
 */
int Configuration::convertToInt(double doubleValue, const std::string& settingName) const {
    double intPart = 0.0;
    if (std::modf(doubleValue, &intPart) != 0.0) {
        throw ConfigurationError("Expected integer value for setting '" + settingName + "' got '" + std::to_string(doubleValue) + "'");
    }
    return static_cast<int>(doubleValue);
}

/**
 * @brief Resolves configuration setting parsed as double.
 * @param name Setting key.
 * @return Numerical value as double.
 */
double Configuration::getDouble(const std::string& name) const {
    return parseDoubleValue(getConfiguration(name), name);
}

/**
 * @brief Resolves configuration setting as double with default fallback.
 * @param name Setting key.
 * @param defaultValue Fallback numeric value.
 * @return Resolved double or defaultValue.
 */
double Configuration::getDouble(const std::string& name, double defaultValue) const {
    try {
        return getDouble(name);
    } catch (const ConfigurationError&) {
        return defaultValue;
    }
}

/**
 * @brief Retrieves optional double setting.
 * @param name Setting key.
 * @return Optional double or std::nullopt.
 */
std::optional<double> Configuration::getOptionalDouble(const std::string& name) const {
    try {
        return getDouble(name);
    } catch (const ConfigurationError&) {
        return std::nullopt;
    }
}

/**
 * @brief Resolves configuration setting cast to integer.
 * @param name Setting key.
 * @return Integer representation of setting.
 */
int Configuration::getInt(const std::string& name) const {
    return convertToInt(getDouble(name), name);
}

/**
 * @brief Resolves configuration setting as integer with default fallback.
 * @param name Setting key.
 * @param defaultValue Fallback int value.
 * @return Resolved integer or defaultValue.
 */
int Configuration::getInt(const std::string& name, int defaultValue) const {
    try {
        return getInt(name);
    } catch (const ConfigurationError&) {
        return defaultValue;
    }
}

/**
 * @brief Retrieves optional integer setting.
 * @param name Setting key.
 * @return Optional integer or std::nullopt.
 */
std::optional<int> Configuration::getOptionalInt(const std::string& name) const {
    try {
        return getInt(name);
    } catch (const ConfigurationError&) {
        return std::nullopt;
    }
}

/**
 * @brief Resolves configuration setting parsed as boolean flag.
 * @param name Setting key.
 * @return Boolean evaluation ("true"/"1" vs "false"/"0").
 * @throws ConfigurationError If value cannot be mapped to boolean.
 */
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

    throw ConfigurationError("Not a boolean value: '" + val + "' for setting " + name);
}

/**
 * @brief Resolves boolean setting with default fallback.
 * @param name Setting key.
 * @param defaultValue Fallback boolean value.
 * @return Resolved boolean or defaultValue.
 */
bool Configuration::getBoolean(const std::string& name, bool defaultValue) const {
    try {
        return getBoolean(name);
    } catch (const ConfigurationError&) {
        return defaultValue;
    }
}

/**
 * @brief Retrieves optional boolean setting.
 * @param name Setting key.
 * @return Optional boolean or std::nullopt.
 */
std::optional<bool> Configuration::getOptionalBoolean(const std::string& name) const {
    try {
        return getBoolean(name);
    } catch (const ConfigurationError&) {
        return std::nullopt;
    }
}

// ============================================================================
// CSV Parsing Methods
// ============================================================================

/**
 * @brief Parses comma-separated string property into list of trimmed strings.
 * @param name Setting key.
 * @return Vector of string tokens.
 */
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

/**
 * @brief Parses CSV tokens and enforces strict element count constraint.
 * @param name Setting key.
 * @param expectedCount Required element quantity.
 * @return Vector of trimmed string tokens.
 * @throws ConfigurationError If actual size does not match expectedCount.
 */
std::vector<std::string> Configuration::getCsvSetting(const std::string& name, std::size_t expectedCount) const {
    std::vector<std::string> values = getCsvSetting(name);
    if (values.size() != expectedCount) {
        throw ConfigurationError("Read unexpected amount (" + std::to_string(values.size()) +
                            ") of comma separated values for setting '" + name +
                            "' (expected " + std::to_string(expectedCount) + ")");
    }
    return values;
}

/**
 * @brief Parses CSV string into vector of double-precision floating-point numbers.
 * @param name Setting key.
 * @return Vector of parsed double values.
 */
std::vector<double> Configuration::getCsvDoubles(const std::string& name) const {
    std::vector<std::string> tokens = getCsvSetting(name);
    std::vector<double> results;
    results.reserve(tokens.size());

    for (const auto& token : tokens) {
        results.push_back(parseDoubleValue(token, name));
    }
    return results;
}

/**
 * @brief Parses CSV string into vector of doubles and enforces expected element count.
 * @param name Setting key.
 * @param expectedCount Expected token count.
 * @return Vector of parsed double values.
 */
std::vector<double> Configuration::getCsvDoubles(const std::string& name, std::size_t expectedCount) const {
    std::vector<std::string> tokens = getCsvSetting(name, expectedCount);
    std::vector<double> results;
    results.reserve(tokens.size());

    for (const auto& token : tokens) {
        results.push_back(parseDoubleValue(token, name));
    }
    return results;
}

/**
 * @brief Parses CSV string into vector of integer values.
 * @param name Setting key.
 * @return Vector of verified integer values.
 */
std::vector<int> Configuration::getCsvInts(const std::string& name) const {
    std::vector<double> doubleVals = getCsvDoubles(name);
    std::vector<int> results;
    results.reserve(doubleVals.size());

    for (double val : doubleVals) {
        results.push_back(convertToInt(val, name));
    }
    return results;
}

/**
 * @brief Parses CSV string into vector of integers and validates total element count.
 * @param name Setting key.
 * @param expectedCount Expected token count.
 * @return Vector of verified integer values.
 */
std::vector<int> Configuration::getCsvInts(const std::string& name, std::size_t expectedCount) const {
    std::vector<double> doubleVals = getCsvDoubles(name, expectedCount);
    std::vector<int> results;
    results.reserve(doubleVals.size());

    for (double val : doubleVals) {
        results.push_back(convertToInt(val, name));
    }
    return results;
}

/**
 * @brief Validates that two-element array satisfies [min, max] range constraints.
 * @param range Vector of integers representing lower and upper bounds.
 * @param name Diagnostic setting name.
 * @throws ConfigurationError If size != 2 or lower bound exceeds upper bound.
 */
void Configuration::assertValidRange(const std::vector<int>& range, const std::string& name) const {
    if (range.size() != 2) {
        throw ConfigurationError("Range setting " + getFullPropertyName(name) +
                            " should contain only two comma separated integer values");
    }
    if (range[0] > range[1]) {
        throw ConfigurationError("Range setting's " + getFullPropertyName(name) +
                            " first value should be smaller or equal to second value");
    }
}

/**
 * @brief Interpolates configuration references delimited by FILL_DELIMITER in target template string.
 * @param input String with embedded dynamic variable tokens.
 * @return Substituted string containing resolved configuration properties.
 */
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

/**
 * @brief Serializes all loaded configuration pairs into a debug string.
 * @return Formatted string containing `{key1=val1, key2=val2}`.
 */
std::string Configuration::toString() const {
    std::stringstream ss;
    ss << "{";
    bool first = true;
    for (const auto& [key, val] : properties) {
        if (!first) {
            ss << ", ";
        }
        ss << key << "=" << val;
        first = false;
    }
    ss << "}";
    return ss.str();
}

/**
 * @brief Injects or updates an explicit setting pair directly in properties registry.
 * @param name Key name.
 * @param value Plain value string.
 */
void Configuration::addSetting(const std::string& name, const std::string& value) {
    properties[name] = value;
}

/**
 * @brief Loads additional configuration file without clearing current state.
 * @param propFile Filesystem path to properties file.
 */
void Configuration::addSettings(const std::string& propFile) {
    loadFile(propFile);
}

/**
 * @brief Parses a key-value properties text file and populates configuration map.
 * @param filename Filesystem path of properties file.
 * @throws ConfigurationError If file cannot be opened.
 */
void Configuration::loadFile(const std::string& filename) {
    std::ifstream file(filename);

    if (!file.is_open()) {
        throw ConfigurationError("Failed to open configuration file: " + filename);
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

/**
 * @brief Initializes global Configuration state, resetting previous variables.
 * @details Reads default settings (`DEF_SETTINGS_FILE`) if present, applies overrides
 *          from `propFile`, and sets up output stream logging (`SETTING_OUTPUT_S`).
 * @param propFile Path to initial properties file, or empty string to use defaults.
 * @throws ConfigurationError If specified output log file cannot be opened.
 */
void Configuration::init(const std::string& propFile) {
    properties.clear();
    writtenSettings.clear();
    outStream = nullptr;
    fileStream.reset();
    isInitialized = true;

    if (std::filesystem::exists(DEF_SETTINGS_FILE)) {
        loadFile(DEF_SETTINGS_FILE);
    }

    if (!propFile.empty()) {
        loadFile(propFile);
    }

    auto it = properties.find(SETTING_OUTPUT_S);
    if (it != properties.end()) {
        std::string outFile = it->second;
        auto start = std::find_if(outFile.begin(), outFile.end(), [](unsigned char ch) {
            return !std::isspace(ch);
        });
        auto end = std::find_if(outFile.rbegin(), outFile.rend(), [](unsigned char ch) {
            return !std::isspace(ch);
        }).base();

        if (start >= end) {
            outStream = &std::cout;
        } else {
            std::string cleanPath(start, end);
            fileStream = std::make_unique<std::ofstream>(cleanPath);
            if (!fileStream->is_open()) {
                throw ConfigurationError("Can't open Settings output file: " + cleanPath);
            }
            outStream = fileStream.get();
        }
    }
}

} // namespace conesim