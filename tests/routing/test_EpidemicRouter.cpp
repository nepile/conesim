/**
 * @file test_EpidemicRouter.cpp
 * @brief Unit tests for EpidemicRouter.
 * @date September 2026
 */

#include <gtest/gtest.h>
#include <fstream>
#include <memory>
#include <string>
#include <vector>

#include "routing/EpidemicRouter.hpp"
#include "interfaces/SimpleBroadcastInterface.hpp"
#include "core/DTNHost.hpp"
#include "core/Message.hpp"
#include "core/CBRConnection.hpp"
#include "core/Configuration.hpp"
#include "core/SimulationClock.hpp"
#include "movement/MovementModel.hpp"
#include "movement/Path.hpp"

using namespace core;
using namespace routing;
using namespace interfaces;

namespace {

class DummyMovementModel : public movement::MovementModel {
public:
    explicit DummyMovementModel(const Configuration& config) : MovementModel(config) {}
    DummyMovementModel(const DummyMovementModel& mm) : MovementModel(mm) {}

    movement::Path getPath() override { return movement::Path(); }
    Coord getInitialLocation() override { return Coord(0.0, 0.0); }
    std::shared_ptr<movement::MovementModel> replicate() const override {
        return std::make_shared<DummyMovementModel>(*this);
    }
};

} // namespace

class EpidemicRouterTest : public ::testing::Test {
protected:
    std::string testConfigFile = "test_epidemic_router.cfg";

    void SetUp() override {
        std::ofstream out(testConfigFile);
        out << "MovementModel.worldSize = 1000, 1000\n"
            << "MovementModel.speed = 1.0, 2.0\n"
            << "MovementModel.waitTime = 0.0, 0.0\n"
            << "MessageRouter.bufferSize = 5000000\n"
            << "MessageRouter.msgTtl = 300\n"
            << "transmitRange = 100.0\n"
            << "transmitSpeed = 1000\n"
            << "scanInterval = 0.0\n"
            << "SimpleBroadcastInterface.transmitRange = 100.0\n"
            << "SimpleBroadcastInterface.transmitSpeed = 1000\n"
            << "SimpleBroadcastInterface.scanInterval = 0.0\n";
        out.close();

        Configuration::init(testConfigFile);
        SimulationClock::reset();
        DTNHost::reset();
    }

    void TearDown() override {
        std::remove(testConfigFile.c_str());
        SimulationClock::reset();
    }

    std::shared_ptr<DTNHost> createHostWithRouter(const std::string& name, const Coord& loc) {
        Configuration config;
        std::vector<MessageListener*> msgLs;
        std::vector<MovementListener*> movLs;
        auto netIf = std::make_shared<SimpleBroadcastInterface>(config);
        std::vector<std::shared_ptr<NetworkInterface>> interfaces = {netIf};

        auto mm = std::make_shared<DummyMovementModel>(config);
        auto router = std::make_shared<EpidemicRouter>(config);

        auto host = std::make_shared<DTNHost>(msgLs, movLs, name, interfaces, nullptr, mm, router);
        host->setLocation(loc);
        return host;
    }

    ActiveRouter* getActiveRouter(const std::shared_ptr<DTNHost>& host) {
        return dynamic_cast<ActiveRouter*>(host->getRouter().get());
    }
};

TEST_F(EpidemicRouterTest, InitializationAndDefaultState) {
    Configuration config;
    EpidemicRouter router(config);

    EXPECT_FALSE(router.isTransferring());
    EXPECT_EQ(router.getNrofMessages(), 0);
    EXPECT_EQ(router.getBufferSize(), 10000000);

    MessageRouter* replica = router.replicate();
    ASSERT_NE(replica, nullptr);
    EXPECT_FALSE(dynamic_cast<EpidemicRouter*>(replica)->isTransferring());
    EXPECT_EQ(replica->getNrofMessages(), 0);

    delete replica;
}

TEST_F(EpidemicRouterTest, PrioritizesDirectDeliveryToRecipient) {
    auto hostA = createHostWithRouter("A", Coord(0.0, 0.0));
    auto hostB = createHostWithRouter("B", Coord(10.0, 0.0));
    auto hostC = createHostWithRouter("C", Coord(500.0, 0.0)); // Far away

    // Connect Host A and Host B
    hostA->getInterfaces()[0]->connect(hostB->getInterfaces()[0].get());
    ASSERT_EQ(hostA->getConnections().size(), 1u);

    // Host A has 2 messages:
    // MSG_RELAY: destined for Host C (not connected)
    // MSG_DIRECT: destined for Host B (directly connected)
    auto msgRelay = std::make_shared<Message>(hostA.get(), hostC.get(), "MSG_RELAY", 100);
    auto msgDirect = std::make_shared<Message>(hostA.get(), hostB.get(), "MSG_DIRECT", 100);

    hostA->createNewMessage(msgRelay);
    hostA->createNewMessage(msgDirect);
    EXPECT_EQ(hostA->getNrofMessages(), 2);

    // Host A updates its router
    hostA->getRouter()->update();

    // The router should prioritize MSG_DIRECT and start transferring it
    auto con = hostA->getConnections()[0];
    EXPECT_TRUE(getActiveRouter(hostA)->isSending("MSG_DIRECT"));
    EXPECT_FALSE(getActiveRouter(hostA)->isSending("MSG_RELAY"));
    EXPECT_EQ(con->getMessage()->getId(), "MSG_DIRECT");
}

