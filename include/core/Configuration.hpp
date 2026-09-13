#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include <stack>

namespace conesim {

class Configuration {
private:
    static std::unordered_map<std::string, std::string> properties;

public:
    Configuration();
};

}