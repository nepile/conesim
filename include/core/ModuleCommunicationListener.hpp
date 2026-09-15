#pragma once

#include <string>
#include <any>

namespace core {

/**
 * @file ModuleCommunicationListener.hpp
 * @brief Interface for monitoring module communication.
 */

/**
 * @class ModuleCommunicationListener
 * @brief Abstract listener Interface for listening to communication changes between modules.
 * 
 * This class acts as an abstract listener (callback interface) that receives 
 * notifications whenever a module value changes within the simulation.
 */
class ModuleCommunicationListener {
public:
    /**
     * @brief Virtual destructor to ensure proper cleanup of derived classes.
     */
    virtual ~ModuleCommunicationListener();

    /**
     * @brief Callback triggered when a monitored module value changes.
     * 
     * @param key The unique string identifier representing the modified variable or property.
     * @param newValue The new value wrapped in a type-safe dynamic container (std::any).
     * 
     * @note Since newValue is passed as std::any, implementers must use std::any_cast 
     *       with the appropriate type to extract the underlying data safely.
     */
    virtual void moduleValueChanged(
        const std::string& key, 
        const std::any& newValue) = 0;
};

} // namespace core
