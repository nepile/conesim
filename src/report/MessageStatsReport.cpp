/**
 * @file MessageStatsReport.cpp
 * @brief Implementation of MessageStatsReport
 * @author Opeteer
 * @date September, 2026
 */

#include "report/MessageStatsReport.hpp"
#include "core/Message.hpp"
#include "core/DTNHost.hpp"
#include <cmath>
#include <sstream>

namespace report {

MessageStatsReport::MessageStatsReport() : Report("MessageStatsReport") {
    init();
}

void MessageStatsReport::init() {
    Report::init();
    
    creationTimes.clear();
    latencies.clear();
    msgBufferTime.clear();
    hopCounts.clear();
    rtt.clear();

    nrofDropped = 0;
    nrofRemoved = 0;
    nrofStarted = 0;
    nrofAborted = 0;
    nrofRelayed = 0;
    nrofCreated = 0;
    nrofResponseReqCreated = 0;
    nrofResponseDelivered = 0;
    nrofDelivered = 0;
}

void MessageStatsReport::messageDeleted(const core::Message& m, const core::DTNHost& where, bool dropped) {
    if (isWarmupID(m.getId())) {
        return;
    }

    if (dropped) {
        nrofDropped++;
    } else {
        nrofRemoved++;
    }

    msgBufferTime.push_back(getSimTime() - m.getReceiveTime());
}

void MessageStatsReport::messageTransferAborted(const core::Message& m, const core::DTNHost& from, const core::DTNHost& to) {
    if (isWarmupID(m.getId())) {
        return;
    }

    nrofAborted++;
}

void MessageStatsReport::messageTransferred(const core::Message& m, const core::DTNHost& from, const core::DTNHost& to, bool finalTarget) {
    if (isWarmupID(m.getId())) {
        return;
    }

    nrofRelayed++;
    if (finalTarget) {
        latencies.push_back(getSimTime() - creationTimes[m.getId()]);
        nrofDelivered++;
        
        // m.getHops() includes the source, so size - 1 is the hop count.
        // Prevent negative hop count if getHops() is somehow empty.
        int hops = static_cast<int>(m.getHops().size());
        hopCounts.push_back(std::max(0, hops - 1));

        if (m.isResponse()) {
            auto request = m.getRequest();
            if (request) {
                rtt.push_back(getSimTime() - request->getCreationTime());
            }
            nrofResponseDelivered++;
        }
    }
}

void MessageStatsReport::newMessage(const core::Message& m) {
    if (isWarmup()) {
        addWarmupID(m.getId());
        return;
    }

    creationTimes[m.getId()] = getSimTime();
    nrofCreated++;
    if (m.getResponseSize() > 0) {
        nrofResponseReqCreated++;
    }
}

void MessageStatsReport::messageTransferStarted(const core::Message& m, const core::DTNHost& from, const core::DTNHost& to) {
    if (isWarmupID(m.getId())) {
        return;
    }

    nrofStarted++;
}

void MessageStatsReport::done() {
    write("Message stats for scenario " + getScenarioName() + 
          "\nsim_time: " + format(getSimTime()));
          
    double deliveryProb = 0.0;
    double responseProb = 0.0;
    
    std::string overHeadStr = NAN_STRING;
    
    if (nrofCreated > 0) {
        deliveryProb = (1.0 * nrofDelivered) / nrofCreated;
    }
    
    if (nrofDelivered > 0) {
        double overHead = (1.0 * (nrofRelayed - nrofDelivered)) / nrofDelivered;
        overHeadStr = format(overHead);
    }
    
    if (nrofResponseReqCreated > 0) {
        responseProb = (1.0 * nrofResponseDelivered) / nrofResponseReqCreated;
    }
    
    std::ostringstream ss;
    ss << "created: " << nrofCreated << "\n"
       << "started: " << nrofStarted << "\n"
       << "relayed: " << nrofRelayed << "\n"
       << "aborted: " << nrofAborted << "\n"
       << "dropped: " << nrofDropped << "\n"
       << "removed: " << nrofRemoved << "\n"
       << "delivered: " << nrofDelivered << "\n"
       << "delivery_prob: " << format(deliveryProb) << "\n"
       << "response_prob: " << format(responseProb) << "\n"
       << "overhead_ratio: " << overHeadStr << "\n"
       << "latency_avg: " << getAverage(latencies) << "\n"
       << "latency_med: " << getMedian(latencies) << "\n"
       << "hopcount_avg: " << getIntAverage(hopCounts) << "\n"
       << "hopcount_med: " << getIntMedian(hopCounts) << "\n"
       << "buffertime_avg: " << getAverage(msgBufferTime) << "\n"
       << "buffertime_med: " << getMedian(msgBufferTime) << "\n"
       << "rtt_avg: " << getAverage(rtt) << "\n"
       << "rtt_med: " << getMedian(rtt);
       
    write(ss.str());
    Report::done();
}

} // namespace report
