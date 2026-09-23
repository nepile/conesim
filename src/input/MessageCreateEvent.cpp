/**
 * @file MessageCreateEvent.cpp
 * @brief Implementation of MessageCreateEvent
 * @author Opeteer
 * @date September, 2026
 */

#include "input/MessageCreateEvent.hpp"
#include "core/World.hpp"
#include "core/DTNHost.hpp"
#include "core/Message.hpp"

#include <sstream>
#include <memory>

namespace input {

MessageCreateEvent::MessageCreateEvent(int from, int to, const std::string& id, int size, int responseSize, double time)
    : MessageEvent(from, to, id, time), size(size), responseSize(responseSize) {
}

void MessageCreateEvent::processEvent(core::World& world) {
    auto to = world.getNodeByAddress(this->toAddr);
    auto from = world.getNodeByAddress(this->fromAddr);
    
    // In Java it creates a Message using new and passes to createNewMessage.
    // In C++ conesim we'll allocate it dynamically (shared_ptr).
    auto m = std::make_shared<core::Message>(from.get(), to.get(), this->id, this->size);
    m->setResponseSize(this->responseSize);
    
    // Assuming DTNHost has createNewMessage(std::shared_ptr<Message>)
    from->createNewMessage(m);
}

std::string MessageCreateEvent::toString() const {
    std::ostringstream ss;
    ss << MessageEvent::toString() << " [" << fromAddr << "->" << toAddr << "] "
       << "size:" << size << " CREATE";
    return ss.str();
}

} // namespace input
