#pragma once

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <stack>
#include <optional>
#include <memory>
#include <functional>
#include <stdexcept>
#include <iostream>
#include <fstream>

namespace core {

class Configuration;

/**
 * @brief Runtime factory for creating objects by class name.
 *
 * ObjectFactory provides a registry-based mechanism for constructing
 * objects dynamically from their string class names. It supports both
 * default construction and construction using a Configuration object.
 *
 * This class is implemented as a singleton because the type registry
 * represents a global registry shared by the simulation.
 *
 * @note
 * This implementation uses std::shared_ptr<void> internally to allow
 * different object types to coexist in the same registry.
 *
 * @author Neville
 * @date September 2026
 *
 * @section rewrite Neville — September 2026
 * Documentation rewritten and standardized for Doxygen-compatible
 * C++ documentation.
 */
class ObjectFactory {
public:
    /**
     * @brief Creator function for default object construction.
     *
     * The creator takes no arguments and returns a newly constructed
     * object wrapped in a std::shared_ptr<void>.
     */
    using DefaultCreator = std::function<std::shared_ptr<void>()>;

    /**
     * @brief Creator function for configuration-based construction.
     *
     * The creator receives a Configuration object and uses it to
     * initialize the newly created object.
     */
    using ConfiguredCreator =
        std::function<std::shared_ptr<void>(const Configuration&)>;

    /**
     * @brief Returns the global ObjectFactory instance.
     *
     * @return Reference to the singleton ObjectFactory instance.
     */
    static ObjectFactory& instance();

    /**
     * @brief Registers a class for default construction.
     *
     * @param className String identifier used to refer to the class.
     * @param creator Function responsible for creating the object.
     *
     * @throws std::runtime_error If the registration conflicts with
     *         an existing registration, depending on the implementation.
     */
    void registerType(
        const std::string& className,
        DefaultCreator creator
    );

    /**
     * @brief Registers a class for configuration-based construction.
     *
     * @param className String identifier used to refer to the class.
     * @param creator Function that constructs the object from a Configuration.
     */
    void registerType(
        const std::string& className,
        ConfiguredCreator creator
    );

    /**
     * @brief Checks whether a class name is registered.
     *
     * @param className Name of the class to look up.
     * @return true if the class is registered; false otherwise.
     */
    bool hasType(const std::string& className) const;

    /**
     * @brief Creates an object using its registered default constructor.
     *
     * @tparam T Expected object type.
     *
     * @param className Registered class name.
     *
     * @return Shared pointer to the newly created object.
     *
     * @throws std::runtime_error If the class is not registered for
     *         default construction.
     */
    template <typename T>
    std::shared_ptr<T> create(const std::string& className) const {
        auto it = defaultRegistry.find(className);

        if (it == defaultRegistry.end()) {
            throw std::runtime_error(
                "Class not registered for default construction: '" +
                className + "'"
            );
        }

        return std::static_pointer_cast<T>(it->second());
    }

    /**
     * @brief Creates an object using a Configuration object.
     *
     * @tparam T Expected object type.
     *
     * @param className Registered class name.
     * @param config Configuration used to initialize the object.
     *
     * @return Shared pointer to the newly created object.
     *
     * @throws std::runtime_error If the class is not registered for
     *         configuration-based construction.
     */
    template <typename T>
    std::shared_ptr<T> create(
        const std::string& className,
        const Configuration& config
    ) const {
        auto it = configuredRegistry.find(className);

        if (it == configuredRegistry.end()) {
            throw std::runtime_error(
                "Class not registered for configured construction: '" +
                className + "'"
            );
        }

        return std::static_pointer_cast<T>(it->second(config));
    }

private:
    /**
     * @brief Registry of classes supporting default construction.
     *
     * The key is the class name and the value is the corresponding
     * object creation function.
     */
    std::unordered_map<std::string, DefaultCreator> defaultRegistry;

