/**
 * @file ExternalEvent.hpp
 * @brief Super class for all external events.
 * @author Opeteer
 * @date September, 2026
 */

#pragma once

#include <string>
#include <memory>

namespace core {
    class World;
}

namespace input {

class ExternalEvent {
protected:
    /** @brief Time of the event (simulated seconds) */
    double time;

public:
    /**
     * @brief Creates a new external event.
     * @param time Time of the event
     */
    ExternalEvent(double time);
    
    virtual ~ExternalEvent() = default;

    /**
     * @brief Processes the external event.
     * @param world World where the actors of the event are
     */
    virtual void processEvent(core::World& world);

    /**
     * @brief Returns the time when this event should happen.
     * @return Event's time
     */
    double getTime() const;

    /**
     * @brief Compares two external events by their time.
     * @param other The other external event
     */
    bool operator<(const ExternalEvent& other) const;
    bool operator==(const ExternalEvent& other) const;
    bool operator>(const ExternalEvent& other) const;

    /**
     * @brief Returns a String representation of the event
     * @return a String representation of the event
     */
    virtual std::string toString() const;
};

} // namespace input
