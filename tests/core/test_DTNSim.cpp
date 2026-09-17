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
    }

    void TearDown() override {
        std::remove(testConfigFile.c_str());
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