    /**
     * @brief Registry of classes supporting configuration-based construction.
     *
     * The key is the class name and the value is the corresponding
     * configuration-aware creation function.
     */
    std::unordered_map<std::string, ConfiguredCreator> configuredRegistry;
};


/**
 * @brief Automatically registers a class with ObjectFactory.
 *
 * AutoRegister is intended to be instantiated as a static object.
 * Its constructor performs the registration during program initialization.
 *
 * @tparam T Class type to register.
 *
 * @section rewrite Neville — September 2026
 * Documentation rewritten by Neville.
 */
template <typename T>
struct AutoRegister {

    /**
     * @brief Registers a class for default construction.
     *
     * @param className Name used to identify the class in ObjectFactory.
     */
    explicit AutoRegister(const std::string& className) {
        ObjectFactory::instance().registerType(
            className,
            []() -> std::shared_ptr<void> {
                return std::make_shared<T>();
            }
        );
    }

    /**
     * @brief Registers a class for configuration-based construction.
     *
     * Registration only occurs when @p isConfigured is true.
     *
     * @param className Name used to identify the class.
     * @param isConfigured Determines whether configuration-based
     *        registration should be performed.
     */
    AutoRegister(
        const std::string& className,
        bool isConfigured
    ) {
        if (isConfigured) {
            ObjectFactory::instance().registerType(
                className,
                [](const Configuration& cfg) -> std::shared_ptr<void> {
                    return std::make_shared<T>(cfg);
                }
            );
        }
    }
};


/**
 * @brief Registers a class for default construction.
 *
 * This macro creates a static AutoRegister instance whose constructor
 * registers the specified class with ObjectFactory.
 *
 * @param ClassName Class to register.
 *
 * @code
 * REGISTER_TYPE(MyClass)
 * @endcode
 *
 * @section rewrite Neville — September 2026
 */
#define REGISTER_TYPE(ClassName) \
    static conesim::AutoRegister<ClassName> _reg_##ClassName(#ClassName);


/**
 * @brief Registers a class for Configuration-based construction.
 *
 * This macro creates a static AutoRegister instance configured to
 * construct objects using a Configuration object.
 *
 * @param ClassName Class to register.
 *
 * @code
 * REGISTER_CONFIGURABLE_TYPE(MyClass)
 * @endcode
 *
 * @section rewrite Neville — September 2026
 */
#define REGISTER_CONFIGURABLE_TYPE(ClassName) \
    static conesim::AutoRegister<ClassName> _reg_conf_##ClassName(#ClassName, true);


/**
 * @brief Global configuration manager for ConeSim.
 *
 * Configuration provides access to simulation settings loaded from
 * configuration files. It supports:
 *
 * - Namespaced configuration properties.
 * - Secondary namespaces.
 * - Integer, floating-point, boolean, and string values.
 * - Optional configuration values.
 * - CSV-based configuration values.
 * - Configuration value validation.
 * - Run-index substitution.
 * - Configuration output and tracking.
 * - Dynamic object creation through ObjectFactory.
 *
 * The class contains static state for properties shared across the
 * simulation and instance state for the currently active namespaces.
 *
 * @section rewrite Neville — September 2026
 * Documentation rewritten and standardized by Neville.
 */
class Configuration {
public:

    /**
     * @brief Default configuration file path.
     */
    static inline const std::string DEF_SETTINGS_FILE =
        "settings/default_settings.cfg";

    /**
     * @brief Name of the configuration setting used for output.
     */
    static inline const std::string SETTING_OUTPUT_S =
        "Settings.output";

    /**
     * @brief Placeholder delimiter used during value substitution.
     */
    static inline const std::string FILL_DELIMITER = "%%";

private:

    /**
     * @brief Global map containing configuration properties.
     *
     * Keys represent fully qualified configuration names while values
     * contain their corresponding configuration values.
     */
    static std::unordered_map<std::string, std::string> properties;

    /**
     * @brief Index of the current simulation run.
     *
     * The run index can be used when resolving run-dependent
     * configuration values.
     */
    static int runIndex;

    /**
     * @brief Indicates whether the global configuration has been initialized.
     */
    static bool isInitialized;

