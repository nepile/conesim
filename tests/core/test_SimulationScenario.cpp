#include <gtest/gtest.h>
#include "core/SimulationScenario.hpp"
#include "core/Configuration.hpp"
#include "core/ConfigurationError.hpp"
#include <fstream>
#include <filesystem>

using namespace core;

class SimulationScenarioTest : public ::testing::Test {
protected:
    std::string testConfigFile = "test_scenario.cfg";

    void SetUp() override {
        std::ofstream out(testConfigFile);
        out << "Scenario.nrofHostGroups = 2\n"
            << "Scenario.name = TestScenario\n"
            << "Scenario.endTime = 1000.5\n"
            << "Scenario.updateInterval = 0.1\n"
            << "Scenario.simulateConnections = true\n"
            << "MovementModel.worldSize = 2500, 3000\n";
        out.close();

        // Initialize Configuration
        Configuration::init(testConfigFile);
    }

    void TearDown() override {
        // Clean up SimulationScenario instance
        SimulationScenario::reset();
        
        // Remove the temporary file
        std::remove(testConfigFile.c_str());
    }
};

TEST_F(SimulationScenarioTest, InitializationTest) {
    // Calling getInstance should invoke the constructor which reads from Configuration
    auto scenario = SimulationScenario::getInstance();
    
    EXPECT_EQ(scenario->getName(), "TestScenario");
    EXPECT_DOUBLE_EQ(scenario->getEndTime(), 1000.5);
    EXPECT_DOUBLE_EQ(scenario->getUpdateInterval(), 0.1);
    EXPECT_TRUE(scenario->simulateConnections());
    EXPECT_EQ(scenario->getWorldSizeX(), 2500);
    EXPECT_EQ(scenario->getWorldSizeY(), 3000);
}

TEST_F(SimulationScenarioTest, SingletonTest) {
    auto scenario1 = SimulationScenario::getInstance();
    auto scenario2 = SimulationScenario::getInstance();
    
    // Both should point to the exact same instance in memory
    EXPECT_EQ(scenario1.get(), scenario2.get());
}

TEST_F(SimulationScenarioTest, EnsurePositiveValueThrowsOnNegative) {
    std::string badConfigFile = "test_scenario_bad.cfg";
    std::ofstream out(badConfigFile);
    out << "Scenario.nrofHostGroups = -1\n" // Negative value should throw
        << "Scenario.name = TestScenario\n"
        << "Scenario.endTime = 1000.5\n"
        << "Scenario.updateInterval = 0.1\n"
        << "Scenario.simulateConnections = true\n"
        << "MovementModel.worldSize = 2500, 3000\n";
    out.close();

    Configuration::init(badConfigFile);
    SimulationScenario::reset();

    // The constructor should throw a ConfigurationError because nrofGroups < 0
    EXPECT_THROW(SimulationScenario::getInstance(), core::ConfigurationError);
    
    std::remove(badConfigFile.c_str());
}
