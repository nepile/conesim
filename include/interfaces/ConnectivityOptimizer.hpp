/**
 * @file ConnectivityOptimizer.hpp
 * @brief Header definition of the ConnectivityOptimizer abstract interface.
 * @details A superclass/interface for schemes optimizing the location of possible
 *          contacts with network interfaces of a specific range.
 * @author Frathol
 * @date September, 2026
 */

#pragma once

#include <vector>

namespace core
{
  class NetworkInterface;
}

namespace interfaces
{

  /**
   * @class ConnectivityOptimizer
   * @brief Abstract base class for spatial optimization structures.
   */
  class ConnectivityOptimizer
  {
  public:
    virtual ~ConnectivityOptimizer() = default;

    /**
     * @brief Adds a network interface to the optimizer (unless already present).
     * @param ni The network interface to add.
     */
    virtual void addInterface(core::NetworkInterface *ni) = 0;

    /**
     * @brief Adds a collection of network interfaces to the optimizer.
     * @param interfaces A vector of network interfaces to add.
     */
    virtual void addInterfaces(const std::vector<core::NetworkInterface *> &interfaces) = 0;

    /**
     * @brief Updates a network interface's location in the optimization structure.
     * @param ni The network interface whose location changed.
     */
    virtual void updateLocation(core::NetworkInterface *ni) = 0;

    /**
     * @brief Finds all network interfaces that might be close enough to connect.
     * @param ni The network interface looking for connections.
     * @return A vector of network interfaces within proximity.
     */
    virtual std::vector<core::NetworkInterface *> getNearInterfaces(core::NetworkInterface *ni) = 0;

    /**
     * @brief Finds all other interfaces registered to this ConnectivityOptimizer.
     * @return A vector of all registered network interfaces.
     */
    virtual std::vector<core::NetworkInterface *> getAllInterfaces() = 0;
  };

} // namespace interfaces