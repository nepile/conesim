/**
 * @file test_SimpleBroadcastInterface.cpp
 * @brief Unit tests for SimpleBroadcastInterface.
 * @date September 2026
 */

#include <gtest/gtest.h>
#include <fstream>
#include <memory>
#include <string>
#include <vector>

#include "interfaces/SimpleBroadcastInterface.hpp"
#include "interfaces/ConnectivityGrid.hpp"
#include "core/DTNHost.hpp"
#include "core/Connection.hpp"
#include "core/ConnectionListener.hpp"
#include "core/Configuration.hpp"
#include "core/SimulationClock.hpp"
#include "movement/MovementModel.hpp"
#include "movement/Path.hpp"
#include "routing/MessageRouter.hpp"

using namespace core;
using namespace interfaces;

namespace {

class DummyMovementModel : public movement::MovementModel {
public:
    bool activeState = true;
    Coord loc = Coord(0.0, 0.0);

    explicit DummyMovementModel(const Configuration& config) : MovementModel(config) {}
    DummyMovementModel(const DummyMovementModel& mm)
        : MovementModel(mm), activeState(mm.activeState), loc(mm.loc) {}

    bool isActive() const override { return activeState; }
    movement::Path getPath() override { return movement::Path(); }
    Coord getInitialLocation() override { return loc; }
    std::shared_ptr<movement::MovementModel> replicate() const override {
        return std::make_shared<DummyMovementModel>(*this);
    }
};

class DummyRouter : public routing::MessageRouter {
public:
    explicit DummyRouter(Configuration& config) : MessageRouter(config) {}
    DummyRouter(const DummyRouter& r) : MessageRouter(r) {}

    void changedConnection(Connection*) override {}
    MessageRouter* replicate() override {
        return new DummyRouter(*this);
    }
};

class MockConnectionListener : public ConnectionListener {
public:
    int upCount = 0;
    int downCount = 0;

    void hostsConnected(const DTNHost&, const DTNHost&) override {
        ++upCount;
    }

    void hostDisconnected(const DTNHost&, const DTNHost&) override {
        ++downCount;
    }
};

} // namespace

class SimpleBroadcastInterfaceTest : public ::testing::Test {
protected:
    std::string testConfigFile = "test_simple_broadcast.cfg";

    void SetUp() override {
        std::ofstream out(testConfigFile);
        out << "MovementModel.worldSize = 1000, 1000\n"
            << "MovementModel.speed = 1.0, 2.0\n"
            << "MovementModel.waitTime = 0.0, 0.0\n"
            << "MessageRouter.bufferSize = 5000000\n"
            << "MessageRouter.msgTtl = 300\n"
            << "transmitRange = 50.0\n"
            << "transmitSpeed = 1000\n"
            << "scanInterval = 0.0\n"
            << "SimpleBroadcastInterface.transmitRange = 50.0\n"
            << "SimpleBroadcastInterface.transmitSpeed = 1000\n"
            << "SimpleBroadcastInterface.scanInterval = 0.0\n";
        out.close();

        Configuration::init(testConfigFile);
        SimulationClock::reset();
        DTNHost::reset();
        NetworkInterface::reset();
        ConnectivityGrid::reset();
    }

    void TearDown() override {
        std::remove(testConfigFile.c_str());
        SimulationClock::reset();
        ConnectivityGrid::reset();
    }

    std::shared_ptr<DTNHost> createHostWithLocation(
        const std::string& name,
        const Coord& loc,
        std::shared_ptr<NetworkInterface> netIf,
        bool active = true
    ) {
        Configuration config;
        std::vector<MessageListener*> msgLs;
        std::vector<MovementListener*> movLs;
        std::vector<std::shared_ptr<NetworkInterface>> interfaces = {netIf};

        auto mm = std::make_shared<DummyMovementModel>(config);
        mm->loc = loc;
        mm->activeState = active;
        auto router = std::make_shared<DummyRouter>(config);

        auto host = std::make_shared<DTNHost>(msgLs, movLs, name, interfaces, nullptr, mm, router);
        host->setLocation(loc);
        return host;
    }
};

TEST_F(SimpleBroadcastInterfaceTest, InitializationAndProperties) {
    Configuration config;
    SimpleBroadcastInterface sbi(config);

    EXPECT_DOUBLE_EQ(sbi.getTransmitRange(), 50.0);
    EXPECT_EQ(sbi.getTransmitSpeed(), 1000);
    EXPECT_TRUE(sbi.getConnections().empty());
    EXPECT_NE(sbi.toString().find("SimpleBroadcastInterface"), std::string::npos);
}

TEST_F(SimpleBroadcastInterfaceTest, ReplicateCreatesIndependentCopy) {
    Configuration config;
    SimpleBroadcastInterface proto(config);

    auto* copy = proto.replicate();
    ASSERT_NE(copy, nullptr);

    EXPECT_DOUBLE_EQ(copy->getTransmitRange(), proto.getTransmitRange());
    EXPECT_EQ(copy->getTransmitSpeed(), proto.getTransmitSpeed());
    EXPECT_NE(copy->getAddress(), proto.getAddress());

    delete copy;
}

