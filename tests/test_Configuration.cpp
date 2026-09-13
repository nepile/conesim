#include <gtest/gtest.h>
#include "core/Configuration.hpp"
#include <fstream>
#include <filesystem>

using namespace conesim;

class ConfigurationTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Buat file config dummy sementara
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

// 1. Test basic getter & unit multiplier (M, k)
TEST_F(ConfigurationTest, ParsesNumericMultipliers) {
    Configuration conf;
    EXPECT_DOUBLE_EQ(conf.getDouble("Group.bufferSize"), 10000000.0);
    EXPECT_EQ(conf.getInt("Group.bufferSize"), 10000000);
}

// 2. Test boolean parser
TEST_F(ConfigurationTest, ParsesBoolean) {
    Configuration conf;
    EXPECT_TRUE(conf.getBoolean("Report.enabled"));
    EXPECT_FALSE(conf.getBoolean("NonExistent.key", false));
}

// 3. Test primary & secondary namespace resolution
TEST_F(ConfigurationTest, ResolvesNamespaces) {
    Configuration conf("Group");
    conf.setSecondaryNamespace("MovementModel");

    // Ditemukan di primary ("Group.router")
    EXPECT_EQ(conf.getConfiguration("router"), "EpidemicRouter");

    // Fallback ke secondary ("MovementModel.rngSeed")
    EXPECT_EQ(conf.getInt("rngSeed"), 100);
}

// 4. Test run index array [10; 20; 30] dengan modulo
TEST_F(ConfigurationTest, HandlesRunIndex) {
    Configuration conf;
    
    Configuration::setRunIndex(0);
    EXPECT_EQ(conf.getInt("Group.nrofHosts"), 10);

    Configuration::setRunIndex(1);
    EXPECT_EQ(conf.getInt("Group.nrofHosts"), 20);

    // Modulo wrap-around: index 3 -> 3 % 3 = index 0 -> 10
    Configuration::setRunIndex(3);
    EXPECT_EQ(conf.getInt("Group.nrofHosts"), 10);
}

// 5. Test CSV parser
TEST_F(ConfigurationTest, ParsesCsvSettings) {
    Configuration conf;
    std::vector<double> speed = conf.getCsvDoubles("Group.speed", 2);
    ASSERT_EQ(speed.size(), 2);
    EXPECT_DOUBLE_EQ(speed[0], 1.5);
    EXPECT_DOUBLE_EQ(speed[1], 4.5);
}

// 6. Test %% replacement
TEST_F(ConfigurationTest, ReplacesDelimiters) {
    Configuration conf;
    std::string filled = conf.valueFillString("run_%%Group.router%%_output");
    EXPECT_EQ(filled, "run_EpidemicRouter_output");
}