#pragma once

class DTNHost;
class Coord;

namespace conesim::core {

/**
 * @file MovementListener.hpp
 * @brief Interface for monitoring node mobility and spatial coordinate events.
 */

/**
 * @class MovementListener
 * @brief Abstract listener interface notified when hosts change location or set new waypoints.
 *
 * Classes that track spatial metrics, generate movement traces, or visualize
 * host trajectories in real time should inherit from this interface.
 */
class MovementListener {
public:
    /**
     * @brief Virtual destructor to allow safe polymorphic cleanup in derived classes.
     */
    virtual ~MovementListener();

    /**
     * @brief Callback invoked when a host picks or starts moving toward a new waypoint.
     *
     * @param host The host that received the new movement directive.
     * @param destination The target coordinate waypoint.
     * @param speed The velocity at which the host is moving towards the destination (in m/s).
     */
    virtual void newDestination(
        const DTNHost& host, 
        const Coord& destination, 
        double speed) = 0;

    /**
     * @brief Callback invoked to register a host's initial spawn coordinates at simulation start.
     *
     * @param host The host whose initial coordinates are being established.
     * @param location The starting spatial coordinates of the host.
     */
    virtual void initialLocation(
        const DTNHost& host, 
        const Coord& location) = 0;
};

} // namespace conesim