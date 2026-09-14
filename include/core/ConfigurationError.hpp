#pragma once

#include "core/SimulationError.hpp"

namespace conesim {

class ConfigurationError : public SimulationError {
public:
    explicit ConfigurationError(const std::string& cause)
        : SimulationError(cause) {}

    ConfigurationError(const std::string& cause, std::exception_ptr e)
        : SimulationError(cause, e) {}

    explicit ConfigurationError(const std::exception& e)
        : SimulationError(e) {}

    ConfigurationError(const std::string& cause, const std::exception& e)
        : SimulationError(cause, e) {}
};

}