TEST_F(SimpleBroadcastInterfaceTest, ConnectWithinRangeEstablishesConnection) {
    Configuration config;
    auto if1 = std::make_shared<SimpleBroadcastInterface>(config);
    auto if2 = std::make_shared<SimpleBroadcastInterface>(config);

    // Host1 at (0, 0), Host2 at (30, 0) -> distance = 30 <= range 50
    auto host1 = createHostWithLocation("H1", Coord(0.0, 0.0), if1);
    auto host2 = createHostWithLocation("H2", Coord(30.0, 0.0), if2);

    auto attachedIf1 = host1->getInterfaces()[0];
    auto attachedIf2 = host2->getInterfaces()[0];

    attachedIf1->connect(attachedIf2.get());

    EXPECT_EQ(attachedIf1->getConnections().size(), 1u);
    EXPECT_EQ(attachedIf2->getConnections().size(), 1u);

    Connection* con = attachedIf1->getConnections()[0];
    ASSERT_NE(con, nullptr);
    EXPECT_TRUE(con->isUp());
    EXPECT_DOUBLE_EQ(con->getSpeed(), 1000.0);
    EXPECT_EQ(con->getOtherNode(host1.get()), host2.get());
    EXPECT_EQ(con->getOtherNode(host2.get()), host1.get());

    // Calling connect again should not duplicate the connection
    attachedIf1->connect(attachedIf2.get());
    EXPECT_EQ(attachedIf1->getConnections().size(), 1u);
    EXPECT_EQ(attachedIf2->getConnections().size(), 1u);
}

TEST_F(SimpleBroadcastInterfaceTest, ConnectOutOfRangeFails) {
    Configuration config;
    auto if1 = std::make_shared<SimpleBroadcastInterface>(config);
    auto if2 = std::make_shared<SimpleBroadcastInterface>(config);

    // Host1 at (0, 0), Host2 at (100, 0) -> distance = 100 > range 50
    auto host1 = createHostWithLocation("H1", Coord(0.0, 0.0), if1);
    auto host2 = createHostWithLocation("H2", Coord(100.0, 0.0), if2);

    auto attachedIf1 = host1->getInterfaces()[0];
    auto attachedIf2 = host2->getInterfaces()[0];

    attachedIf1->connect(attachedIf2.get());

    EXPECT_EQ(attachedIf1->getConnections().size(), 0u);
    EXPECT_EQ(attachedIf2->getConnections().size(), 0u);
}

TEST_F(SimpleBroadcastInterfaceTest, DoesNotConnectWhenHostInactive) {
    Configuration config;
    auto if1 = std::make_shared<SimpleBroadcastInterface>(config);
    auto if2 = std::make_shared<SimpleBroadcastInterface>(config);

    // Host2 is inactive
    auto host1 = createHostWithLocation("H1", Coord(0.0, 0.0), if1, true);
    auto host2 = createHostWithLocation("H2", Coord(10.0, 0.0), if2, false);

    auto attachedIf1 = host1->getInterfaces()[0];
    auto attachedIf2 = host2->getInterfaces()[0];

    attachedIf1->connect(attachedIf2.get());

    EXPECT_EQ(attachedIf1->getConnections().size(), 0u);
    EXPECT_EQ(attachedIf2->getConnections().size(), 0u);
}

TEST_F(SimpleBroadcastInterfaceTest, CreateConnectionIgnoresRangeCheck) {
    Configuration config;
    auto if1 = std::make_shared<SimpleBroadcastInterface>(config);
    auto if2 = std::make_shared<SimpleBroadcastInterface>(config);

    // Distance 500 > range 50
    auto host1 = createHostWithLocation("H1", Coord(0.0, 0.0), if1);
    auto host2 = createHostWithLocation("H2", Coord(500.0, 0.0), if2);

    auto attachedIf1 = host1->getInterfaces()[0];
    auto attachedIf2 = host2->getInterfaces()[0];

    attachedIf1->createConnection(attachedIf2.get());

    EXPECT_EQ(attachedIf1->getConnections().size(), 1u);
    EXPECT_EQ(attachedIf2->getConnections().size(), 1u);
}

TEST_F(SimpleBroadcastInterfaceTest, UpdateLifecycleWithGridOptimizer) {
    Configuration config;
    auto if1 = std::make_shared<SimpleBroadcastInterface>(config);
    auto if2 = std::make_shared<SimpleBroadcastInterface>(config);

    MockConnectionListener listener;
    auto host1 = createHostWithLocation("H1", Coord(0.0, 0.0), if1);
    auto host2 = createHostWithLocation("H2", Coord(20.0, 0.0), if2);

    auto attachedIf1 = host1->getInterfaces()[0];
    auto attachedIf2 = host2->getInterfaces()[0];

    attachedIf1->setClisteners({&listener});

    // Both hosts within range 50. update() should detect and connect
    attachedIf1->update();
    EXPECT_EQ(attachedIf1->getConnections().size(), 1u);
    EXPECT_EQ(attachedIf2->getConnections().size(), 1u);
    EXPECT_EQ(listener.upCount, 1);

    // Move Host2 out of range (to x = 200)
    host2->setLocation(Coord(200.0, 0.0));
    attachedIf2->getLocation(); // location updated

    // Next update should disconnect
    attachedIf1->update();
    EXPECT_EQ(attachedIf1->getConnections().size(), 0u);
    EXPECT_EQ(attachedIf2->getConnections().size(), 0u);
    EXPECT_EQ(listener.downCount, 1);
}
