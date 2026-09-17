/**
 * @file ExternalEvent.cpp
 * @brief Implementation for external events.
 * @date September 2026
 * @author Ferry
 * 
 * Ported from Aalto University, ComNet (Java) to C++
 */

#include "ExternalEvent.hpp"
#include <sstream>

namespace input {

    ExternalEvent::ExternalEvent(double time) : time(time) {}

    void ExternalEvent::processEvent(World& /* world */) {
        // This is just a dummy event
    }

    double ExternalEvent::getTime() const {
        return this->time;
    }

    int ExternalEvent::compareTo(const ExternalEvent& other) const {
        if (this->time == other.time) {
            return 0;
        } else if (this->time < other.time) {
            return -1;
        } else {
            return 1;
        }
    }

    bool ExternalEvent::operator<(const ExternalEvent& other) const {
        return this->time < other.time;
    }

    std::string ExternalEvent::toString() const {
        std::ostringstream oss;
        oss << "ExtEvent @ " << this->time;
        return oss.str();
    }

} // namespace input