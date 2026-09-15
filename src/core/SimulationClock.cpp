/**
 * @file SimulationClock.cpp
 * @brief Implementation of the global simulation clock mechanism.
 * @details Manages static clock state transitions, precision rounding conversions,
 *          and lazy singleton lifecycle initialization for the conesim engine.
 * @author Neville Jeremy Onorato Laia
 */

#include "core/SimulationClock.hpp"

#include <cmath>
#include <string>

namespace core {

double SimulationClock::clockTime = 0.0;
SimulationClock* SimulationClock::clock = nullptr;

/**
 * @brief Constructs the SimulationClock instance.
 */
SimulationClock::SimulationClock() {}

/**
 * @brief Obtains the singleton instance, creating it on first access.
 * @return Pointer to the shared SimulationClock instance.
 */
SimulationClock* SimulationClock::getInstance() {
    if (clock == nullptr) {
        clock = new SimulationClock();
    }

    return clock;
}

/**
 * @brief Retrieves the current simulation timestamp.
 * @return High-resolution double representing simulated seconds elapsed.
 */
double SimulationClock::getTime() {
    return clockTime;
}

/**
 * @brief Computes nearest integer second representation of simulation time.
 * @return Simulation time rounded to the nearest integer.
 */
int SimulationClock::getIntTime() {
    return static_cast<int>(std::round(clockTime));
}

/**
 * @brief Advances the clock timestamp forward.
 * @param time Duration increment in seconds to accumulate.
 */
void SimulationClock::advance(double time) {
    clockTime += time;
}

/**
 * @brief Overrides the simulation time with an explicit target value.
 * @param time Timestamp in seconds to assign.
 */
void SimulationClock::setTime(double time) {
    clockTime = time;
}

/**
 * @brief Converts the current clock state into a readable string format.
 * @return String formatted as `"SimTime: <time>"`.
 */
std::string SimulationClock::toString() const {
    return "SimTime: " + std::to_string(clockTime);
}

/**
 * @brief Resets the global simulation timestamp to zero.
 */
void SimulationClock::reset() {
    clockTime = 0.0;
}

} // namespace core