#include "core/Configuration.hpp"

#include <fstream>
#include <sstream>
#include <stdexcept>

namespace conesim {

std::unordered_map<std::string, std::string>
    Configuration::properties;

}