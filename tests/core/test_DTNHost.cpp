/**
 * @file test_DTNHost.cpp
 * @brief Full Unit tests for the DTNHost class.
 * @details Tests the core functionalities of DTNHost including address generation,
 *          movement integration, routing integration, metric tracking, and comparisons.
 * @author Frathol
 * @date September 2026
 */

#include <gtest/gtest.h>
#include <fstream>
#include <cstdio>
#include <vector>
#include <memory>
#include <string>

#include "core/DTNHost.hpp"
#include "core/Configuration.hpp"
#include "core/Coord.hpp"
#include "movement/MovementModel.hpp"
#include "movement/Path.hpp"
#include "routing/MessageRouter.hpp"
#include "routing/community/Duration.hpp"

using namespace core;

// ============================================================================
// MOCKS & STUBS
// ============================================================================
// We create minimal implementations of the abstract MovementModel and MessageRouter
// to successfully instantiate a DTNHost during testing.

namespace movement {
    /**
     * @brief A stub MovementModel to provide basic movement capabilities for testing.
     */
    class DummyMovementModel : public MovementModel {
    public:
        explicit DummyMovementModel(const Configuration& config) : MovementModel(config) {}
        DummyMovementModel(const DummyMovementModel& mm) : MovementModel(mm) {}

        // Override Pure Virtuals
        Path getPath() override { 
            return Path(); // Assuming Path has a default constructor (verified by PathTest)
        }
        
        Coord getInitialLocation() override { 
            return Coord(10.0, 20.0); 
        }
        
        std::shared_ptr<MovementModel> replicate() const override { 
            return std::make_shared<DummyMovementModel>(*this); 
        }
    };
}

namespace routing {
    /**
     * @brief A stub MessageRouter to satisfy DTNHost's routing requirements.
     */
    class DummyRouter : public MessageRouter {
    public:
        explicit DummyRouter(Configuration& config) : MessageRouter(config) {}
        DummyRouter(const DummyRouter& r) : MessageRouter(r) {}

        // Override Pure Virtuals
        void changedConnection(Connection *con) override {
            // Do nothing for basic host tests
        }
        
        MessageRouter* replicate() override { 
            return new DummyRouter(*this); 
        }
    };
}

// ============================================================================
// TEST FIXTURES
// ============================================================================

class DTNHostTest : public ::testing::Test {
protected:
    std::string testConfigFile = "test_dtnhost.cfg";

    void SetUp() override {
        // 1. Create a dummy configuration file to satisfy MovementModel and MessageRouter
        std::ofstream out(testConfigFile);
        out << "MovementModel.worldSize = 1000, 1000\n"
            << "MovementModel.speed = 1.0, 2.0\n"
            << "MovementModel.waitTime = 0.0, 0.0\n"
            << "MessageRouter.bufferSize = 5000000\n"
            << "MessageRouter.msgTtl = 300\n";
        out.close();

        // 2. Initialize the global Configuration (sets isInitialized = true)
        Configuration::init(testConfigFile);

        // 3. Reset the static address counter so tests are isolated and deterministic
        DTNHost::reset();
    }

    void TearDown() override {
        // Clean up the temporary configuration file
        std::remove(testConfigFile.c_str());
    }

    /**
     * @brief Factory helper to quickly create a DTNHost for tests.
     * @param groupId The prefix name of the host (e.g., "Car", "Pedestrian").
     * @return Shared pointer to the newly created DTNHost.
     */
    std::shared_ptr<DTNHost> createTestHost(const std::string& groupId) {
        Configuration config;
        
        // Empty listeners
        std::vector<MessageListener*> msgLs;
        std::vector<MovementListener*> movLs;
        
        // Empty interfaces (Prevents the need to mock NetworkInterface)
        std::vector<std::shared_ptr<NetworkInterface>> interfaces;
        
        // Create prototypes
        auto movProto = std::make_shared<movement::DummyMovementModel>(config);
        auto routerProto = std::make_shared<routing::DummyRouter>(config);

        // Instantiate the host
        return std::make_shared<DTNHost>(
            msgLs, movLs, groupId, interfaces, nullptr, movProto, routerProto
        );
    }
};

