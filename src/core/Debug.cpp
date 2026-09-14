/**
 * @file Debug.cpp
 * @brief Implementation of the Debug logging and timing utility.
 * @author Neville
 * @date September 2026
 */

#include "core/Debug.hpp"
#include "core/SimulationClock.hpp"

#include <iostream>
#include <chrono>

namespace conesim {

std::ostream* Debug::out = &std::cout;
int Debug::debugLevel = 0;
long long Debug::timingStart = -1;
std::string Debug::timingCause = "";

void Debug::setDebugLevel(int level) {
    debugLevel = level;
}

void Debug::setPrintStream(std::ostream& outStrm) {
    out = &outStrm;
}

void Debug::p(const std::string& txt) {
    p(txt, 0, false);
}

void Debug::p(const std::string& txt, int level) {
    p(txt, level, false);
}

void Debug::p(const std::string& txt, int level, bool timestamp) {
    if (level < debugLevel) {
        return;
    }

    std::string time = "";
    int simTime = SimulationClock::getIntTime();

    if (timestamp) {
        time = "[@" + std::to_string(simTime) + "]";
    }

    *out << "D" << time << ": " << txt << std::endl;
}

void Debug::pt(const std::string& txt, int level) {
    p(txt, level, true);
}

void Debug::pt(const std::string& txt) {
    p(txt, 0, true);
}

void Debug::startTiming(const std::string& cause) {
    if (timingStart != -1) {
        doneTiming();
    }
    timingCause = cause;
    timingStart = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()
    ).count();
}

void Debug::doneTiming() {
    if (timingStart == -1) {
        return;
    }

    long long end = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()
    ).count();

    long long diff = end - timingStart;
    if (diff > 0) {
        pt(timingCause + " took " + std::to_string(diff / 1000.0) + "s");
    }

    timingStart = -1;
}

} // namespace conesim