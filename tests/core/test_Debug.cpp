#include <gtest/gtest.h>
#include <sstream>
#include <thread>
#include <chrono>

#include "core/Debug.hpp"
#include "core/SimulationClock.hpp"

using namespace conesim::core;

class DebugTest : public ::testing::Test {
protected:
    std::ostringstream testStream;

    void SetUp() override {
        Debug::setPrintStream(testStream);
        Debug::setDebugLevel(0);
        testStream.str("");
        testStream.clear();
    }

    void TearDown() override {
        Debug::setPrintStream(std::cout);
        Debug::setDebugLevel(0);
    }
};

TEST_F(DebugTest, PrintsBasicMessageWithoutTimestamp) {
    Debug::p("Test message");
    EXPECT_EQ(testStream.str(), "D: Test message\n");
}

TEST_F(DebugTest, FiltersMessagesBasedOnDebugLevel) {
    Debug::setDebugLevel(2);

    Debug::p("Level 1 message", 1);
    EXPECT_TRUE(testStream.str().empty());

    Debug::p("Level 2 message", 2);
    EXPECT_EQ(testStream.str(), "D: Level 2 message\n");

    testStream.str("");
    Debug::p("Level 3 message", 3);
    EXPECT_EQ(testStream.str(), "D: Level 3 message\n");
}

TEST_F(DebugTest, PrintsWithTimestamp) {
    Debug::pt("Timestamped message");
    
    std::string output = testStream.str();
    EXPECT_TRUE(output.rfind("D[@", 0) == 0); // Dimulai dengan "D[@"
    EXPECT_NE(output.find("]: Timestamped message\n"), std::string::npos);
}

TEST_F(DebugTest, MeasuresExecutionTiming) {
    Debug::startTiming("ProcessingBatch");
    
    std::this_thread::sleep_for(std::chrono::milliseconds(15));
    
    Debug::doneTiming();

    std::string output = testStream.str();
    EXPECT_NE(output.find("ProcessingBatch took "), std::string::npos);
    EXPECT_NE(output.find("s\n"), std::string::npos);
}

TEST_F(DebugTest, AutoCompletesTimingWhenStartingNewOne) {
    Debug::startTiming("TaskA");
    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    Debug::startTiming("TaskB");
    
    std::string output = testStream.str();
    EXPECT_NE(output.find("TaskA took "), std::string::npos);

    testStream.str("");
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    Debug::doneTiming();

    output = testStream.str();
    EXPECT_NE(output.find("TaskB took "), std::string::npos);
}