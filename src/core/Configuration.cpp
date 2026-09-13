#include "core/Configuration.hpp"

#include <fstream>
#include <sstream>
#include <stdexcept>

namespace conesim {

std::unordered_map<std::string, std::string>
    Configuration::properties;


Configuration::Configuration()
    :   namespaceName(""),
        secondaryNamespace("")
{}

Configuration::Configuration(const std::string& namespaceName)
    :   namespaceName(namespaceName),
        secondaryNamespace("")
{}

void Configuration::setNamespace(const std::string& name)
{
    oldNamespaces.push(namespaceName);
    namespaceName = name;
}

void Configuration::restoreNamespace()
{
    if (oldNamespaces.empty())
    {
        throw std::runtime_error(
            "No previous namespace to restore"
        );
    }

    namespaceName = oldNamespaces.top();
    oldNamespaces.pop();
}

void Configuration::setSecondaryNamespace(const std::string& name)
{
    oldSecondaryNamespaces.push(secondaryNamespace);
    secondaryNamespace = name;
}

void Configuration::restoreSecondaryNamespace()
{
    if (oldSecondaryNamespaces.empty())
    {
        throw std::runtime_error(
            "No previous secondary namespace to restore"
        );
    }

    secondaryNamespace = oldSecondaryNamespaces.top();
    oldSecondaryNamespaces.pop();
}


}