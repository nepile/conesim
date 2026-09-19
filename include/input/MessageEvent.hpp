/**
 * @file MessageEvent.hpp
 * @brief A message related external event
 * @author Opeteer
 * @date September, 2026
 */

#pragma once

#include "input/ExternalEvent.hpp"
#include <string>

namespace input {

class MessageEvent : public ExternalEvent {
protected:
    /** @brief address of the node the message is from */
    int fromAddr;
    
    /** @brief address of the node the message is to */
    int toAddr;
    
    /** @brief identifier of the message */
    std::string id;

public:
    /**
     * @brief Creates a message event
     * @param from Where the message comes from
     * @param to Who the message goes to 
     * @param id ID of the message
     * @param time Time when the message event occurs
     */
    MessageEvent(int from, int to, const std::string& id, double time);
    
    virtual ~MessageEvent() = default;

    std::string toString() const override;
};

} // namespace input
