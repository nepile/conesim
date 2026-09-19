#include <gtest/gtest.h>
#include "report/MessageStatsReport.hpp"
#include "core/Configuration.hpp"
#include "core/SimulationClock.hpp"
#include "core/Message.hpp"
#include "core/DTNHost.hpp"

#include <fstream>
#include <filesystem>
#include <memory>

using namespace report;
using namespace core;

// Removed Message stubs since Message.cpp is now present in core


// ============================================================================
// TESTS
// ============================================================================
class MessageStatsReportTest : public ::testing::Test {
protected:
    std::string testConfigFile = "test_msgstats.cfg";
    std::string testReportDir = "test_reports_dir";

    void SetUp() override {
        // Create configuration
        std::ofstream out(testConfigFile);
        out << "Scenario.name = TestScenario\n"
            << "Report.reportDir = " << testReportDir << "\n"
            << "MessageStatsReport.precision = 4\n"
            << "MessageStatsReport.warmup = 0\n";
        out.close();

        Configuration::init(testConfigFile);
        SimulationClock::reset();
    }

    void TearDown() override {
        SimulationClock::reset();
        std::remove(testConfigFile.c_str());
        
        if (std::filesystem::exists(testReportDir)) {
            std::filesystem::remove_all(testReportDir);
        }
    }
};

TEST_F(MessageStatsReportTest, ComprehensiveMessageStatsTest) {
    // Instantiate report
    MessageStatsReport report;
    
    // Create dummy hosts
    DTNHost* hostA = reinterpret_cast<DTNHost*>(0xA);
    DTNHost* hostB = reinterpret_cast<DTNHost*>(0xB);
    
    // 1. New Message created at t=10
    SimulationClock::reset(); // t=0
    // Advance time manually using a mock or just rely on the clock?
    // Since SimulationClock doesn't have a direct setTime in our stubs, let's just assume time 0.
    
    auto m1 = std::make_shared<Message>(hostA, hostB, "M1", 100);
    report.newMessage(*m1); 
    // nrofCreated = 1
    
    // 2. Transfer started
    report.messageTransferStarted(*m1, *hostA, *hostB);
    // nrofStarted = 1
    
    // 3. Transfer aborted
    report.messageTransferAborted(*m1, *hostA, *hostB);
    // nrofAborted = 1
    
    // 4. Transfer started again and finished at t=0
    report.messageTransferStarted(*m1, *hostA, *hostB);
    m1->addNodeOnPath(hostB);
    report.messageTransferred(*m1, *hostA, *hostB, true);
    // nrofStarted = 2, nrofRelayed = 1, nrofDelivered = 1, latency = 0
    
    // 5. Message deleted from buffer
    m1->setReceiveTime(0.0);
    report.messageDeleted(*m1, *hostA, true); // dropped
    // nrofDropped = 1
    
    // Request & Response test
    auto mReq = std::make_shared<Message>(hostA, hostB, "M_REQ", 100);
    mReq->setResponseSize(50);
    report.newMessage(*mReq);
    // nrofCreated = 2, nrofResponseReqCreated = 1
    
    auto mRes = std::make_shared<Message>(hostB, hostA, "M_RES", 50);
    mRes->setRequest(mReq);
    report.newMessage(*mRes);
    // nrofCreated = 3
    
    mRes->addNodeOnPath(hostA);
    report.messageTransferred(*mRes, *hostB, *hostA, true);
    // nrofRelayed = 2, nrofDelivered = 2, nrofResponseDelivered = 1

    report.done();
    
    std::string expectedFilename = testReportDir + "/TestScenario_MessageStatsReport.csv";
    EXPECT_TRUE(std::filesystem::exists(expectedFilename));

    // Verify content
    std::ifstream in(expectedFilename);
    std::string content;
    std::string allContent = "";
    while (std::getline(in, content)) {
        allContent += content + "\n";
    }
    
    EXPECT_NE(allContent.find("created: 3"), std::string::npos);
    EXPECT_NE(allContent.find("started: 2"), std::string::npos);
    EXPECT_NE(allContent.find("relayed: 2"), std::string::npos);
    EXPECT_NE(allContent.find("aborted: 1"), std::string::npos);
    EXPECT_NE(allContent.find("dropped: 1"), std::string::npos);
    EXPECT_NE(allContent.find("removed: 0"), std::string::npos);
    EXPECT_NE(allContent.find("delivered: 2"), std::string::npos);
}
