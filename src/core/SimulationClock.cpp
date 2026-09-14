#include "core/SimulationClock.hpp"

#include <cmath>
#include <string>

namespace conesim {

double SimulationClock::clockTime = 0.0;
SimulationClock* SimulationClock::clock = nullptr;

SimulationClock::SimulationClock() {}

SimulationClock* SimulationClock::getInstance() {
    if (clock == nullptr) {
        clock = new SimulationClock();
    }

    return clock;
}

double SimulationClock::getTime() {
    return clockTime;
}

int SimulationClock::getIntTime() {
    return static_cast<int>(std::round(clockTime));
}

void SimulationClock::advance(double time) {
    clockTime += time;
}

void SimulationClock::setTime(double time) {
    clockTime = time;
}

std::string SimulationClock::toString() const {
    return "SimTime: " + std::to_string(clockTime);
}

void SimulationClock::reset() {
    clockTime = 0.0;
}

}