/**
 * @file SimulationError.hpp
 * @brief Base runtime exception definition for the conesim simulation core.
 * @details Defines the foundational exception hierarchy and nested exception preservation
 *          pattern for simulator runtime errors.
 * @author Neville
 * @date September 2026
 */

#pragma once

#include <string>
#include <stdexcept>
#include <exception>

namespace conesim::core {

/**
 * @class SimulationError
 * @brief Base exception class for all runtime errors occurring during simulation.
 * @details Derived from std::runtime_error, this class supports chaining underlying exceptions
 *          using std::exception_ptr, allowing caller code to inspect root causes across layers.
 * @author Neville
 * @date September 2026
 */
class SimulationError : public std::runtime_error {
private:
    std::exception_ptr nestedException; ///< Holds a pointer to an underlying nested exception, if any.

public:
    /**
     * @brief Constructs a SimulationError with an error description.
     * @param cause Explanatory string describing the simulation failure.
     */
    explicit SimulationError(const std::string& cause)
        : std::runtime_error(cause), nestedException(nullptr) {}

    /**
     * @brief Constructs a SimulationError with an error description and an explicit exception pointer.
     * @param cause Explanatory string describing the failure.
     * @param e Pointer to the underlying exception.
     */
    SimulationError(const std::string& cause, std::exception_ptr e)
        : std::runtime_error(cause), nestedException(e) {}

    /**
     * @brief Constructs a SimulationError wrapping an existing std::exception.
     * @param e Source exception instance to capture.
     */
    explicit SimulationError(const std::exception& e)
        : std::runtime_error(e.what()), nestedException(std::make_exception_ptr(e)) {}

    /**
     * @brief Constructs a SimulationError combining a context message with an existing std::exception.
     * @param cause Contextual error message.
     * @param e Source exception instance to capture and append.
     */
    SimulationError(const std::string& cause, const std::exception& e)
        : std::runtime_error(cause + "\nCause: " + e.what()), 
          nestedException(std::make_exception_ptr(e)) {}

    /**
     * @brief Retrieves the captured nested exception pointer.
     * @return Pointer to the nested std::exception, or nullptr if none was captured.
     */
    std::exception_ptr getException() const noexcept {
        return nestedException;
    }
};

} // namespace conesim