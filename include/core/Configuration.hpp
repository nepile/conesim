#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include <stack>
#include <optional>
#include <memory>
#include <functional>
#include <stdexcept>

namespace conesim {

class Configuration;

class ObjectFactory {
public:
    using DefaultCreator = std::function<std::shared_ptr<void>()>;
    using ConfiguredCreator = std::function<std::shared_ptr<void>(const Configuration&)>;

    static ObjectFactory& instance();

    void registerType(const std::string& className, DefaultCreator creator);
    void registerType(const std::string& className, ConfiguredCreator creator);

    bool hasType(const std::string& className) const;

    template <typename T>
    std::shared_ptr<T> create(const std::string& className) const {
        auto it = defaultRegistry.find(className);
        if (it == defaultRegistry.end()) {
            throw std::runtime_error("Class not registered for default construction: '" + className + "'");
        }
        return std::static_pointer_cast<T>(it->second());
    }

    template <typename T>
    std::shared_ptr<T> create(const std::string& className, const Configuration& config) const {
        auto it = configuredRegistry.find(className);
        if (it == configuredRegistry.end()) {
            throw std::runtime_error("Class not registered for configured construction: '" + className + "'");
        }
        return std::static_pointer_cast<T>(it->second(config));
    }

private:
    std::unordered_map<std::string, DefaultCreator> defaultRegistry;
    std::unordered_map<std::string, ConfiguredCreator> configuredRegistry;
};

template <typename T>
struct AutoRegister {
    explicit AutoRegister(const std::string& className) {
        ObjectFactory::instance().registerType(className, []() -> std::shared_ptr<void> {
            return std::make_shared<T>();
        });
    }

    AutoRegister(const std::string& className, bool isConfigured) {
        if (isConfigured) {
            ObjectFactory::instance().registerType(className, [](const Configuration& cfg) -> std::shared_ptr<void> {
                return std::make_shared<T>(cfg);
            });
        }
    }
};

#define REGISTER_TYPE(ClassName) \
    static conesim::AutoRegister<ClassName> _reg_##ClassName(#ClassName);

#define REGISTER_CONFIGURABLE_TYPE(ClassName) \
    static conesim::AutoRegister<ClassName> _reg_conf_##ClassName(#ClassName, true);

class Configuration {
public:
    static inline const std::string FILL_DELIMITER = "%%";

private:
    static std::unordered_map<std::string, std::string> properties;
    static int runIndex;

    std::string namespaceName;
    std::string secondaryNamespace;

    std::stack<std::string> oldNamespaces;
    std::stack<std::string> oldSecondaryNamespaces;

    std::string getFullConfigurationName(
        const std::string& name,
        bool useSecondary = false
    ) const;

    static void loadFile(
        const std::string& filename
    );

    static std::string parseRunSetting(
        const std::string& value
    );

    double parseDoubleValue(
        const std::string& value,
        const std::string& settingName
    ) const;

    int convertToInt(
        double doubleValue,
        const std::string& settingName
    ) const;

public:
    Configuration();
    explicit Configuration(const std::string& namespaceName);

    void setNamespace(const std::string& name);
    void restoreNamespace();

    void setSecondaryNamespace(const std::string& name);
    void restoreSecondaryNamespace();

    bool contains(const std::string& name) const;
    std::string getFullPropertyName(const std::string& name) const;

    std::string getConfiguration(const std::string& name) const;
    std::string getConfiguration(const std::string& name, const std::string& defaultValue) const;
    std::optional<std::string> getOptionalConfiguration(const std::string& name) const;

    int getInt(const std::string& name) const;
    int getInt(const std::string& name, int defaultValue) const;
    std::optional<int> getOptionalInt(const std::string& name) const;

    double getDouble(const std::string& name) const;
    double getDouble(const std::string& name, double defaultValue) const;
    std::optional<double> getOptionalDouble(const std::string& name) const;

    bool getBoolean(const std::string& name) const;
    bool getBoolean(const std::string& name, bool defaultValue) const;
    std::optional<bool> getOptionalBoolean(const std::string& name) const;

    std::vector<std::string> getCsvSetting(const std::string& name) const;
    std::vector<std::string> getCsvSetting(const std::string& name, std::size_t expectedCount) const;

    std::vector<double> getCsvDoubles(const std::string& name) const;
    std::vector<double> getCsvDoubles(const std::string& name, std::size_t expectedCount) const;

    std::vector<int> getCsvInts(const std::string& name) const;
    std::vector<int> getCsvInts(const std::string& name, std::size_t expectedCount) const;

    void assertValidRange(const std::vector<int>& range, const std::string& name) const;

    std::string valueFillString(const std::string& input) const;

    template <typename T>
    std::shared_ptr<T> createObject(const std::string& className) const {
        return ObjectFactory::instance().create<T>(className);
    }

    template <typename T>
    std::shared_ptr<T> createConfiguredObject(const std::string& className) const {
        return ObjectFactory::instance().create<T>(className, *this);
    }

    static void setRunIndex(int index);
    static int getRunIndex();

    static void addSetting(const std::string& name, const std::string& value);
    static void addSettings(const std::string& propFile);
    static void init(const std::string& propFile);
};

}