    /**
     * @brief Output stream used for configuration logging.
     */
    static std::ostream* outStream;

    /**
     * @brief File stream used when configuration output is written to a file.
     */
    static std::unique_ptr<std::ofstream> fileStream;

    /**
     * @brief Tracks configuration settings that have already been written.
     *
     * This prevents the same setting from being emitted multiple times
     * during configuration output.
     */
    static std::unordered_set<std::string> writtenSettings;

    /**
     * @brief Primary namespace currently associated with this configuration.
     */
    std::string namespaceName;

    /**
     * @brief Secondary namespace currently associated with this configuration.
     */
    std::string secondaryNamespace;

    /**
     * @brief Stack containing previously active primary namespaces.
     *
     * Used by setNamespace() and restoreNamespace() to temporarily
     * change the active namespace.
     */
    std::stack<std::string> oldNamespaces;

    /**
     * @brief Stack containing previously active secondary namespaces.
     *
     * Used by setSecondaryNamespace() and restoreSecondaryNamespace()
     * to temporarily change the active secondary namespace.
     */
    std::stack<std::string> oldSecondaryNamespaces;

    /**
     * @brief Builds the fully qualified name of a configuration property.
     *
     * @param name Local property name.
     * @param useSecondary Whether the secondary namespace should be used.
     *
     * @return Fully qualified configuration property name.
     */
    std::string getFullConfigurationName(
        const std::string& name,
        bool useSecondary = false
    ) const;

    /**
     * @brief Builds a string representation of matching property names.
     *
     * @param name Configuration property name or prefix.
     *
     * @return String containing matching property names.
     */
    std::string getPropertyNamesString(
        const std::string& name
    ) const;

    /**
     * @brief Loads configuration properties from a file.
     *
     * @param filename Path to the configuration file.
     */
    static void loadFile(
        const std::string& filename
    );

    /**
     * @brief Resolves run-dependent values in a configuration property.
     *
     * @param value Raw configuration value.
     *
     * @return Configuration value after run-setting substitution.
     */
    static std::string parseRunSetting(
        const std::string& value
    );

    /**
     * @brief Writes a configuration setting to the configured output stream.
     *
     * @param setting Fully qualified configuration setting name.
     */
    static void outputSetting(
        const std::string& setting
    );

    /**
     * @brief Parses a string as a floating-point value.
     *
     * @param value String representation of the value.
     * @param settingName Name of the configuration setting.
     *
     * @return Parsed double value.
     *
     * @throws std::runtime_error If the value cannot be parsed.
     */
    double parseDoubleValue(
        const std::string& value,
        const std::string& settingName
    ) const;

    /**
     * @brief Converts a floating-point value to an integer.
     *
     * @param doubleValue Value to convert.
     * @param settingName Configuration setting associated with the value.
     *
     * @return Converted integer value.
     *
     * @throws std::runtime_error If the conversion is invalid.
     */
    int convertToInt(
        double doubleValue,
        const std::string& settingName
    ) const;

public:

    /**
     * @brief Constructs an empty Configuration using the default namespace.
     */
    Configuration();

    /**
     * @brief Constructs a Configuration with the specified namespace.
     *
     * @param namespaceName Initial primary namespace.
     */
    explicit Configuration(const std::string& namespaceName);

    /**
     * @brief Changes the active primary namespace.
     *
     * The current namespace is saved so that it can later be restored
     * using restoreNamespace().
     *
     * @param name New primary namespace.
     */
    void setNamespace(const std::string& name);

    /**
     * @brief Restores the previously active primary namespace.
     *
     * Restores the namespace saved by setNamespace().
     */
    void restoreNamespace();

    /**
     * @brief Changes the active secondary namespace.
     *
     * The current secondary namespace is saved so that it can later
     * be restored using restoreSecondaryNamespace().
     *
     * @param name New secondary namespace.
     */
    void setSecondaryNamespace(const std::string& name);

    /**
     * @brief Restores the previously active secondary namespace.
     */
    void restoreSecondaryNamespace();

