/**
 * @file SimulationClock.hpp
 * @brief Header definition of the global simulation clock mechanism.
 * @details Implements a singleton-based temporal coordinator responsible for tracking,
 *          advancing, and resetting the virtual elapsed simulation time across the conesim engine.
 * @author Neville
 * @date September, 2026
 */

#pragma once

#include <string>

namespace conesim::core {

/**
 * @class SimulationClock
 * @brief Singleton coordinator for managing simulated engine time.
 * @details Provides both static accessors and instance-based mutation controls
 *          for discrete-event time tracking, supporting floating-point precision
 *          and rounded integer time representations.
 * @author Neville
 * @date September, 2026
 */
class SimulationClock {
private:
    static double clockTime;           ///< Active virtual simulation timestamp in seconds.
    static SimulationClock* clock;     ///< Singleton instance reference.

    /**
     * @brief Private default constructor to prevent direct instantiation.
     */
    SimulationClock();

public:
    /**
     * @brief Retrieves or lazily instantiates the global singleton instance.
     * @return Pointer to the shared SimulationClock instance.
     */
    static SimulationClock* getInstance();

    /**
     * @brief Retrieves current simulation time in floating-point seconds.
     * @return Current simulation time as a double.
     */
    static double getTime();

    /**
     * @brief Retrieves rounded integer representation of current simulation time.
     * @return Current simulation time rounded to the nearest integer.
     */
    static int getIntTime();

    /**
     * @brief Advances simulation time by a specified positive duration.
     * @param time Duration in seconds to step the simulation clock forward.
     */
    void advance(double time);

    /**
     * @brief Sets absolute simulation time explicitly.
     * @param time Timestamp in seconds to set as current clock time.
     */
    void setTime(double time);

    /**
     * @brief Serializes current simulation time to a human-readable diagnostic string.
     * @return Formatted string representation of current simulation time.
     */
    std::string toString() const;

    /**
     * @brief Resets simulation clock back to zero.
     * @details Typically invoked when preparing a new simulation run or clearing engine state.
     */
    static void reset();
};

} // namespace conesim