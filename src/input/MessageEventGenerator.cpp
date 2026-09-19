/**
 * @file MessageEventGenerator.cpp
 * @brief Implementation for MessageEventGenerator.
 * @date September 2026
 * @author Ferry
 * 
 * Ported from Aalto University, ComNet (Java) to C++
 */

#include "MessageEventGenerator.hpp"
//#include "MessageCreateEvent.hpp"
#include <stdexcept>
#include <functional>
#include <limits>

namespace input {

    const std::string MessageEventGenerator::MESSAGE_SIZE_S = "messageSize";
    const std::string MessageEventGenerator::MESSAGE_INTERVAL_S = "messageInterval";
    const std::string MessageEventGenerator::HOST_RANGE_S = "hostRange";
    const std::string MessageEventGenerator::TO_HOST_RANGE_S = "toHostRange";
    const std::string MessageEventGenerator::MESSAGE_ID_PREFIX_S = "messageIdPrefix";
    const std::string MessageEventGenerator::MESSAGE_TIME_S = "messageTime";

    MessageEventGenerator::MessageEventGenerator(const core::Configuration& s) {
        this->sizeRange = s.getCsvInts(MESSAGE_SIZE_S);
        this->msgInterval = s.getCsvInts(MESSAGE_INTERVAL_S);
        this->hostRange = s.getCsvInts(HOST_RANGE_S, 2);
        this->idPrefix = s.getCsvSetting(MESSAGE_ID_PREFIX_S);
        this->id = 0;

        if (s.contains(MESSAGE_TIME_S)) {
            this->msgTime = s.getCsvDoubles(MESSAGE_TIME_S, 2);
            this->hasMsgTime = true;
        } else {
            this->hasMsgTime = false;
        }

        if (s.contains(TO_HOST_RANGE_S)) {
            this->toHostRange = s.getCsvInts(TO_HOST_RANGE_S, 2);
            this->hasToHostRange = true;
        } else {
            this->hasToHostRange = false;
        }

        std::hash<std::string> hasher;
        this->rng.seed(static_cast<unsigned int>(hasher(idPrefix)));

        if (this->sizeRange.size() == 1) {
            this->sizeRange = {this->sizeRange[0], this->sizeRange[0]};
        } else {
            s.assertValidRange(this->sizeRange, MESSAGE_SIZE_S);
        }
        
        if (this->msgInterval.size() == 1) {
            this->msgInterval = {this->msgInterval[0], this->msgInterval[0]};
        } else {
            s.assertValidRange(this->msgInterval, MESSAGE_INTERVAL_S);
        }
        
        s.assertValidRange(this->hostRange, HOST_RANGE_S);
        
        if (this->hostRange[1] - this->hostRange[0] < 2) {
            if (!this->hasToHostRange) {
                throw core::ConfigurationError("Host range must contain at least two nodes unless toHostRange is defined");
            } else if (toHostRange[0] == this->hostRange[0] && toHostRange[1] == this->hostRange[1]) {
                throw core::ConfigurationError("If to and from host ranges contain only one host, they can't be the equal");
            }
        }
        
        int intervalDiff = (msgInterval[0] == msgInterval[1]) ? 0 : 
            std::uniform_int_distribution<int>(0, msgInterval[1] - msgInterval[0] - 1)(rng);
        
        double startTime = this->hasMsgTime ? this->msgTime[0] : 0.0;
        this->nextEventsTimeVal = startTime + msgInterval[0] + intervalDiff;
    }

    int MessageEventGenerator::drawHostAddress(const std::vector<int>& range) {
        if (range[1] == range[0]) {
            return range[0];
        }
        std::uniform_int_distribution<int> dist(0, range[1] - range[0] - 1);
        return range[0] + dist(rng);
    }

    int MessageEventGenerator::drawMessageSize() {
        int sizeDiff = (sizeRange[0] == sizeRange[1]) ? 0 : 
            std::uniform_int_distribution<int>(0, sizeRange[1] - sizeRange[0] - 1)(rng);
        return sizeRange[0] + sizeDiff;
    }

    int MessageEventGenerator::drawNextEventTimeDiff() {
        int timeDiff = (msgInterval[0] == msgInterval[1]) ? 0 : 
            std::uniform_int_distribution<int>(0, msgInterval[1] - msgInterval[0] - 1)(rng);
        return msgInterval[0] + timeDiff;
    }

    int MessageEventGenerator::drawToAddress(const std::vector<int>& range, int from) {
        int to;
        do {
            to = this->hasToHostRange ? drawHostAddress(this->toHostRange) : drawHostAddress(this->hostRange);
        } while (from == to);
        
        return to;
    }

    std::unique_ptr<ExternalEvent> MessageEventGenerator::next() {
        int responseSize = 0; 
        int msgSize;
        int interval;
        int from;
        int to;
        
        from = drawHostAddress(this->hostRange); 
        to = drawToAddress(this->hostRange, from);
        
        msgSize = drawMessageSize();
        interval = drawNextEventTimeDiff();
        
        auto mce = std::make_unique<MessageCreateEvent>(from, to, this->getID(), 
                msgSize, responseSize, this->nextEventsTimeVal);
                
        this->nextEventsTimeVal += interval;    
        
        if (this->hasMsgTime && this->nextEventsTimeVal > this->msgTime[1]) {
            this->nextEventsTimeVal = std::numeric_limits<double>::max();
        }
        
        return mce;
    }

    double MessageEventGenerator::nextEventsTime() const {
        return this->nextEventsTimeVal;
    }

    std::string MessageEventGenerator::getID() {
        this->id++;
        return idPrefix + std::to_string(this->id);
    }
} // namespace input