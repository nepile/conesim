/**
 * @file ConfigurationError.hpp
 * @brief Declaration of the configuration-specific runtime exception class.
 * @details Represents errors encountered during configuration loading, key lookup,
 *          type conversion, and parameter format verification in the conesim engine.
 * @author Neville
 * @date September 2026
 */

#pragma once

#include "core/SimulationError.hpp"

namespace conesim {

/**
 * @class ConfigurationError
 * @brief Exception thrown when an error occurs in the configuration subsystem.
 * @details Inherits from SimulationError to maintain the unified simulator exception hierarchy,
 *          supporting descriptive messages and nested exception forwarding.
 * @author Neville
 * @date September 2026
 */
class ConfigurationError : public SimulationError {
public:
    /**
     * @brief Constructs a ConfigurationError with a descriptive message.
     * @param cause Explanatory string describing the error.
     */
    explicit ConfigurationError(const std::string& cause)
        : SimulationError(cause) {}

    /**
     * @brief Constructs a ConfigurationError with a descriptive message and an underlying exception pointer.
     * @param cause Explanatory string describing the error.
     * @param e Pointer to the nested exception that caused this failure.
     */
    ConfigurationError(const std::string& cause, std::exception_ptr e)
        : SimulationError(cause, e) {}

    /**
     * @brief Constructs a ConfigurationError by wrapping an existing std::exception.
     * @param e Source exception instance.
     */
    explicit ConfigurationError(const std::exception& e)
        : SimulationError(e) {}

    /**
     * @brief Constructs a ConfigurationError with a custom message wrapping an existing std::exception.
     * @param cause Explanatory string describing the error context.
     * @param e Source exception instance.
     */
    ConfigurationError(const std::string& cause, const std::exception& e)
        : SimulationError(cause, e) {}
};

} // namespace conesim