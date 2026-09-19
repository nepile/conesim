/**
 * @file ExternalEvent.hpp
 * @brief Super class for all external events.
 * @date September 2026
 * @author Ferry
 * 
 * Ported from Aalto University, ComNet (Java) to C++
 */

#pragma once

#include <string>

// Forward declaration untuk kelas World agar tidak terjadi circular dependency
class World;

namespace input {

    class ExternalEvent {
    protected:
        double time; // Waktu terjadinya event (detik simulasi)

    public:
        explicit ExternalEvent(double time);
        virtual ~ExternalEvent() = default;

        /**
         * Processes the external event.
         * @param world World where the actors of the event are
         */
        virtual void processEvent(World& world);

        /**
         * Returns the time when this event should happen.
         * @return Event's time
         */
        double getTime() const;

        /**
         * Compares two external events by their time (mirip Comparable di Java).
         * @param other The other external event
         * @return -1, 0, 1 if this event happens before, at the same time, or after
         */
        int compareTo(const ExternalEvent& other) const;

        /**
         * Operator kurang dari (<) agar mudah dipakai di C++ Standard Library (seperti std::priority_queue)
         */
        bool operator<(const ExternalEvent& other) const;

        /**
         * Returns a String representation of the event
         * @return string representation
         */
        virtual std::string toString() const;
    };

} // namespace input