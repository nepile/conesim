#pragma once

#include <vector>


namespace conesim {

class DTNHost;

/**
 * @file UpdateListener.hpp
 * @brief Interface for receiving periodic simulation update events.
 */

/**
 * @class UpdateListener
 * @brief Abstract listener interface notified on every simulation update tick.
 *
 * Classes that need to inspect or react to the state of all hosts in the
 * simulation (such as metric reporters, GUI visualizers, or monitors) should
 * inherit from this interface and register themselves to the simulation world.
 */
class UpdateListener {
public:
    /**
     * @brief Virtual destructor to allow safe polymorphic destruction.
     */
    virtual ~UpdateListener();

    /**
     * @brief Callback invoked after every simulation update cycle.
     *
     * @param hosts Read-only reference to the list of active host pointers
     *              in the simulation for the current tick.
     */
    virtual void updated(const std::vector<DTNHost*>& hosts) = 0;
};

} // namespace conesim