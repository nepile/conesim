/**
 * @file SimpleBroadcastInterface.hpp
 * @brief A simple Network Interface providing constant bit-rate service.
 * @details Adapted from The ONE simulator's SimpleBroadcastInterface.java.
 *          Provides constant bit-rate service where one transmission can be on at a time.
 * @author Neville
 * @date September, 2026
 */

#pragma once

#include "core/NetworkInterface.hpp"
#include "core/Configuration.hpp"
#include <string>

namespace interfaces {

/**
 * @class SimpleBroadcastInterface
 * @brief Simple broadcast network interface using Constant Bit-Rate (CBR) connections.
 */
class SimpleBroadcastInterface : public core::NetworkInterface {
public:
    /**
     * @brief Constructs a new SimpleBroadcastInterface reading settings from Configuration.
     * @param config The Configuration object where interface settings are read from.
     */
    explicit SimpleBroadcastInterface(const core::Configuration& config);

    /**
     * @brief Copy constructor.
     * @param ni Prototype interface to copy from.
     */
    SimpleBroadcastInterface(const SimpleBroadcastInterface& ni);

    virtual ~SimpleBroadcastInterface() = default;

    /**
     * @brief Creates a replicated copy of this interface.
     * @return Pointer to the newly allocated SimpleBroadcastInterface copy.
     */
    core::NetworkInterface* replicate() override;

    /**
     * @brief Tries to connect this host's interface to another interface.
     * @details Checks if scanning, both hosts active, within range, and not already connected.
     * @param anotherInterface Target network interface to connect to.
     */
    void connect(core::NetworkInterface* anotherInterface) override;

    /**
     * @brief Updates the state of current connections.
     * @details Tears down connections that moved out of range and checks for new nearby contacts.
     */
    void update() override;

    /**
     * @brief Creates a connection to another host without proximity or scanning checks.
     * @param anotherInterface Target network interface to connect to.
     */
    void createConnection(core::NetworkInterface* anotherInterface) override;

    /**
     * @brief Returns a string representation of the interface.
     * @return Formatted string including type, address, host, and connection status.
     */
    std::string toString() const override;
};

} // namespace interfaces
