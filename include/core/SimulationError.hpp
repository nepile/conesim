#pragma once

#include <string>
#include <stdexcept>
#include <exception>

namespace conesim {

class SimulationError : public std::runtime_error {
private:
    std::exception_ptr nestedException;

public:
    explicit SimulationError(const std::string& cause)
        : std::runtime_error(cause), nestedException(nullptr) {}

    SimulationError(const std::string& cause, std::exception_ptr e)
        : std::runtime_error(cause), nestedException(e) {}

    explicit SimulationError(const std::exception& e)
        : std::runtime_error(e.what()), nestedException(std::make_exception_ptr(e)) {}

    SimulationError(const std::string& cause, const std::exception& e)
        : std::runtime_error(cause + "\nCause: " + e.what()), 
          nestedException(std::make_exception_ptr(e)) {}

    std::exception_ptr getException() const noexcept {
        return nestedException;
    }
};

}