/**
 * @file test_NetworkInterface.cpp
 * @brief Unit tests for NetworkInterface, ModuleCommunicationBus, and ConnectivityGrid.
 * @author Frathol
 * @date September, 2026
 */

#include <gtest/gtest.h>
#include "core/NetworkInterface.hpp"
#include "core/ModuleCommunicationBus.hpp"
#include "core/Configuration.hpp"
#include "interfaces/ConnectivityGrid.hpp"
#include "core/Coord.hpp"

// Mock implementation of NetworkInterface for testing purposes
class DummyNetworkInterface : public core::NetworkInterface
{
public:
  explicit DummyNetworkInterface(const core::Configuration &config)
      : core::NetworkInterface(config) {}

  NetworkInterface *replicate() override
  {
    return new DummyNetworkInterface(*this);
  }

  void update() override
  {
    // Mock update
  }

  void connect(NetworkInterface *anotherInterface) override
  {
    // Mock connect
  }

  void createConnection(NetworkInterface *anotherInterface) override
  {
    // Mock create connection
  }
};

// Test fixture for Network Interface tests
class NetworkInterfaceTest : public ::testing::Test
{
protected:
  void SetUp() override
  {
    core::NetworkInterface::reset();
    interfaces::ConnectivityGrid::reset();
  }
};

// Test Unique Address Generation & Configuration Parsing
TEST_F(NetworkInterfaceTest, InitializationAndAddressGeneration)
{
  core::Configuration config;
  core::Configuration::addSetting("transmitRange", "100.0");
  core::Configuration::addSetting("transmitSpeed", "1024");

  DummyNetworkInterface if1(config);
  DummyNetworkInterface if2(config);

  EXPECT_EQ(if1.getTransmitRange(), 100.0);
  EXPECT_EQ(if1.getTransmitSpeed(), 1024);

  // Addresses should auto-increment
  EXPECT_EQ(if1.getAddress() + 1, if2.getAddress());
}

// Test Module Communication Bus property sharing and Pub/Sub notifications
class MockListener : public core::ModuleCommunicationListener
{
public:
  std::string lastKey = "";
  std::any lastValue;
  int callbackCount = 0;

  void moduleValueChanged(const std::string &key, const std::any &newValue) override
  {
    lastKey = key;
    lastValue = newValue;
    callbackCount++;
  }
};

TEST_F(NetworkInterfaceTest, ModuleCommunicationBusPubSub)
{
  core::ModuleCommunicationBus bus;
  MockListener listener;

  bus.addProperty("Network.radioRange", 250.0);
  bus.subscribe("Network.radioRange", &listener);

  // Update property should trigger listener callback
  bus.updateProperty("Network.radioRange", 300.0);

  EXPECT_EQ(listener.callbackCount, 1);
  EXPECT_EQ(listener.lastKey, "Network.radioRange");
  EXPECT_DOUBLE_EQ(std::any_cast<double>(listener.lastValue), 300.0);
}

// Test ConnectivityGrid Factory and Cell Localization
TEST_F(NetworkInterfaceTest, ConnectivityGridSpatialLookup)
{
  core::Configuration::addSetting("MovementModel.worldSize", "1000,1000");
  interfaces::ConnectivityGrid::reset();

  // Create a grid with cell size 100
  interfaces::ConnectivityGrid *grid = interfaces::ConnectivityGrid::ConnectivityGridFactory(1, 100.0);

  ASSERT_NE(grid, nullptr);

  // Check grid representation string
  std::string rep = grid->toString();
  EXPECT_FALSE(rep.empty());
}