    /**
     * @brief Checks whether a configuration property exists.
     *
     * @param name Local configuration property name.
     *
     * @return true if the property exists; false otherwise.
     */
    bool contains(const std::string& name) const;

    /**
     * @brief Returns the fully qualified name of a property.
     *
     * @param name Local property name.
     *
     * @return Fully qualified property name.
     */
    std::string getFullPropertyName(
        const std::string& name
    ) const;

    /**
     * @brief Retrieves a string configuration value.
     *
     * @param name Configuration property name.
     *
     * @return Configuration value.
     *
     * @throws std::runtime_error If the property does not exist.
     */
    std::string getConfiguration(
        const std::string& name
    ) const;

    /**
     * @brief Retrieves a string configuration value with a fallback.
     *
     * @param name Configuration property name.
     * @param defaultValue Value returned when the property does not exist.
     *
     * @return Configuration value or @p defaultValue.
     */
    std::string getConfiguration(
        const std::string& name,
        const std::string& defaultValue
    ) const;

    /**
     * @brief Retrieves an optional string configuration value.
     *
     * @param name Configuration property name.
     *
     * @return Property value if present; std::nullopt otherwise.
     */
    std::optional<std::string> getOptionalConfiguration(
        const std::string& name
    ) const;

    /**
     * @brief Retrieves an integer configuration value.
     *
     * @param name Configuration property name.
     *
     * @return Integer configuration value.
     */
    int getInt(
        const std::string& name
    ) const;

    /**
     * @brief Retrieves an integer configuration value with a fallback.
     *
     * @param name Configuration property name.
     * @param defaultValue Value returned when the property does not exist.
     *
     * @return Integer configuration value or @p defaultValue.
     */
    int getInt(
        const std::string& name,
        int defaultValue
    ) const;

    /**
     * @brief Retrieves an optional integer configuration value.
     *
     * @param name Configuration property name.
     *
     * @return Integer value if present; std::nullopt otherwise.
     */
    std::optional<int> getOptionalInt(
        const std::string& name
    ) const;

    /**
     * @brief Retrieves a floating-point configuration value.
     *
     * @param name Configuration property name.
     *
     * @return Double configuration value.
     */
    double getDouble(
        const std::string& name
    ) const;

    /**
     * @brief Retrieves a floating-point configuration value with a fallback.
     *
     * @param name Configuration property name.
     * @param defaultValue Value returned when the property does not exist.
     *
     * @return Double configuration value or @p defaultValue.
     */
    double getDouble(
        const std::string& name,
        double defaultValue
    ) const;

    /**
     * @brief Retrieves an optional floating-point configuration value.
     *
     * @param name Configuration property name.
     *
     * @return Double value if present; std::nullopt otherwise.
     */
    std::optional<double> getOptionalDouble(
        const std::string& name
    ) const;

    /**
     * @brief Retrieves a boolean configuration value.
     *
     * @param name Configuration property name.
     *
     * @return Boolean configuration value.
     */
    bool getBoolean(
        const std::string& name
    ) const;

    /**
     * @brief Retrieves a boolean configuration value with a fallback.
     *
     * @param name Configuration property name.
     * @param defaultValue Value returned when the property does not exist.
     *
     * @return Boolean configuration value or @p defaultValue.
     */
    bool getBoolean(
        const std::string& name,
        bool defaultValue
    ) const;

    /**
     * @brief Retrieves an optional boolean configuration value.
     *
     * @param name Configuration property name.
     *
     * @return Boolean value if present; std::nullopt otherwise.
     */
    std::optional<bool> getOptionalBoolean(
        const std::string& name
    ) const;

    /**
     * @brief Retrieves a comma-separated configuration value.
     *
     * @param name Configuration property name.
     *
     * @return Vector containing the parsed string values.
     */
    std::vector<std::string> getCsvSetting(
        const std::string& name
    ) const;

    /**
     * @brief Retrieves a comma-separated configuration value with validation.
     *
     * @param name Configuration property name.
     * @param expectedCount Expected number of CSV elements.
     *
     * @return Vector containing the parsed string values.
     *
     * @throws std::runtime_error If the number of elements does not match
     *         @p expectedCount.
     */
    std::vector<std::string> getCsvSetting(
        const std::string& name,
        std::size_t expectedCount
    ) const;

