#include <gtest/gtest.h>

#include "core/SimulationClock.hpp"

using namespace core;

class SimulationClockTest : public ::testing::Test {
protected:
    void SetUp() override {
        SimulationClock::reset();
    }

    void TearDown() override {
        SimulationClock::reset();
    }
};

TEST_F(SimulationClockTest, InitialTimeIsZero) {
    EXPECT_DOUBLE_EQ(SimulationClock::getTime(), 0.0);
}

TEST_F(SimulationClockTest, AdvanceTime) {
    SimulationClock* clock = SimulationClock::getInstance();

    clock->advance(10.0);

    EXPECT_DOUBLE_EQ(SimulationClock::getTime(), 10.0);
}

TEST_F(SimulationClockTest, AdvanceMultipleTimes) {
    SimulationClock* clock = SimulationClock::getInstance();

    clock->advance(5.0);
    clock->advance(10.0);
    clock->advance(2.5);

    EXPECT_DOUBLE_EQ(SimulationClock::getTime(), 17.5);
}

TEST_F(SimulationClockTest, SetTime) {
    SimulationClock* clock = SimulationClock::getInstance();

    clock->setTime(42.5);

    EXPECT_DOUBLE_EQ(SimulationClock::getTime(), 42.5);
}

TEST_F(SimulationClockTest, ResetTime) {
    SimulationClock* clock = SimulationClock::getInstance();

    clock->advance(100.0);
    SimulationClock::reset();

    EXPECT_DOUBLE_EQ(SimulationClock::getTime(), 0.0);
}

TEST_F(SimulationClockTest, GetIntTimeRoundsCorrectly) {
    SimulationClock* clock = SimulationClock::getInstance();

    clock->setTime(10.4);
    EXPECT_EQ(SimulationClock::getIntTime(), 10);

    clock->setTime(10.5);
    EXPECT_EQ(SimulationClock::getIntTime(), 11);

    clock->setTime(10.8);
    EXPECT_EQ(SimulationClock::getIntTime(), 11);
}

TEST_F(SimulationClockTest, ToString) {
    SimulationClock* clock = SimulationClock::getInstance();

    clock->setTime(12.5);

    EXPECT_EQ(clock->toString(), "SimTime: 12.500000");
}