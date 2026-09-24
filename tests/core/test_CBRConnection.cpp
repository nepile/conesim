/**
 * @file test_CBRConnection.cpp
 * @brief Unit tests for CBRConnection.
 * @date September 2026
 */

#include <gtest/gtest.h>
#include <fstream>
#include <memory>
#include <string>

#include "core/CBRConnection.hpp"
#include "core/DTNHost.hpp"
#include "core/NetworkInterface.hpp"
#include "core/Message.hpp"
#include "core/SimulationClock.hpp"
#include "core/Configuration.hpp"
#include "movement/MovementModel.hpp"
#include "movement/Path.hpp"
#include "routing/MessageRouter.hpp"

using namespace core;

namespace {

class CBRDummyMovementModel : public movement::MovementModel {
public:
    explicit CBRDummyMovementModel(const Configuration& config) : MovementModel(config) {}
    CBRDummyMovementModel(const CBRDummyMovementModel& mm) : MovementModel(mm) {}

    movement::Path getPath() override { return movement::Path(); }
    Coord getInitialLocation() override { return Coord(0.0, 0.0); }
    std::shared_ptr<movement::MovementModel> replicate() const override {
        return std::make_shared<CBRDummyMovementModel>(*this);
    }
};

class CBRDummyRouter : public routing::MessageRouter {
public:
    int receiveMessageReturnValue = routing::MessageRouter::RCV_OK;

    explicit CBRDummyRouter(Configuration& config) : MessageRouter(config) {}
    CBRDummyRouter(const CBRDummyRouter& r) : MessageRouter(r), receiveMessageReturnValue(r.receiveMessageReturnValue) {}

    void changedConnection(Connection*) override {}
    MessageRouter* replicate() override {
        return new CBRDummyRouter(*this);
    }

    int receiveMessage(Message* m, DTNHost* from) override {
        if (receiveMessageReturnValue == routing::MessageRouter::RCV_OK) {
            return MessageRouter::receiveMessage(m, from);
        }
        return receiveMessageReturnValue;
    }
};

class CBRDummyInterface : public NetworkInterface {
public:
    explicit CBRDummyInterface(const Configuration& config) : NetworkInterface(config) {}
    NetworkInterface* replicate() override {
        return new CBRDummyInterface(*this);
    }
    void update() override {}
    void connect(NetworkInterface*) override {}
    void createConnection(NetworkInterface*) override {}
};

} // namespace

class CBRConnectionTest : public ::testing::Test {
protected:
    std::string testConfigFile = "test_cbr_conn.cfg";

    void SetUp() override {
        std::ofstream out(testConfigFile);
        out << "MovementModel.worldSize = 1000, 1000\n"
            << "MovementModel.speed = 1.0, 2.0\n"
            << "MovementModel.waitTime = 0.0, 0.0\n"
            << "MessageRouter.bufferSize = 5000000\n"
            << "MessageRouter.msgTtl = 300\n"
            << "transmitRange = 100.0\n"
            << "transmitSpeed = 1024\n"
            << "NetworkInterface.transmitRange = 100.0\n"
            << "NetworkInterface.transmitSpeed = 1024\n"
            << "CBRDummyInterface.transmitRange = 100.0\n"
            << "CBRDummyInterface.transmitSpeed = 1024\n";
        out.close();

        Configuration::init(testConfigFile);
        SimulationClock::reset();
        DTNHost::reset();
    }

    void TearDown() override {
        std::remove(testConfigFile.c_str());
        SimulationClock::reset();
    }

    std::shared_ptr<DTNHost> createHost(const std::string& name) {
        Configuration config;
        std::vector<MessageListener*> msgLs;
        std::vector<MovementListener*> movLs;
        auto netIf = std::make_shared<CBRDummyInterface>(config);
        std::vector<std::shared_ptr<NetworkInterface>> interfaces = {netIf};
        auto mm = std::make_shared<CBRDummyMovementModel>(config);
        auto router = std::make_shared<CBRDummyRouter>(config);

        return std::make_shared<DTNHost>(msgLs, movLs, name, interfaces, nullptr, mm, router);
    }
};

TEST_F(CBRConnectionTest, InitializationAndProperties) {
    auto host1 = createHost("NodeA");
    auto host2 = createHost("NodeB");

    auto if1 = host1->getInterfaces()[0].get();
    auto if2 = host2->getInterfaces()[0].get();

    CBRConnection conn(host1.get(), if1, host2.get(), if2, 500);

    EXPECT_TRUE(conn.isUp());
    EXPECT_DOUBLE_EQ(conn.getSpeed(), 500.0);
    EXPECT_TRUE(conn.isReadyForTransfer());
    EXPECT_EQ(conn.getMessage(), nullptr);
    EXPECT_EQ(conn.getRemainingByteCount(), 0);
    EXPECT_EQ(conn.getTotalBytesTransferred(), 0);
    EXPECT_DOUBLE_EQ(conn.getTransferDoneTime(), 0.0);
    EXPECT_TRUE(conn.isInitiator(host1.get()));
    EXPECT_FALSE(conn.isInitiator(host2.get()));
}

