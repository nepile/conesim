/**
 * @file ConnectionEvent.cpp
 * @brief Implementation of ConnectionEvent
 * @author Opeteer
 * @date October, 2026
 * 
 * Ported from Aalto University, ComNet (Java) to C++
 */

#include "input/ConnectionEvent.hpp"
#include "core/World.hpp"
#include "core/DTNHost.hpp"

#include <cassert>
#include <sstream>

namespace input {

ConnectionEvent::ConnectionEvent(int from, int to, const std::string& interf, bool up, double time)
    : ExternalEvent(time), fromAddr(from), toAddr(to), isUp(up), interfaceId(interf) {
    assert(from != to && "Can't self connect");
}

void ConnectionEvent::processEvent(core::World& world) {
    auto from = world.getNodeByAddress(this->fromAddr);
    auto to = world.getNodeByAddress(this->toAddr);

    if (from != nullptr && to != nullptr) {
        from->forceConnection(to.get(), this->interfaceId, this->isUp);
    }
}

std::string ConnectionEvent::toString() const {
    std::ostringstream ss;
    ss << "CONN " << (isUp ? "up" : "down") << " @" << this->time << " "
       << this->fromAddr << "<->" << this->toAddr;
    return ss.str();
}

} // namespace input
