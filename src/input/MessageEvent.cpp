/**
 * @file MessageEvent.cpp
 * @brief Implementation of MessageEvent
 * @author Opeteer
 * @date September, 2026
 */

#include "input/MessageEvent.hpp"
#include <sstream>

namespace input {

MessageEvent::MessageEvent(int from, int to, const std::string& id, double time)
    : ExternalEvent(time), fromAddr(from), toAddr(to), id(id) {
}

std::string MessageEvent::toString() const {
    std::ostringstream ss;
    ss << "MSG @" << time << " " << id;
    return ss.str();
}

} // namespace input
