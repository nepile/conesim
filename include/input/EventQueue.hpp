/**
 * @file EventQueue.hpp
 * @brief Interface for event queues.
 * @date September 2026
 * @author Ferry
 * 
 * Ported from Aalto University, ComNet (Java) to C++
 */

#pragma once

#include <memory>
#include <limits> // for using MAX_VALUE

namespace input {

    class ExternalEvent;

    class EventQueue {
    public:
        virtual ~EventQueue() = default;

        /**
         * Returns the next event in the queue or ExternalEvent with time of
         * std::numeric_limits<double>::max() if there are no events left.
         * @return std::unique_ptr pointer to the next event
         */
        virtual std::unique_ptr<ExternalEvent> next() = 0;

        /**
         * Returns next event's time or std::numeric_limits<double>::max() if there are no events left in the queue.
         * @return Next event's time
         */
        virtual double nextEventsTime() const = 0;
    };
} // namespace input