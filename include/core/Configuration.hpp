#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include <stack>

namespace conesim {

class Configuration {
private:
    static std::unordered_map<std::string, std::string> properties;

    std::string namespaceName;
    std::string secondaryNamespace;

    std::stack<std::string> oldNamespaces;
    std::stack<std::string> oldSecondaryNamespaces;

    std::string getFullConfigurationName(
        const std::string& name
    ) const;

public:
    Configuration();
    explicit Configuration(const std::string& namespaceName);

    void setNamespace(const std::string& name);
    void restoreNamespace();

    void setSecondaryNamespace(const std::string& name);
    void restoreSecondaryNamespace();

    std::string getConfiguration(
        const std::string& name
    ) const;

    std::string getConfiguration(
        const std::string& name,
        const std::string& defaultValue
    ) const;
};

}