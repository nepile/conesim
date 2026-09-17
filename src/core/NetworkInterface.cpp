/**
 * @file NetworkInterface.cpp
 * @brief Implementation of the NetworkInterface abstract class.
 * @details Implements the logic for physical connectivity, range checking,
 *          scanning intervals, and managing active connections. Integrates
 *          with ConnectivityGrid to optimize spatial neighbor lookups.
 * @author Frathol
 * @date September, 2026
 */

#include "core/NetworkInterface.hpp"
#include "core/DTNHost.hpp"
#include "core/Connection.hpp"
#include "core/ConnectionListener.hpp"
#include "core/Configuration.hpp"
#include "core/ModuleCommunicationBus.hpp"
#include "interfaces/ConnectivityGrid.hpp"
#include "core/SimulationClock.hpp"

#include <stdexcept>
#include <sstream>
#include <algorithm>
#include <functional>

namespace core
{

  /**
   * @brief Resets the static state of all network interfaces.
   * @details Resets the atomic address counter to zero and re-seeds the random
   *          number generator for deterministic simulation runs.
   */
  void NetworkInterface::reset()
  {
    nextAddress.store(0, std::memory_order_relaxed);
    rng.seed(12345); // Reset seed for reproducible scenarios
  }

  /**
   * @brief Thread-safe generator for unique network addresses.
   * @return A unique integer address.
   */
  int NetworkInterface::getNextNetAddress()
  {
    return nextAddress.fetch_add(1, std::memory_order_relaxed);
  }

  /**
   * @brief Constructs an interface configured via the simulation's Configuration object.
   * @param config The Configuration dictionary containing range, speed, and intervals.
   */
  NetworkInterface::NetworkInterface(const Configuration &config)
      : host(nullptr),
        interfaceType("Default"), // Can be adapted if Configuration supports namespaces
        optimizer(nullptr),
        address(getNextNetAddress()),
        lastScanTime(0.0)
  {
    this->transmitRange = config.getDouble(TRANSMIT_RANGE_S);
    this->transmitSpeed = config.getInt(TRANSMIT_SPEED_S);

    ensurePositiveValue(this->transmitRange, TRANSMIT_RANGE_S);
    ensurePositiveValue(this->transmitSpeed, TRANSMIT_SPEED_S);

    if (config.contains(SCAN_INTERVAL_S))
    {
      this->scanInterval = config.getDouble(SCAN_INTERVAL_S);
    }
    else
    {
      this->scanInterval = 0.0;
    }
  }

  /**
   * @brief Constructs a raw, empty network interface with default values.
   */
  NetworkInterface::NetworkInterface()
      : host(nullptr),
        interfaceType("Default"),
        transmitRange(0.0),
        transmitSpeed(0),
        optimizer(nullptr),
        address(getNextNetAddress()),
        scanInterval(0.0),
        lastScanTime(0.0)
  {
  }

  /**
   * @brief Safely clones an existing network interface prototype.
   * @details Retains transmission configurations but initializes a new address
   *          and scrambles the initial scan time to avoid synchronized scanning spikes.
   * @param ni The interface prototype to clone.
   */
  NetworkInterface::NetworkInterface(const NetworkInterface &ni)
      : host(ni.host),
        interfaceType(ni.interfaceType),
        transmitRange(ni.transmitRange),
        transmitSpeed(ni.transmitSpeed),
        optimizer(nullptr), // Optimizer is bound later in setHost()
        cListeners(ni.cListeners),
        address(getNextNetAddress()),
        scanInterval(ni.scanInterval)
  {
    // Draw lastScanTime randomly between [0 -- scanInterval]
    if (this->scanInterval > 0.0)
    {
      std::uniform_real_distribution<double> dist(0.0, this->scanInterval);
      this->lastScanTime = dist(rng);
    }
    else
    {
      this->lastScanTime = 0.0;
    }
  }

  /**
   * @brief Binds this interface to a specific DTNHost and hooks into communication buses.
   * @param host Pointer to the parent DTNHost.
   */
  void NetworkInterface::setHost(DTNHost *host)
  {
    this->host = host;

    // Subscribe to combus to listen for dynamic setting changes
    if (ModuleCommunicationBus *comBus = host->getComBus())
    {
      comBus->subscribe(SCAN_INTERVAL_ID, this);
      comBus->subscribe(RANGE_ID, this);
      comBus->subscribe(SPEED_ID, this);
    }

    // Initialize spatial optimizer (ConnectivityGrid)
    // We use a hash of the interface type to separate different technologies (e.g. WiFi vs Bluetooth)
    int typeHash = static_cast<int>(std::hash<std::string>{}(this->interfaceType));

    this->optimizer = interfaces::ConnectivityGrid::ConnectivityGridFactory(typeHash, this->transmitRange);
    this->optimizer->addInterface(this);
  }

