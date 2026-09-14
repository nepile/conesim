#pragma once

// this is just dummy class. this is gonna remove when Host class has been defined.
class Host;

namespace conesim {

/**
 * @file ConnectionListener.hpp
 * @brief Interface for monitoring connection lifecycle events between hosts.
 */

/**
 * @class ConnectionListener
 * @brief Abstract listener interface notified when communication links form or break.
 *
 * Classes that track topology changes, calculate contact durations, or report
 * connectivity metrics should inherit from this interface and register with
 * the simulation engine.
 */
class ConnectionListener {
public:
    /**
     * @brief Virtual destructor to ensure proper cleanup in derived classes.
     */
    virtual ~ConnectionListener();

    /**
     * @brief Callback invoked when a new connection is established between two hosts.
     *
     * @param host1 The first host involved in the connection.
     * @param host2 The second host involved in the connection.
     */
    virtual void hostsConnected(const Host& host1, const Host& host2) = 0;

    /**
     * @brief Callback invoked when an active connection between two hosts is severed.
     *
     * @param host1 The first host involved in the disconnected link.
     * @param host2 The second host involved in the disconnected link.
     */
    virtual void hostDisconnected(const Host& host1, const Host& host2) = 0;
};

} // namespace conesim