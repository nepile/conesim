#include <gtest/gtest.h>
#include "movement/RandomWaypoint.hpp"
#include "core/Configuration.hpp"
#include "core/SimulationError.hpp"
#include "core/Coord.hpp"
#include "movement/Path.hpp"
#include <fstream>
#include <memory>

using namespace movement;
using namespace core;

class RandomWaypointTest : public ::testing::Test {
protected:
    std::string testConfigFile = "test_rwp_settings.cfg";

    void SetUp() override {
        std::ofstream out(testConfigFile);
        out << "MovementModel.worldSize = 1000, 1000\n";
        out << "MovementModel.rngSeed = 42\n";
        out << "MovementModel.speed = 2.0, 5.0\n";
        out << "MovementModel.waitTime = 10.0, 20.0\n";
        out.close();
        
        core::Configuration::init(testConfigFile);
    }

    void TearDown() override {
        std::remove(testConfigFile.c_str());
    }
};

TEST_F(RandomWaypointTest, InitializationTest) {
    Configuration config("MovementModel");
    MovementModel::reset();

    RandomWaypoint rwp(config);
    
    EXPECT_EQ(rwp.getMaxX(), 1000);
    EXPECT_EQ(rwp.getMaxY(), 1000);
}

TEST_F(RandomWaypointTest, GetInitialLocationWithinBounds) {
    Configuration config("MovementModel");
    MovementModel::reset();

    RandomWaypoint rwp(config);
    Coord initialLoc = rwp.getInitialLocation();
    
    EXPECT_GE(initialLoc.getX(), 0.0);
    EXPECT_LE(initialLoc.getX(), 1000.0);
    EXPECT_GE(initialLoc.getY(), 0.0);
    EXPECT_LE(initialLoc.getY(), 1000.0);
}

TEST_F(RandomWaypointTest, GetPathTest) {
    Configuration config("MovementModel");
    MovementModel::reset();

    RandomWaypoint rwp(config);
    rwp.getInitialLocation(); 
    
    Path p = rwp.getPath();
    
    EXPECT_EQ(p.getCoords().size(), 2);
    
    p.getNextWaypoint();
    double speed = p.getSpeed();
    EXPECT_GE(speed, 2.0);
    EXPECT_LE(speed, 5.0);
}

TEST_F(RandomWaypointTest, ReplicateCreatesIndependentCopy) {
    Configuration config("MovementModel");
    MovementModel::reset();

    RandomWaypoint rwp(config);
    auto copy = rwp.replicate();
    
    EXPECT_EQ(copy->getMaxX(), 1000);
    EXPECT_EQ(copy->getMaxY(), 1000);
    
    Coord c = copy->getInitialLocation();
    EXPECT_GE(c.getX(), 0.0);
    EXPECT_LE(c.getX(), 1000.0);
}
