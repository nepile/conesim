/**
 * @file test_ActiveRouter.cpp
 * @brief Unit tests for the ActiveRouter base class.
 * @date September 2026
 */

#include <gtest/gtest.h>
#include "routing/ActiveRouter.hpp"
#include "core/Configuration.hpp"
#include "core/Message.hpp"
#include "core/DTNHost.hpp"
#include "core/SimulationClock.hpp"

#include <memory>
#include <vector>

using namespace routing;
using namespace core;

class DummyActiveRouter : public ActiveRouter {
public:
    explicit DummyActiveRouter(Configuration &s) : ActiveRouter(s) {}
    DummyActiveRouter(const DummyActiveRouter &r) : ActiveRouter(r) {}

    MessageRouter *replicate() override {
        return new DummyActiveRouter(*this);
    }

    using ActiveRouter::canStartTransfer;
    using ActiveRouter::hasMessage;
    using ActiveRouter::dropExpiredMessages;
    using ActiveRouter::getOldestMessage;
};

class ActiveRouterTest : public ::testing::Test {
protected:
    void SetUp() override {
        SimulationClock::reset();
    }

    void TearDown() override {
        SimulationClock::reset();
    }
};

TEST_F(ActiveRouterTest, InitializationAndDefaultSettings) {
    Configuration conf("");
    DummyActiveRouter router(conf);

    EXPECT_FALSE(router.isTransferring());
    EXPECT_FALSE(router.canStartTransfer());
    EXPECT_EQ(router.getNrofMessages(), 0);
}

TEST_F(ActiveRouterTest, CreateNewMessageAndRoomAllocation) {
    Configuration conf("");
    DummyActiveRouter router(conf);

    DTNHost* hostA = reinterpret_cast<DTNHost*>(0x10);
    DTNHost* hostB = reinterpret_cast<DTNHost*>(0x20);

    auto msg1 = std::make_shared<Message>(hostA, hostB, "MSG_1", 100);
    bool created = router.createNewMessage(msg1);

    EXPECT_TRUE(created);
    EXPECT_EQ(router.getNrofMessages(), 1);
    EXPECT_TRUE(router.hasMessage("MSG_1"));
}

TEST_F(ActiveRouterTest, OldestMessageSelection) {
    Configuration conf("");
    DummyActiveRouter router(conf);

    DTNHost* hostA = reinterpret_cast<DTNHost*>(0x10);
    DTNHost* hostB = reinterpret_cast<DTNHost*>(0x20);

    auto msg1 = std::make_shared<Message>(hostA, hostB, "MSG_1", 100);
    msg1->setReceiveTime(1.0);
    router.createNewMessage(msg1);

    auto msg2 = std::make_shared<Message>(hostA, hostB, "MSG_2", 100);
    msg2->setReceiveTime(2.0);
    router.createNewMessage(msg2);

    EXPECT_EQ(router.getNrofMessages(), 2);
    EXPECT_TRUE(router.hasMessage("MSG_1"));
    EXPECT_TRUE(router.hasMessage("MSG_2"));
    EXPECT_EQ(router.getOldestMessage(false)->getId(), "MSG_1");
}

TEST_F(ActiveRouterTest, DropExpiredMessagesOnTtlZero) {
    Configuration conf("");
    DummyActiveRouter router(conf);

    DTNHost* hostA = reinterpret_cast<DTNHost*>(0x10);
    DTNHost* hostB = reinterpret_cast<DTNHost*>(0x20);

    auto msg = std::make_shared<Message>(hostA, hostB, "EXPIRED_MSG", 100);
    msg->setTtl(0); // Expired TTL

    router.createNewMessage(msg);
    EXPECT_EQ(router.getNrofMessages(), 1);

    // Advance clock beyond TTL check interval
    SimulationClock::getInstance()->advance(ActiveRouter::TTL_CHECK_INTERVAL + 1.0);
    router.update();

    // Expired message should be dropped
    EXPECT_EQ(router.getNrofMessages(), 0);
    EXPECT_FALSE(router.hasMessage("EXPIRED_MSG"));
}