    /**
     * @brief Retrieves comma-separated floating-point values.
     *
     * @param name Configuration property name.
     *
     * @return Vector containing parsed double values.
     */
    std::vector<double> getCsvDoubles(
        const std::string& name
    ) const;

    /**
     * @brief Retrieves comma-separated floating-point values with validation.
     *
     * @param name Configuration property name.
     * @param expectedCount Expected number of values.
     *
     * @return Vector containing parsed double values.
     */
    std::vector<double> getCsvDoubles(
        const std::string& name,
        std::size_t expectedCount
    ) const;

    /**
     * @brief Retrieves comma-separated integer values.
     *
     * @param name Configuration property name.
     *
     * @return Vector containing parsed integer values.
     */
    std::vector<int> getCsvInts(
        const std::string& name
    ) const;

    /**
     * @brief Retrieves comma-separated integer values with validation.
     *
     * @param name Configuration property name.
     * @param expectedCount Expected number of values.
     *
     * @return Vector containing parsed integer values.
     */
    std::vector<int> getCsvInts(
        const std::string& name,
        std::size_t expectedCount
    ) const;

    /**
     * @brief Validates an integer range configuration.
     *
     * @param range Integer values representing the range.
     * @param name Configuration property name used for error reporting.
     *
     * @throws std::runtime_error If the range is invalid.
     */
    void assertValidRange(
        const std::vector<int>& range,
        const std::string& name
    ) const;

    /**
     * @brief Replaces configuration placeholders in a string.
     *
     * The method resolves placeholders using the current configuration
     * and run-related settings.
     *
     * @param input Input string containing placeholders.
     *
     * @return String after placeholder substitution.
     */
    std::string valueFillString(
        const std::string& input
    ) const;

    /**
     * @brief Returns a textual representation of this configuration.
     *
     * @return String representation of the active configuration context.
     */
    std::string toString() const;

    /**
     * @brief Creates an object using its registered default constructor.
     *
     * This is a convenience wrapper around ObjectFactory::create().
     *
     * @tparam T Expected object type.
     *
     * @param className Registered class name.
     *
     * @return Shared pointer to the newly created object.
     *
     * @throws std::runtime_error If the class is not registered.
     */
    template <typename T>
    std::shared_ptr<T> createObject(
        const std::string& className
    ) const {
        return ObjectFactory::instance().create<T>(className);
    }

    /**
     * @brief Creates an object using the current configuration.
     *
     * This is a convenience wrapper around the configuration-aware
     * ObjectFactory::create() overload.
     *
     * @tparam T Expected object type.
     *
     * @param className Registered class name.
     *
     * @return Shared pointer to the newly created configured object.
     *
     * @throws std::runtime_error If the class is not registered for
     *         configuration-based construction.
     */
    template <typename T>
    std::shared_ptr<T> createConfiguredObject(
        const std::string& className
    ) const {
        return ObjectFactory::instance().create<T>(
            className,
            *this
        );
    }

    /**
     * @brief Sets the global simulation run index.
     *
     * @param index New run index.
     */
    static void setRunIndex(int index);

    /**
     * @brief Returns the current global simulation run index.
     *
     * @return Current run index.
     */
    static int getRunIndex();

    /**
     * @brief Adds or updates a global configuration property.
     *
     * @param name Property name.
     * @param value Property value.
     */
    static void addSetting(
        const std::string& name,
        const std::string& value
    );

    /**
     * @brief Loads configuration properties from a property file.
     *
     * @param propFile Path to the property file.
     */
    static void addSettings(
        const std::string& propFile
    );

    /**
     * @brief Initializes the global configuration system.
     *
     * This method loads the specified property file and prepares the
     * configuration system for use by the simulation.
     *
     * @param propFile Path to the initial property file.
     */
    static void init(
        const std::string& propFile
    );
};

} // namespace core