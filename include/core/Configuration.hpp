#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include <stack>
#include <optional>

namespace conesim {

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

    int convertToInt(double doubleValue, const std::string& settingName) const;

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

    static void setRunIndex(int index);
    static int getRunIndex();

    static void addSetting(const std::string& name, const std::string& value);
    static void addSettings(const std::string& propFile);
    static void init(const std::string& propFile);
};

}