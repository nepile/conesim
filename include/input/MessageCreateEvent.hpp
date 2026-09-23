/**
 * @file MessageCreateEvent.hpp
 * @brief External event for creating a message
 * @author Opeteer
 * @date September, 2026
 */

#pragma once

#include "input/MessageEvent.hpp"

namespace core {
    class World;
    class DTNHost;
    class Message;
}

namespace input {

class MessageCreateEvent : public MessageEvent {
private:
    int size;
    int responseSize;

public:
    /**
     * @brief Creates a message creation event with an optional response request
     * @param from The creator of the message
     * @param to Where the message is destined to
     * @param id ID of the message
     * @param size Size of the message
     * @param responseSize Size of the requested response message or 0 if no response requested
     * @param time Time, when the message is created
     */
    MessageCreateEvent(int from, int to, const std::string& id, int size, int responseSize, double time);
    
    virtual ~MessageCreateEvent() = default;

    /**
     * @brief Creates the message this event represents
     */
    void processEvent(core::World& world) override;

    std::string toString() const override;
};

} // namespace input