// ============================================================================
// TEST CASES
// ============================================================================

/**
 * @brief Tests whether hosts generate incremental unique addresses and correct names.
 */
TEST_F(DTNHostTest, InitializationAndUniqueAddress) {
    auto host1 = createTestHost("Car");
    auto host2 = createTestHost("Car");
    auto host3 = createTestHost("Pedestrian");

    // Addresses must start from 0 and increment sequentially
    EXPECT_EQ(host1->getAddress(), 0);
    EXPECT_EQ(host2->getAddress(), 1);
    EXPECT_EQ(host3->getAddress(), 2);

    // String representation should concatenate GroupID + Address
    EXPECT_EQ(host1->toString(), "Car0");
    EXPECT_EQ(host2->toString(), "Car1");
    EXPECT_EQ(host3->toString(), "Pedestrian2");

    // Host should report as active based on MovementModel
    EXPECT_TRUE(host1->isActive());
}

/**
 * @brief Tests manual location updates and color property assignments.
 */
TEST_F(DTNHostTest, LocationAndColorProperties) {
    auto host = createTestHost("Node");

    // Test location setter and getter
    Coord newLoc(500.5, 300.2);
    host->setLocation(newLoc);
    
    EXPECT_DOUBLE_EQ(host->getLocation().getX(), 500.5);
    EXPECT_DOUBLE_EQ(host->getLocation().getY(), 300.2);

    // Test RGB color setter and getter
    std::vector<int> rgbColor = {255, 128, 0};
    host->setColor(rgbColor);
    
    auto retrievedColor = host->getColor();
    ASSERT_EQ(retrievedColor.size(), 3u);
    EXPECT_EQ(retrievedColor[0], 255);
    EXPECT_EQ(retrievedColor[1], 128);
    EXPECT_EQ(retrievedColor[2], 0);
}

/**
 * @brief Tests equality and comparison logic used for sorting hosts in sets/maps.
 */
TEST_F(DTNHostTest, HostComparisonAndEquality) {
    auto hostA = createTestHost("A"); // Gets Address 0
    auto hostB = createTestHost("B"); // Gets Address 1

    // equals() checks exact memory location
    EXPECT_TRUE(hostA->equals(hostA.get()));
    EXPECT_FALSE(hostA->equals(hostB.get()));

    // compareTo() subtracts addresses (0 - 1 = -1)
    EXPECT_LT(hostA->compareTo(hostB.get()), 0);
    EXPECT_GT(hostB->compareTo(hostA.get()), 0);

    // Operator < ensures they can be used in std::set or sorted vectors
    EXPECT_TRUE(*hostA < *hostB);
    EXPECT_FALSE(*hostB < *hostA);
}

/**
 * @brief Tests the Machine Learning / Metric tracking logic for contact durations.
 */
TEST_F(DTNHostTest, DurationMetricsAndIntervals) {
    auto host = createTestHost("Agent");

    // Create mock duration intervals
    routing::community::Duration dur1(10.0, 20.0);
    routing::community::Duration dur2(50.0, 60.5);

    host->addDuration(dur1);
    host->addDuration(dur2);

    // Verify the serialized output of the intervals
    std::string intervalsOutput = host->getNodeIntervals();
    EXPECT_NE(intervalsOutput.find("<10, 20>"), std::string::npos);
    EXPECT_NE(intervalsOutput.find("<50, 60.5>"), std::string::npos);
}

/**
 * @brief Ensures that executing standard update cycles does not cause a crash.
 */
TEST_F(DTNHostTest, UpdateAndMoveExecutionResilience) {
    auto host = createTestHost("SimNode");

    // Call update (simulateConnections = false)
    EXPECT_NO_THROW(host->update(false));
    
    // Call move (progress time by 1.0 second)
    EXPECT_NO_THROW(host->move(1.0));
}