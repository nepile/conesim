/**
 * @file MessageStatsReport.hpp
 * @brief Report for generating different kind of total statistics about message relaying performance.
 * @author Opeteer
 * @date September, 2026
 */

#pragma once

#include "report/Report.hpp"
#include "core/MessageListener.hpp"
#include <map>
#include <vector>
#include <string>

namespace core {
    class Message;
    class DTNHost;
}

namespace report {

class MessageStatsReport : public Report, public core::MessageListener {
private:
    std::map<std::string, double> creationTimes;
    std::vector<double> latencies;
    std::vector<int> hopCounts;
    std::vector<double> msgBufferTime;
    std::vector<double> rtt; // round trip times

    int nrofDropped;
    int nrofRemoved;
    int nrofStarted;
    int nrofAborted;
    int nrofRelayed;
    int nrofCreated;
    int nrofResponseReqCreated;
    int nrofResponseDelivered;
    int nrofDelivered;

protected:
    void init() override;

public:
    MessageStatsReport();
    ~MessageStatsReport() override = default;

    // MessageListener methods
    void newMessage(const core::Message& m) override;
    void messageTransferStarted(const core::Message& m, const core::DTNHost& from, const core::DTNHost& to) override;
    void messageDeleted(const core::Message& m, const core::DTNHost& where, bool dropped) override;
    void messageTransferAborted(const core::Message& m, const core::DTNHost& from, const core::DTNHost& to) override;
    void messageTransferred(const core::Message& m, const core::DTNHost& from, const core::DTNHost& to, bool finalTarget) override;

    void done() override;
};

} // namespace report
