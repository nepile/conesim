/**
 * @file ExternalEvent.cpp
 * @brief Implementation of ExternalEvent
 * @author Opeteer
 * @date September, 2026
 */

#include "input/ExternalEvent.hpp"
#include <sstream>

namespace input {

ExternalEvent::ExternalEvent(double time) : time(time) {}

void ExternalEvent::processEvent(core::World& world) {
    // this is just a dummy event by default
}

double ExternalEvent::getTime() const {
    return time;
}

bool ExternalEvent::operator<(const ExternalEvent& other) const {
    return this->time < other.time;
}

bool ExternalEvent::operator==(const ExternalEvent& other) const {
    return this->time == other.time;
}

bool ExternalEvent::operator>(const ExternalEvent& other) const {
    return this->time > other.time;
}

std::string ExternalEvent::toString() const {
    std::ostringstream ss;
    ss << "ExtEvent @ " << time;
    return ss.str();
}

} // namespace input
