/**
 * @file ConnectionEvent.hpp
 * @brief External event for forcing a connection up or down between two nodes.
 * @author Opeteer
 * @date October, 2026
 * 
 * Ported from Aalto University, ComNet (Java) to C++
 */

#pragma once

#include "input/ExternalEvent.hpp"
#include <string>

namespace core {
    class World;
}

namespace input {

class ConnectionEvent : public ExternalEvent {
protected:
    /** @brief Address of the node the (dis)connection is from */
    int fromAddr;
    /** @brief Address of the node the (dis)connection is to */
    int toAddr;
    /** @brief Is this a "connection up" event */
    bool isUp;
    /** @brief Interface identifier for this event */
    std::string interfaceId;

public:
    /**
     * @brief Creates a new connection event
     * @param from End point of connection
     * @param to Another end of connection
     * @param interf The interface identifier for the connection
     * @param up If true, this is a "connection up" event; if false, "connection down"
     * @param time Time when the connection event occurs
     */
    ConnectionEvent(int from, int to, const std::string& interf, bool up, double time);
    
    virtual ~ConnectionEvent() = default;

    /**
     * @brief Processes the connection event by forcing the connection state change.
     * @param world World where the hosts are located
     */
    void processEvent(core::World& world) override;

    /**
     * @brief Returns a string representation of the connection event.
     * @return Formatted string describing the event
     */
    std::string toString() const override;
};

} // namespace input