  /**
   * @brief Determines whether the interface is actively scanning for peers based on simulation time.
   * @return True if the interface is in an active scanning phase, otherwise false.
   */
  bool NetworkInterface::isScanning()
  {
    double simTime = SimulationClock::getTime();

    if (this->scanInterval > 0.0)
    {
      if (simTime < this->lastScanTime)
      {
        return false; // Not time for the first scan yet
      }
      else if (simTime > this->lastScanTime + this->scanInterval)
      {
        this->lastScanTime = simTime; // Time to start the next scan round
        return true;
      }
      else if (simTime != this->lastScanTime)
      {
        return false; // Between scan intervals
      }
    }

    // Interval == 0 or still in the exact moment of the last scan round
    return true;
  }

  /**
   * @brief Internally links two network interfaces via a connection object.
   * @details Registers bidirectional links and broadcasts CON_UP events to listeners.
   * @param con Pointer to the connection object.
   * @param anotherInterface Pointer to the peer's network interface.
   */
  void NetworkInterface::connect(Connection *con, NetworkInterface *anotherInterface)
  {
    this->connections.push_back(con);
    notifyConnectionListeners(CON_UP, anotherInterface->getHost());

    // Set up bidirectional connection on the peer's side
    anotherInterface->connections.push_back(con);

    // Inform the host routers about the connection
    this->host->connectionUp(con);
    anotherInterface->getHost()->connectionUp(con);
  }

  /**
   * @brief Internally tears down an active connection.
   * @details Removes bidirectional links and broadcasts CON_DOWN events.
   * @param con Pointer to the connection object to destroy.
   * @param anotherInterface Pointer to the peer's network interface.
   * @throws std::runtime_error If the connection is not found on the peer's side.
   */
  void NetworkInterface::disconnect(Connection *con, NetworkInterface *anotherInterface)
  {
    con->setUpState(false);
    notifyConnectionListeners(CON_DOWN, anotherInterface->getHost());

    // Tear down bidirectional connection
    auto &otherConns = anotherInterface->connections;
    auto it = std::find(otherConns.begin(), otherConns.end(), con);

    if (it != otherConns.end())
    {
      otherConns.erase(it);
    }
    else
    {
      throw std::runtime_error("No connection found in anotherInterface during disconnect.");
    }

    this->host->connectionDown(con);
    anotherInterface->getHost()->connectionDown(con);
  }

  /**
   * @brief Disconnects any active connection tied to a specific peer interface.
   * @param anotherInterface Pointer to the peer's network interface.
   */
  void NetworkInterface::destroyConnection(NetworkInterface *anotherInterface)
  {
    DTNHost *anotherHost = anotherInterface->getHost();

    // Iterate backwards safely because we are erasing elements from the vector during the loop
    for (int i = static_cast<int>(this->connections.size()) - 1; i >= 0; --i)
    {
      if (this->connections[i]->getOtherNode(this->host) == anotherHost)
      {
        removeConnectionByIndex(static_cast<size_t>(i), anotherInterface);
      }
    }
  }

  /**
   * @brief Helper method to remove a connection by its array index.
   * @param index The position of the connection in the internal connections list.
   * @param anotherInterface Pointer to the peer's network interface.
   * @throws std::runtime_error If the connection index is invalid or peer data is corrupt.
   */
  void NetworkInterface::removeConnectionByIndex(size_t index, NetworkInterface *anotherInterface)
  {
    if (index >= this->connections.size())
      return;

    Connection *con = this->connections[index];
    DTNHost *anotherNode = anotherInterface->getHost();

    con->setUpState(false);
    notifyConnectionListeners(CON_DOWN, anotherNode);

    // Tear down bidirectional connection
    auto &otherConns = anotherInterface->connections;
    auto it = std::find(otherConns.begin(), otherConns.end(), con);

    if (it != otherConns.end())
    {
      otherConns.erase(it);
    }
    else
    {
      throw std::runtime_error("No connection found in anotherInterface during removeConnectionByIndex.");
    }

    this->host->connectionDown(con);
    anotherNode->connectionDown(con);

    // Remove from our own list
    this->connections.erase(this->connections.begin() + index);
  }

