#include <gtest/gtest.h>
#include "core/DTN2Manager.hpp"
#include "core/Configuration.hpp"
#include "core/Coord.hpp"
#include "core/Connection.hpp"

// Define dummy classes to satisfy forward declarations in DTN2Manager
namespace ecla {
    class Bundle {
    public:
        int data;
        Bundle(int data) : data(data) {}
    };
    class CLAParser {};
}

namespace report {
    class DTN2Reporter {
    public:
        int id;
        DTN2Reporter(int id) : id(id) {}
    };
}

namespace input {
    class DTN2Events {
    public:
        int id;
        DTN2Events(int id) : id(id) {}
    };
}

using namespace core;

class DTN2ManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Clear global state before each test if necessary
        // Since setup() resets the maps, we can call setup with a null world to reset them.
        DTN2Manager::setup(nullptr);
        DTN2Manager::setReporter(nullptr);
        DTN2Manager::setEvents(nullptr);
    }

    void TearDown() override {
        DTN2Manager::setReporter(nullptr);
        DTN2Manager::setEvents(nullptr);
    }
};

TEST_F(DTN2ManagerTest, BundleManagementTest) {
    auto bundle1 = std::make_shared<ecla::Bundle>(42);
    auto bundle2 = std::make_shared<ecla::Bundle>(99);

    DTN2Manager::addBundle("bundle_1", bundle1);
    DTN2Manager::addBundle("bundle_2", bundle2);

    // Retrieve bundle1
    auto retrieved1 = DTN2Manager::getBundle("bundle_1");
    EXPECT_NE(retrieved1, nullptr);
    EXPECT_EQ(retrieved1->data, 42);

    // getBundle removes the bundle from the map in Java, let's check C++
    auto retrieved1_again = DTN2Manager::getBundle("bundle_1");
    EXPECT_EQ(retrieved1_again, nullptr); // Should be removed

    // Retrieve bundle2
    auto retrieved2 = DTN2Manager::getBundle("bundle_2");
    EXPECT_NE(retrieved2, nullptr);
    EXPECT_EQ(retrieved2->data, 99);
}

TEST_F(DTN2ManagerTest, SetGetReporterTest) {
    auto rep = std::make_shared<report::DTN2Reporter>(101);
    
    EXPECT_EQ(DTN2Manager::getReporter(), nullptr);
    
    DTN2Manager::setReporter(rep);
    auto retrieved = DTN2Manager::getReporter();
    
    EXPECT_NE(retrieved, nullptr);
    EXPECT_EQ(retrieved->id, 101);
}

TEST_F(DTN2ManagerTest, SetGetEventsTest) {
    auto evts = std::make_shared<input::DTN2Events>(202);
    
    EXPECT_EQ(DTN2Manager::getEvents(), nullptr);
    
    DTN2Manager::setEvents(evts);
    auto retrieved = DTN2Manager::getEvents();
    
    EXPECT_NE(retrieved, nullptr);
    EXPECT_EQ(retrieved->id, 202);
}

TEST_F(DTN2ManagerTest, SetupEarlyReturnOnMissingReporterOrEventsTest) {
    // If reporter or events are null, setup should early return without crashing
    DTN2Manager::setReporter(nullptr);
    DTN2Manager::setEvents(nullptr);
    
    // Pass a dummy world (nullptr in this case since we just want it to early return)
    // It should not dereference world if reporter/events are null
    EXPECT_NO_THROW(DTN2Manager::setup(nullptr));
}

// ============================================================================
// STUBS FOR LINKING
// ============================================================================
// Because DTN2Manager includes World, and World calls DTNHost methods,
// the linker requires these symbols to exist. Since the actual DTNHost
// is not fully implemented in this branch, we provide stubs for testing.
namespace core {
    // int DTNHost::getAddress() const { return 0; }
    // void DTNHost::update(bool) {}
    // void DTNHost::move(double) {}
}