TEST_F(EpidemicRouterTest, RelaysMessageWhenNoDirectDeliveryAvailable) {
    auto hostA = createHostWithRouter("A", Coord(0.0, 0.0));
    auto hostB = createHostWithRouter("B", Coord(10.0, 0.0));
    auto hostC = createHostWithRouter("C", Coord(500.0, 0.0)); // Far away

    // Connect Host A and Host B
    hostA->getInterfaces()[0]->connect(hostB->getInterfaces()[0].get());
    ASSERT_EQ(hostA->getConnections().size(), 1u);

    // Host A only has message for Host C
    auto msgForC = std::make_shared<Message>(hostA.get(), hostC.get(), "MSG_FOR_C", 200);
    hostA->createNewMessage(msgForC);

    // Update should trigger epidemic spread (forward message to relay Host B)
    hostA->getRouter()->update();

    EXPECT_TRUE(getActiveRouter(hostA)->isSending("MSG_FOR_C"));
    auto con = hostA->getConnections()[0];
    ASSERT_NE(con->getMessage(), nullptr);
    EXPECT_EQ(con->getMessage()->getId(), "MSG_FOR_C");
}

TEST_F(EpidemicRouterTest, DoesNotStartTransferWhenAlreadyTransferring) {
    auto hostA = createHostWithRouter("A", Coord(0.0, 0.0));
    auto hostB = createHostWithRouter("B", Coord(10.0, 0.0));
    auto hostC = createHostWithRouter("C", Coord(15.0, 0.0));

    // Connect Host A to Host B
    hostA->getInterfaces()[0]->connect(hostB->getInterfaces()[0].get());

    auto msg1 = std::make_shared<Message>(hostA.get(), hostB.get(), "MSG_1", 500);
    auto msg2 = std::make_shared<Message>(hostA.get(), hostC.get(), "MSG_2", 500);

    hostA->createNewMessage(msg1);
    hostA->createNewMessage(msg2);

    // First update starts transfer of MSG_1
    hostA->getRouter()->update();
    EXPECT_TRUE(getActiveRouter(hostA)->isSending("MSG_1"));

    // Second update while transfer in progress should not disrupt or start another transfer
    hostA->getRouter()->update();
    EXPECT_TRUE(getActiveRouter(hostA)->isSending("MSG_1"));
    EXPECT_FALSE(getActiveRouter(hostA)->isSending("MSG_2"));
}

TEST_F(EpidemicRouterTest, DoesNotSendIfPeerAlreadyHasMessage) {
    auto hostA = createHostWithRouter("A", Coord(0.0, 0.0));
    auto hostB = createHostWithRouter("B", Coord(10.0, 0.0));
    auto hostC = createHostWithRouter("C", Coord(500.0, 0.0));

    auto msg = std::make_shared<Message>(hostA.get(), hostC.get(), "MSG_SHARED", 100);

    // Both Host A and Host B already have MSG_SHARED
    hostA->createNewMessage(msg);
    hostB->createNewMessage(msg);

    hostA->getInterfaces()[0]->connect(hostB->getInterfaces()[0].get());

    // Host A updates: B already has MSG_SHARED, so transfer should NOT start
    hostA->getRouter()->update();

    EXPECT_FALSE(getActiveRouter(hostA)->isSending("MSG_SHARED"));
    EXPECT_FALSE(getActiveRouter(hostA)->isTransferring());
}

TEST_F(EpidemicRouterTest, MessageTransmissionCompletionAndReception) {
    auto hostA = createHostWithRouter("A", Coord(0.0, 0.0));
    auto hostB = createHostWithRouter("B", Coord(10.0, 0.0));

    hostA->getInterfaces()[0]->connect(hostB->getInterfaces()[0].get());

    // Message size 1000 bytes, speed 1000 Bps -> takes 1.0 second
    auto msg = std::make_shared<Message>(hostA.get(), hostB.get(), "MSG_FINAL", 1000);
    hostA->createNewMessage(msg);

    // Start transfer
    hostA->getRouter()->update();
    EXPECT_TRUE(getActiveRouter(hostA)->isSending("MSG_FINAL"));
    EXPECT_EQ(hostB->getNrofMessages(), 0);

    // Advance clock by 1.0s to complete transfer
    SimulationClock::getInstance()->advance(1.0);

    // Host A update finalizes transfer
    hostA->getRouter()->update();

    EXPECT_FALSE(getActiveRouter(hostA)->isSending("MSG_FINAL"));
    // Host B has now received the message
    EXPECT_EQ(hostB->getNrofMessages(), 1);
    EXPECT_EQ(hostB->getMessageCollection()[0]->getId(), "MSG_FINAL");
}
