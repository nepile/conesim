/**
 * @file MessageEventGenerator.hpp
 * @brief Interface for message event generators.
 * @date September 2026
 * @author Ferry
 * 
 * Ported from Aalto University, ComNet (Java) to C++
 */

#pragma once

#include <string>
#include <vector>
#include <random>
#include "core/Configuration.hpp"
#include "core/ConfigurationError.hpp"
#include "input/EventQueue.hpp"
#include "input/ExternalEvent.hpp"

namespace input {

    class MessageEventGenerator : public input::EventQueue {
    public:
        static const std::string MESSAGE_SIZE_S;
        static const std::string MESSAGE_INTERVAL_S;
        static const std::string HOST_RANGE_S;
        static const std::string TO_HOST_RANGE_S;
        static const std::string MESSAGE_ID_PREFIX_S;
        static const std::string MESSAGE_TIME_S;

        explicit MessageEventGenerator(const core::Configuration& s);
        virtual ~MessageEventGenerator() = default;

        std::unique_ptr<input::ExternalEvent> next() override;
        double nextEventsTime() const override;

    protected:
        int drawHostAddress(const std::vector<int>& range);
        int drawMessageSize();
        int drawNextEventTimeDiff();
        int drawToHostAddress(const std::vector<int>& range, int from);
        std::string getID();

        double nextEventsTimeVal;
        std::vector<int> hostRange;
        std::vector<int> toHostRange;
        bool hasToHostRange;
        int id;
        std::string idPrefix;
        std::vector<int> sizeRange;
        std::vector<int> msgInterval;
        std::vector<double> msgTime;
        bool hasMsgTime;

        std::mt19937 rng; // Random number generator    
    };
} // namespace input