TEST_F(CBRConnectionTest, StartTransferCalculatesDoneTime) {
    auto host1 = createHost("NodeA");
    auto host2 = createHost("NodeB");

    auto if1 = host1->getInterfaces()[0].get();
    auto if2 = host2->getInterfaces()[0].get();

    // Speed: 100 Bps
    CBRConnection conn(host1.get(), if1, host2.get(), if2, 100);

    // Message size: 500 bytes -> transfer duration should be 500 / 100 = 5.0 seconds
    auto msg = std::make_shared<Message>(host1.get(), host2.get(), "M1", 500);

    int ret = conn.startTransfer(host1.get(), msg);
    EXPECT_EQ(ret, routing::MessageRouter::RCV_OK);
    EXPECT_FALSE(conn.isReadyForTransfer());
    EXPECT_NE(conn.getMessage(), nullptr);
    EXPECT_DOUBLE_EQ(conn.getTransferDoneTime(), 5.0);
    EXPECT_EQ(conn.getRemainingByteCount(), 500);
    EXPECT_FALSE(conn.isMessageTransferred());

    // Advance time by 2.0s -> remaining time 3.0s -> remaining bytes: 300
    SimulationClock::getInstance()->advance(2.0);
    EXPECT_EQ(conn.getRemainingByteCount(), 300);
    EXPECT_FALSE(conn.isMessageTransferred());

    // Advance time to 5.0s -> remaining bytes: 0 -> transfer done
    SimulationClock::getInstance()->advance(3.0);
    EXPECT_EQ(conn.getRemainingByteCount(), 0);
    EXPECT_TRUE(conn.isMessageTransferred());
}

TEST_F(CBRConnectionTest, FinalizeTransferCompletesSuccessfully) {
    auto host1 = createHost("NodeA");
    auto host2 = createHost("NodeB");

    auto if1 = host1->getInterfaces()[0].get();
    auto if2 = host2->getInterfaces()[0].get();

    CBRConnection conn(host1.get(), if1, host2.get(), if2, 200);
    auto msg = std::make_shared<Message>(host1.get(), host2.get(), "M2", 400);

    conn.startTransfer(host1.get(), msg);
    SimulationClock::getInstance()->advance(2.0); // 400 / 200 = 2.0s
    EXPECT_TRUE(conn.isMessageTransferred());

    conn.finalizeTransfer();

    EXPECT_EQ(conn.getMessage(), nullptr);
    EXPECT_TRUE(conn.isReadyForTransfer());
    EXPECT_EQ(conn.getTotalBytesTransferred(), 400);
    EXPECT_DOUBLE_EQ(conn.getTransferDoneTime(), 0.0);
}

TEST_F(CBRConnectionTest, AbortTransferAccumulatesPartialBytes) {
    auto host1 = createHost("NodeA");
    auto host2 = createHost("NodeB");

    auto if1 = host1->getInterfaces()[0].get();
    auto if2 = host2->getInterfaces()[0].get();

    CBRConnection conn(host1.get(), if1, host2.get(), if2, 100);
    auto msg = std::make_shared<Message>(host1.get(), host2.get(), "M3", 500);

    conn.startTransfer(host1.get(), msg);
    SimulationClock::getInstance()->advance(2.0); // 200 bytes transferred, 300 remaining

    conn.abortTransfer();

    EXPECT_EQ(conn.getMessage(), nullptr);
    EXPECT_TRUE(conn.isReadyForTransfer());
    EXPECT_EQ(conn.getTotalBytesTransferred(), 200);
    EXPECT_DOUBLE_EQ(conn.getTransferDoneTime(), 0.0);
}

TEST_F(CBRConnectionTest, ToStringFormat) {
    auto host1 = createHost("NodeA");
    auto host2 = createHost("NodeB");

    auto if1 = host1->getInterfaces()[0].get();
    auto if2 = host2->getInterfaces()[0].get();

    CBRConnection conn(host1.get(), if1, host2.get(), if2, 250);
    std::string strBefore = conn.toString();
    EXPECT_NE(strBefore.find("250"), std::string::npos);
    EXPECT_NE(strBefore.find("up"), std::string::npos);

    auto msg = std::make_shared<Message>(host1.get(), host2.get(), "M4", 500);
    conn.startTransfer(host1.get(), msg);

    std::string strDuring = conn.toString();
    EXPECT_NE(strDuring.find("transferring M4"), std::string::npos);
    EXPECT_NE(strDuring.find("until"), std::string::npos);
}
