/**
 * @file test_PassiveRouter.cpp
 * @brief Unit tests for the PassiveRouter class.
 * @date September 2026
 */

#include <gtest/gtest.h>
#include "routing/PassiveRouter.hpp"
#include "core/Configuration.hpp"
#include "core/SimulationClock.hpp"

using namespace routing;
using namespace core;

class PassiveRouterTest : public ::testing::Test {
protected:
    void SetUp() override {
        SimulationClock::reset();
    }

    void TearDown() override {
        SimulationClock::reset();
    }
};

TEST_F(PassiveRouterTest, InitializationAndDefaultState) {
    Configuration conf("");
    PassiveRouter router(conf);

    EXPECT_EQ(router.getNrofMessages(), 0);
}

TEST_F(PassiveRouterTest, ReplicateCreatesIndependentCopy) {
    Configuration conf("");
    PassiveRouter prototype(conf);

    MessageRouter* replica = prototype.replicate();
    ASSERT_NE(replica, nullptr);

    PassiveRouter* passiveReplica = dynamic_cast<PassiveRouter*>(replica);
    EXPECT_NE(passiveReplica, nullptr);

    delete replica;
}

TEST_F(PassiveRouterTest, UpdateAndChangedConnectionExecution) {
    Configuration conf("");
    PassiveRouter router(conf);

    EXPECT_NO_THROW(router.update());
    EXPECT_NO_THROW(router.changedConnection(nullptr));
}
