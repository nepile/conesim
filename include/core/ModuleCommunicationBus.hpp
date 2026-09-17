/**
 * @file ModuleCommunicationBus.hpp
 * @brief Header definition of the ModuleCommunicationBus class.
 * @details Intermodule communication bus. Works as a blackboard where modules can
 *          post data, subscribe to data changes, and also poll for data values.
 *          This allows decoupled communication between different parts of a node.
 * @author Frathol
 * @date September, 2026
 */

#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include <any>
#include <stdexcept>

namespace core
{

  class ModuleCommunicationListener;

  /**
   * @class ModuleCommunicationBus
   * @brief Blackboard system for inter-module communication within a host.
   */
  class ModuleCommunicationBus
  {
  public:
    /**
     * @brief Default constructor.
     */
    ModuleCommunicationBus() = default;

    /**
     * @brief Default destructor.
     */
    ~ModuleCommunicationBus() = default;

    /**
     * @brief Adds a new property for this node.
     * @details The key can be any string, but it should be unique to prevent collisions.
     * @param key The key which is used to lookup the value.
     * @param value The value to store, wrapped in std::any.
     * @throws std::runtime_error if there is already a value for the given key.
     */
    void addProperty(const std::string &key, const std::any &value);

    /**
     * @brief Returns an object that was stored using the given key.
     * @param key The key used to lookup the object.
     * @return The stored object as std::any. If not found, returns an empty std::any.
     */
    std::any getProperty(const std::string &key) const;

    /**
     * @brief Updates a value for an existing property.
     * @details Triggers notifications to all subscribed listeners.
     * @param key The key which is used to lookup the value.
     * @param value The new value to store.
     */
    void updateProperty(const std::string &key, const std::any &value);

    /**
     * @brief Changes the Double value with the given key by adding a delta.
     * @param key The key of the variable to update.
     * @param delta Value added to the old value.
     * @return The newly calculated value.
     * @throws std::runtime_error if the key doesn't exist or isn't a double.
     */
    double updateDouble(const std::string &key, double delta);

    /**
     * @brief Returns a double value from the communication bus.
     * @param key The key of the variable.
     * @param naValue The fallback value to return if there is no value for the key.
     * @return The value of the key, or naValue if not found.
     * @throws std::runtime_error if the stored value is not a double.
     */
    double getDouble(const std::string &key, double naValue) const;

    /**
     * @brief Returns an integer value from the communication bus.
     * @param key The key of the variable.
     * @param naValue The fallback value to return if there is no value for the key.
     * @return The value of the key, or naValue if not found.
     * @throws std::runtime_error if the stored value is not an integer.
     */
    int getInt(const std::string &key, int naValue) const;

    /**
     * @brief Subscribes a module to listen for changes to a specific key.
     * @param key The key of the value whose changes the module is interested in.
     * @param module Pointer to the listener module.
     */
    void subscribe(const std::string &key, ModuleCommunicationListener *module);

    /**
     * @brief Removes a notification subscription.
     * @param key The key for which the subscription should be removed.
     * @param module Pointer to the listener module.
     */
    void unsubscribe(const std::string &key, ModuleCommunicationListener *module);

    /**
     * @brief Returns a string representation of the bus and its current mappings.
     * @return A formatted string.
     */
    std::string toString() const;

  private:
    /**
     * @brief The values in the blackboard.
     * @details std::any is used to safely hold any data type (int, double, string, etc.).
     */
    std::unordered_map<std::string, std::any> values;

    /**
     * @brief Subscribed listeners grouped by the key they are listening to.
     */
    std::unordered_map<std::string, std::vector<ModuleCommunicationListener *>> listeners;

    /**
     * @brief Notifies all listeners that have subscribed to the given key.
     * @param key The key which got a new value.
     * @param newValue The new value for the key.
     */
    void notifyListeners(const std::string &key, const std::any &newValue) const;
  };

} // namespace core