  /**
   * @brief Checks if a target interface is physically within this interface's radio range.
   * @param anotherInterface Pointer to the peer interface.
   * @return True if distance <= minimum shared range, otherwise false.
   */
  bool NetworkInterface::isWithinRange(NetworkInterface *anotherInterface) const
  {
    double smallerRange = anotherInterface->getTransmitRange();
    double myRange = getTransmitRange();

    if (myRange < smallerRange)
    {
      smallerRange = myRange;
    }

    return this->host->getLocation().distance(anotherInterface->getHost()->getLocation()) <= smallerRange;
  }

  /**
   * @brief Verifies if this interface is currently connected to a given target interface.
   * @param netinterface Pointer to the target interface.
   * @return True if a matching Connection object is found, otherwise false.
   */
  bool NetworkInterface::isConnected(NetworkInterface *netinterface) const
  {
    for (Connection *con : this->connections)
    {
      if (con->getOtherInterface(const_cast<NetworkInterface *>(this)) == netinterface)
      {
        return true;
      }
    }
    return false;
  }

  /**
   * @brief Utility to guard against negative setting values.
   * @param value The configuration value to check.
   * @param settingName The string name of the setting.
   * @throws std::runtime_error If the value is negative.
   */
  void NetworkInterface::ensurePositiveValue(double value, const std::string &settingName) const
  {
    if (value < 0)
    {
      throw std::runtime_error("Negative value (" + std::to_string(value) +
                               ") not accepted for setting " + settingName);
    }
  }

  /**
   * @brief Broadcasts topology changes to registered ConnectionListener modules.
   * @param type CON_UP (1) or CON_DOWN (2).
   * @param otherHost The remote host involved in the connection change.
   */
  void NetworkInterface::notifyConnectionListeners(int type, DTNHost *otherHost)
  {
    if (this->cListeners.empty())
      return;

    for (ConnectionListener *cl : this->cListeners)
    {
      switch (type)
      {
      case CON_UP:
        cl->hostsConnected(*(this->host), *otherHost);
        break;
      case CON_DOWN:
        cl->hostDisconnected(*(this->host), *otherHost);
        break;
      default:
        throw std::invalid_argument("Invalid connection state type.");
      }
    }
  }

  /**
   * @brief Intercepts dynamic value changes broadcasted via ModuleCommunicationBus.
   * @param key The string identifier of the changing variable.
   * @param newValue The new value safely wrapped in std::any.
   * @throws std::runtime_error If a type mismatch occurs during std::any_cast.
   */
  void NetworkInterface::moduleValueChanged(const std::string &key, const std::any &newValue)
  {
    try
    {
      if (key == SCAN_INTERVAL_ID)
      {
        this->scanInterval = std::any_cast<double>(newValue);
      }
      else if (key == SPEED_ID)
      {
        this->transmitSpeed = std::any_cast<int>(newValue);
      }
      else if (key == RANGE_ID)
      {
        this->transmitRange = std::any_cast<double>(newValue);
      }
      else
      {
        throw std::runtime_error("Unexpected combus ID " + key);
      }
    }
    catch (const std::bad_any_cast &)
    {
      throw std::runtime_error("Type mismatch in moduleValueChanged for key: " + key);
    }
  }

  std::string NetworkInterface::getInterfaceType() const { return this->interfaceType; }

  void NetworkInterface::setClisteners(const std::vector<ConnectionListener *> &cListeners)
  {
    this->cListeners = cListeners;
  }

  int NetworkInterface::getAddress() const { return this->address; }
  double NetworkInterface::getTransmitRange() const { return this->transmitRange; }
  int NetworkInterface::getTransmitSpeed() const { return this->transmitSpeed; }
  std::vector<Connection *> NetworkInterface::getConnections() const { return this->connections; }
  DTNHost *NetworkInterface::getHost() const { return this->host; }
  Coord NetworkInterface::getLocation() const { return this->host->getLocation(); }

  /**
   * @brief Serializes the interface metadata to a string format.
   * @return A descriptive string detailing address, host, and active connections.
   */
  std::string NetworkInterface::toString() const
  {
    std::ostringstream oss;
    oss << "net interface " << this->address << " of "
        << (this->host ? this->host->toString() : "UnknownHost")
        << ". Connections: " << this->connections.size();
    return oss.str();
  }

} // namespace core