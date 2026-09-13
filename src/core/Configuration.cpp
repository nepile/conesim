#include "core/Configuration.hpp"

#include <fstream>
#include <sstream>
#include <stdexcept>
#include <algorithm>
#include <cctype>

namespace conesim {

std::unordered_map<std::string, std::string> Configuration::properties;

Configuration::Configuration(): namespaceName(""), secondaryNamespace("") {}

Configuration::Configuration(const std::string& namespaceName): namespaceName(namespaceName), secondaryNamespace("") {}

void Configuration::setNamespace(const std::string& name) {
    oldNamespaces.push(namespaceName);
    namespaceName = name;
}

void Configuration::restoreNamespace() {
    if (oldNamespaces.empty())
    {
        throw std::runtime_error(
            "No previous namespace to restore"
        );
    }

    namespaceName = oldNamespaces.top();
    oldNamespaces.pop();
}

void Configuration::setSecondaryNamespace(const std::string& name) {
    oldSecondaryNamespaces.push(secondaryNamespace);
    secondaryNamespace = name;
}

void Configuration::restoreSecondaryNamespace() {
    if (oldSecondaryNamespaces.empty())
    {
        throw std::runtime_error(
            "No previous secondary namespace to restore"
        );
    }

    secondaryNamespace = oldSecondaryNamespaces.top();
    oldSecondaryNamespaces.pop();
}

std::string Configuration::getFullConfigurationName(const std::string& name) const {
    if(namespaceName.empty()) {
        return name;
    }

    return namespaceName + "." + name;
}

std::string Configuration::getConfiguration(const std::string& name) const {
    const std::string fullName = getFullConfigurationName(name);

    auto it = properties.find(fullName);

    if (it == properties.end()) {
        throw std::runtime_error(
            "Configuration not found: " + fullName
        );
    }

    return it->second;
}

std::string Configuration::getConfiguration(const std::string& name, const std::string& defaultValue) const {
    const std::string fullName = getFullConfigurationName(name);

    auto it = properties.find(fullName);

    if (it == properties.end())
    {
        return defaultValue;
    }

    return it->second;
}

void Configuration::loadFile(const std::string& filename) {
    std::ifstream file(filename);

    if (!file.is_open()) {
        throw std::runtime_error(
            "Failed to open configuration file: "
            + filename
        );
    }

    std::string line;

    while (std::getline(file, line)) {
        line.erase(
            line.begin(),
            std::find_if(
                line.begin(),
                line.end(),
                [](unsigned char ch) {
                    return !std::isspace(ch);
                }
            )
        );

        if (line.empty()) {
            continue;
        }

        if (line[0] == '#') {
            continue;
        }

        const std::size_t delimiter =
            line.find('=');

        if (delimiter == std::string::npos) {
            continue;
        }

        std::string key =
            line.substr(0, delimiter);

        std::string value =
            line.substr(delimiter + 1);

        key.erase(
            key.begin(),
            std::find_if(
                key.begin(),
                key.end(),
                [](unsigned char ch) {
                    return !std::isspace(ch);
                }
            )
        );

        key.erase(
            std::find_if(
                key.rbegin(),
                key.rend(),
                [](unsigned char ch) {
                    return !std::isspace(ch);
                }
            ).base(),
            key.end()
        );

        value.erase(
            value.begin(),
            std::find_if(
                value.begin(),
                value.end(),
                [](unsigned char ch) {
                    return !std::isspace(ch);
                }
            )
        );

        value.erase(
            std::find_if(
                value.rbegin(),
                value.rend(),
                [](unsigned char ch) {
                    return !std::isspace(ch);
                }
            ).base(),
            value.end()
        );

        properties[key] = value;
    }
}

void Configuration::init(const std::string& propFile) {
    properties.clear();

    loadFile(
        "settings/default_settings.cfg"
    );

    if(!propFile.empty()) {
        loadFile(propFile);
    }
}
}