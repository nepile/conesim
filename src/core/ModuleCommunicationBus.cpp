/**
 * @file ModuleCommunicationBus.cpp
 * @brief Implementation of the ModuleCommunicationBus class.
 * @details Handles inter-module data sharing and the observer pattern (pub/sub)
 *          safely using C++17 std::any for type erasure. This allows decoupled
 *          communication between different parts of a node without strict dependencies.
 * @author Frathol
 * @date September, 2026
 */

#include "core/ModuleCommunicationBus.hpp"
#include "core/ModuleCommunicationListener.hpp"

#include <sstream>
#include <algorithm>

namespace core
{

  /**
   * @brief Adds a new property to the communication bus.
   * @details Ensures that no accidental namespace collisions happen by checking if
   *          the key already exists before insertion.
   * @param key The unique identifier for the property.
   * @param value The initial value wrapped in std::any.
   * @throws std::runtime_error If a property with the given key already exists.
   */
  void ModuleCommunicationBus::addProperty(const std::string &key, const std::any &value)
  {
    if (this->values.find(key) != this->values.end())
    {
      throw std::runtime_error("A value for the key '" + key + "' already exists");
    }

    this->updateProperty(key, value);
  }

  /**
   * @brief Retrieves a stored property from the bus.
   * @param key The unique identifier of the property.
   * @return The stored value as std::any. Returns an empty std::any if the key is not found.
   */
  std::any ModuleCommunicationBus::getProperty(const std::string &key) const
  {
    auto it = this->values.find(key);
    if (it != this->values.end())
    {
      return it->second;
    }
    return std::any(); // Returns an empty std::any
  }

  /**
   * @brief Updates an existing property or creates it if it doesn't exist.
   * @details Triggers notification to all subscribed listeners about the value change.
   * @param key The unique identifier of the property.
   * @param value The new value to store.
   */
  void ModuleCommunicationBus::updateProperty(const std::string &key, const std::any &value)
  {
    this->values[key] = value;
    notifyListeners(key, value);
  }

  /**
   * @brief Safely increments a stored double value by a specific delta.
   * @param key The unique identifier of the double variable.
   * @param delta The amount to add to the current value.
   * @return The newly calculated double value.
   * @throws std::runtime_error If the key does not exist or if the stored value is not a double.
   */
  double ModuleCommunicationBus::updateDouble(const std::string &key, double delta)
  {
    auto it = this->values.find(key);
    if (it == this->values.end())
    {
      throw std::runtime_error("No value for key '" + key + "'");
    }

    try
    {
      double current = std::any_cast<double>(it->second);
      double newValue = current + delta;
      updateProperty(key, newValue);
      return newValue;
    }
    catch (const std::bad_any_cast &)
    {
      throw std::runtime_error("No Double value for key '" + key + "'");
    }
  }

  /**
   * @brief Safely retrieves a double value from the bus.
   * @param key The unique identifier of the double variable.
   * @param naValue The fallback value to return if the key is not found.
   * @return The stored double value, or naValue if the key doesn't exist.
   * @throws std::runtime_error If the key exists but the stored value is not a double.
   */
  double ModuleCommunicationBus::getDouble(const std::string &key, double naValue) const
  {
    auto it = this->values.find(key);
    if (it == this->values.end())
    {
      return naValue;
    }

    try
    {
      return std::any_cast<double>(it->second);
    }
    catch (const std::bad_any_cast &)
    {
      throw std::runtime_error("No Double value for key '" + key + "'");
    }
  }

  /**
   * @brief Safely retrieves an integer value from the bus.
   * @param key The unique identifier of the integer variable.
   * @param naValue The fallback value to return if the key is not found.
   * @return The stored integer value, or naValue if the key doesn't exist.
   * @throws std::runtime_error If the key exists but the stored value is not an integer.
   */
  int ModuleCommunicationBus::getInt(const std::string &key, int naValue) const
  {
    auto it = this->values.find(key);
    if (it == this->values.end())
    {
      return naValue;
    }

    try
    {
      return std::any_cast<int>(it->second);
    }
    catch (const std::bad_any_cast &)
    {
      throw std::runtime_error("No Integer value for key '" + key + "'");
    }
  }

  /**
   * @brief Registers a module to listen for changes to a specific property key.
   * @param key The unique identifier of the property to monitor.
   * @param module Pointer to the listener module.
   */
  void ModuleCommunicationBus::subscribe(const std::string &key, ModuleCommunicationListener *module)
  {
    // std::unordered_map automatically creates the vector if the key doesn't exist yet
    this->listeners[key].push_back(module);
  }

  /**
   * @brief Removes a module from the subscription list of a specific property key.
   * @param key The unique identifier of the property.
   * @param module Pointer to the listener module to remove.
   */
  void ModuleCommunicationBus::unsubscribe(const std::string &key, ModuleCommunicationListener *module)
  {
    auto it = this->listeners.find(key);
    if (it == this->listeners.end())
    {
      return; // No subscriptions for this key
    }

    auto &list = it->second;
    // C++ Erase-Remove idiom for optimal element removal from a vector without memory leaks
    list.erase(std::remove(list.begin(), list.end(), module), list.end());
  }

  /**
   * @brief Internal method to notify all subscribed listeners about a value change.
   * @param key The unique identifier of the property that changed.
   * @param newValue The updated value wrapped in std::any.
   */
  void ModuleCommunicationBus::notifyListeners(const std::string &key, const std::any &newValue) const
  {
    auto it = this->listeners.find(key);
    if (it == this->listeners.end())
    {
      return;
    }

    // Iterate through all subscribed listeners and trigger their callbacks
    for (ModuleCommunicationListener *mcl : it->second)
    {
      if (mcl != nullptr)
      {
        mcl->moduleValueChanged(key, newValue);
      }
    }
  }

  /**
   * @brief Returns a string representation of the bus and its stored mappings.
   * @details Useful for debugging purposes to inspect the blackboard's state.
   * @return Formatted string containing the number of properties and their keys.
   */
  std::string ModuleCommunicationBus::toString() const
  {
    std::ostringstream oss;
    oss << "ComBus with " << this->values.size() << " mapping(s)";

    // Iterating to show keys (values are hidden because std::any cannot be generically stringified)
    if (!this->values.empty())
    {
      oss << " [Keys: ";
      bool first = true;
      for (const auto &pair : this->values)
      {
        if (!first)
          oss << ", ";
        oss << pair.first;
        first = false;
      }
      oss << "]";
    }

    return oss.str();
  }

} // namespace core