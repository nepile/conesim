/**
 * @file test_NetworkInterface.cpp
 * @brief Unit tests for NetworkInterface, ModuleCommunicationBus, and ConnectivityGrid.
 * @author Frathol
 * @date September 2026
 */

#include <gtest/gtest.h>
#include <string>
#include <any>
#include <fstream>
#include <cstdio> // Untuk std::remove

#include "core/NetworkInterface.hpp"
#include "core/ModuleCommunicationBus.hpp"
#include "core/Configuration.hpp"
#include "interfaces/ConnectivityGrid.hpp"
#include "core/Coord.hpp"

// ============================================================================
// Mocks & Helpers
// ============================================================================

class DummyNetworkInterface : public core::NetworkInterface {
public:
    explicit DummyNetworkInterface(const core::Configuration &config)
        : core::NetworkInterface(config) {}

    core::NetworkInterface* replicate() override {
        return new DummyNetworkInterface(*this);
    }

    void update() override {}
    void connect(core::NetworkInterface* anotherInterface) override {}
    void createConnection(core::NetworkInterface* anotherInterface) override {}
};

class MockListener : public core::ModuleCommunicationListener {
public:
    std::string lastKey = "";
    std::any lastValue;
    int callbackCount = 0;

    void moduleValueChanged(const std::string &key, const std::any &newValue) override {
        this->lastKey = key;
        this->lastValue = newValue;
        this->callbackCount++;
    }
};

// ============================================================================
// Test Fixtures
// ============================================================================

class NetworkInterfaceTest : public ::testing::Test {
protected:
    std::string testConfigFile = "test_network_interface.cfg";

    void SetUp() override {
        // 1. Buat file konfigurasi Dummy
        std::ofstream out(testConfigFile);
        out << "transmitRange = 100.0\n"
            << "transmitSpeed = 1024\n"
            << "NetworkInterface.transmitRange = 100.0\n"
            << "NetworkInterface.transmitSpeed = 1024\n"
            << "DummyNetworkInterface.transmitRange = 100.0\n"
            << "DummyNetworkInterface.transmitSpeed = 1024\n"
            << "MovementModel.worldSize = 1000, 1000\n";
        out.close();

        // 2. KUNCI UTAMA: Wajib menggunakan init() agar isInitialized = true!
        core::Configuration::init(testConfigFile);

        // 3. Reset internal static states
        core::NetworkInterface::reset();
        interfaces::ConnectivityGrid::reset();
    }

    void TearDown() override {
        // Hapus file konfigurasi setelah tes selesai
        std::remove(testConfigFile.c_str());
    }
};

// ============================================================================
// Test Cases
// ============================================================================

TEST_F(NetworkInterfaceTest, InitializationAndAddressGeneration) {
    // Karena sudah di-init(), Configuration sekarang akan mengizinkan pembacaan nilai
    core::Configuration config;

    DummyNetworkInterface if1(config);
    DummyNetworkInterface if2(config);

    EXPECT_DOUBLE_EQ(if1.getTransmitRange(), 100.0);
    EXPECT_EQ(if1.getTransmitSpeed(), 1024);

    // Addresses should auto-increment
    EXPECT_EQ(if1.getAddress() + 1, if2.getAddress());
}

TEST_F(NetworkInterfaceTest, ModuleCommunicationBusPubSub) {
    core::ModuleCommunicationBus bus;
    MockListener listener;

    bus.addProperty("Network.radioRange", 250.0);
    bus.subscribe("Network.radioRange", &listener);

    bus.updateProperty("Network.radioRange", 300.0);

    EXPECT_EQ(listener.callbackCount, 1);
    EXPECT_EQ(listener.lastKey, "Network.radioRange");
    
    ASSERT_TRUE(listener.lastValue.has_value());
    EXPECT_DOUBLE_EQ(std::any_cast<double>(listener.lastValue), 300.0);
}

TEST_F(NetworkInterfaceTest, ConnectivityGridSpatialLookup) {
    interfaces::ConnectivityGrid *grid = interfaces::ConnectivityGrid::ConnectivityGridFactory(1, 100.0);

    ASSERT_NE(grid, nullptr);

    std::string rep = grid->toString();
    EXPECT_FALSE(rep.empty());
}