#include <gtest/gtest.h>
#include "core/Configuration.hpp"
#include <fstream>
#include <filesystem>
#include "core/SimulationError.hpp"
#include "core/ConfigurationError.hpp"

using namespace conesim;

class ConfigurationTest : public ::testing::Test {
protected:
    void SetUp() override {
        std::ofstream defFile("test_default.cfg");
        defFile << "Group.router = EpidemicRouter\n"
                << "Group.bufferSize = 10M\n"
                << "Group.speed = 1.5, 4.5\n"
                << "MovementModel.rngSeed = 100\n";
        defFile.close();

        std::ofstream runFile("test_run.cfg");
        runFile << "Scenario.name = test_scenario\n"
                << "Group.nrofHosts = [10; 20; 30]\n"
                << "Report.enabled = true\n"
                << "Scenario.tag = sim_%%Group.router%%\n";
        runFile.close();

        Configuration::init("test_run.cfg");
        Configuration::addSettings("test_default.cfg");
    }

    void TearDown() override {
        std::filesystem::remove("test_default.cfg");
        std::filesystem::remove("test_run.cfg");
        Configuration::setRunIndex(0);
    }
};

TEST_F(ConfigurationTest, ParsesNumericMultipliers) {
    Configuration conf;
    EXPECT_DOUBLE_EQ(conf.getDouble("Group.bufferSize"), 10000000.0);
    EXPECT_EQ(conf.getInt("Group.bufferSize"), 10000000);
}

TEST_F(ConfigurationTest, ParsesBoolean) {
    Configuration conf;
    EXPECT_TRUE(conf.getBoolean("Report.enabled"));
    EXPECT_FALSE(conf.getBoolean("NonExistent.key", false));
}

TEST_F(ConfigurationTest, ResolvesNamespaces) {
    Configuration conf("Group");
    conf.setSecondaryNamespace("MovementModel");
    EXPECT_EQ(conf.getConfiguration("router"), "EpidemicRouter");
    EXPECT_EQ(conf.getInt("rngSeed"), 100);
}

TEST_F(ConfigurationTest, HandlesRunIndex) {
    Configuration conf;
    
    Configuration::setRunIndex(0);
    EXPECT_EQ(conf.getInt("Group.nrofHosts"), 10);

    Configuration::setRunIndex(1);
    EXPECT_EQ(conf.getInt("Group.nrofHosts"), 20);

    Configuration::setRunIndex(3);
    EXPECT_EQ(conf.getInt("Group.nrofHosts"), 10);
}

TEST_F(ConfigurationTest, ParsesCsvSettings) {
    Configuration conf;
    std::vector<double> speed = conf.getCsvDoubles("Group.speed", 2);
    ASSERT_EQ(speed.size(), 2);
    EXPECT_DOUBLE_EQ(speed[0], 1.5);
    EXPECT_DOUBLE_EQ(speed[1], 4.5);
}

TEST_F(ConfigurationTest, ReplacesDelimiters) {
    Configuration conf;
    std::string filled = conf.valueFillString("run_%%Group.router%%_output");
    EXPECT_EQ(filled, "run_EpidemicRouter_output");
}

TEST_F(ConfigurationTest, ThrowsConfigurationErrorOnMissingKey) {
    Configuration conf;
    EXPECT_THROW(conf.getConfiguration("NonExistent.Key"), ConfigurationError);
    EXPECT_THROW(conf.getConfiguration("NonExistent.Key"), SimulationError);
}

TEST_F(ConfigurationTest, ThrowsConfigurationErrorOnInvalidNumeric) {
    Configuration conf;
    Configuration::addSetting("Test.badNumber", "123XYZ");
    EXPECT_THROW(conf.getInt("Test.badNumber"), ConfigurationError);
}