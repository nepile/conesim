#include <gtest/gtest.h>
#include "core/DTNSim.hpp"
#include "core/Configuration.hpp"

#include <fstream>
#include <filesystem>

using namespace core;

// A simple global flag to check if reset was called
static int resetCallCount = 0;

void dummyResetFunction() {
    resetCallCount++;
}

class DTNSimTest : public ::testing::Test {
protected:
    std::string testConfigFile = "test_dtnsim.cfg";

    void SetUp() override {
        // Create a dummy config file
        std::ofstream out(testConfigFile);
        out << "Scenario.name = TestDTNSim\n"
            << "Optimization.cellSizeMult = 5\n";
        out.close();

        resetCallCount = 0;
        DTNSim::reset();
    }

    void TearDown() override {
        std::remove(testConfigFile.c_str());
        DTNSim::reset();
    }
};

TEST_F(DTNSimTest, RegisterForResetTest) {
    // Register the dummy reset function
    DTNSim::registerForReset(&dummyResetFunction);

    // Call DTNSim::main with batch mode flag to trigger reset
    const char* argv[] = {
        "conesim",
        "-b",
        "2", // run 2 times (index 0 and 1)
        testConfigFile.c_str()
    };
    int argc = 4;

    // We can't easily capture the output without redirecting std::cout,
    // but we can at least ensure it executes without crashing and calls reset.
    DTNSim::main(argc, const_cast<char**>(argv));

    // Because it runs twice (0 and 1), the reset function should be called 2 times
    EXPECT_EQ(resetCallCount, 2);
}

TEST_F(DTNSimTest, GuiModeTest) {
    // Call DTNSim::main in GUI mode (no -b flag)
    const char* argv[] = {
        "conesim",
        "5", // run index 5
        testConfigFile.c_str()
    };
    int argc = 3;

    DTNSim::main(argc, const_cast<char**>(argv));
    
    // In GUI mode, reset is not called automatically by main before the run
    EXPECT_EQ(resetCallCount, 0);
}

TEST_F(DTNSimTest, BatchModeLongFlagTest) {
    DTNSim::registerForReset(&dummyResetFunction);

    const char* argv[] = {
        "conesim",
        "--batch",
        "2",
        testConfigFile.c_str()
    };
    int argc = 4;

    DTNSim::main(argc, const_cast<char**>(argv));

    EXPECT_EQ(resetCallCount, 2);
}

TEST_F(DTNSimTest, BatchModeDirectConfigTest) {
    DTNSim::registerForReset(&dummyResetFunction);

    // -b followed immediately by config file (defaults to 1 run)
    const char* argv[] = {
        "conesim",
        "-b",
        testConfigFile.c_str()
    };
    int argc = 3;

    DTNSim::main(argc, const_cast<char**>(argv));

    EXPECT_EQ(resetCallCount, 1);
}

TEST_F(DTNSimTest, BatchModeLongFlagDirectConfigTest) {
    DTNSim::registerForReset(&dummyResetFunction);

    // --batch followed immediately by config file (defaults to 1 run)
    const char* argv[] = {
        "conesim",
        "--batch",
        testConfigFile.c_str()
    };
    int argc = 3;

    DTNSim::main(argc, const_cast<char**>(argv));

    EXPECT_EQ(resetCallCount, 1);
}

TEST_F(DTNSimTest, ConfigWithoutRunIndexTest) {
    // Non-batch mode with config file directly (no numeric run index)
    const char* argv[] = {
        "conesim",
        testConfigFile.c_str()
    };
    int argc = 2;

    // Should not crash trying to parse config filename as an integer
    DTNSim::main(argc, const_cast<char**>(argv));

    Configuration conf;
    EXPECT_EQ(conf.getConfiguration("Scenario.name"), "TestDTNSim");
}

TEST_F(DTNSimTest, BatchModeFallbackToDefaultSettingsTest) {
    DTNSim::registerForReset(&dummyResetFunction);

    // Only -b flag passed, should fallback to default_settings.cfg
    const char* argv[] = {
        "conesim",
        "-b"
    };
    int argc = 2;

    DTNSim::main(argc, const_cast<char**>(argv));

    EXPECT_EQ(resetCallCount, 1);
    Configuration conf;
    EXPECT_TRUE(conf.contains("Scenario.name"));
}
