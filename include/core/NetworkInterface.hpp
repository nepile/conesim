/**
 * @file NetworkInterface.hpp
 * @brief Header definition of the NetworkInterface abstract class.
 * @details Represents the network interface (e.g., Wi-Fi, Bluetooth radio) of a DTNHost.
 *          It manages the physical connectivity among hosts, checking transmission range,
 *          speed, and managing scanning intervals. It integrates with spatial optimizers
 *          (like ConnectivityGrid) to ensure O(1) or O(N) neighbor lookups instead of O(N^2).
 * @author Frathol
 * @date September, 2026
 */

#pragma once

#include <string>
#include <vector>
#include <random>
#include <atomic>
#include <any>

#include "core/ModuleCommunicationListener.hpp"
#include "core/Coord.hpp"

namespace interfaces
{
  class ConnectivityOptimizer;
}

namespace core
{

  class DTNHost;
  class Connection;
  class ConnectionListener;
  class Configuration;

  /**
   * @class NetworkInterface
   * @brief Abstract base class for managing host network connectivity.
   * @details Takes care of finding peers within range, establishing connections,
   *          and notifying listeners when connections go up or down.
   */
  class NetworkInterface : public ModuleCommunicationListener
  {
  public:
    /** @name Configuration Constants
     * String keys used for reading variables from the Configuration object.
     */
    ///@{
    inline static const std::string TRANSMIT_RANGE_S = "transmitRange";
    inline static const std::string TRANSMIT_SPEED_S = "transmitSpeed";
    inline static const std::string SCAN_INTERVAL_S = "scanInterval";
    ///@}

    /** @name ModuleCommunicationBus Constants
     * Identifiers used to listen to real-time variable changes over the bus.
     */
    ///@{
    inline static const std::string SCAN_INTERVAL_ID = "Network.scanInterval";
    inline static const std::string RANGE_ID = "Network.radioRange";
    inline static const std::string SPEED_ID = "Network.speed";
    ///@}

    /**
     * @brief Resets the static fields of the class (e.g., nextAddress and random generator).
     */
    static void reset();

    /**
     * @brief Constructor for creating an interface based on Configuration.
     * @param config The Configuration object containing interface parameters.
     */
    explicit NetworkInterface(const Configuration &config);

    /**
     * @brief Default constructor creating an empty generic interface.
     */
    NetworkInterface();

    /**
     * @brief Copy constructor.
     * @param ni The prototype interface to copy settings from.
     */
    NetworkInterface(const NetworkInterface &ni);

    virtual ~NetworkInterface() = default;

    /**
     * @brief Replication function. Creates a copy of the interface.
     * @return A pointer to the newly created NetworkInterface.
     */
    virtual NetworkInterface *replicate() = 0;

    /**
     * @brief Updates the state of current connections.
     * @details Tears down connections that are out of range, recalculates speeds, etc.
     */
    virtual void update() = 0;

    /**
     * @brief Connects the interface to another interface.
     * @details Overloaded in derived classes. Checks requirements before calling
     *          the internal connect(Connection*, NetworkInterface*) method.
     * @param anotherInterface The interface to connect to.
     */
    virtual void connect(NetworkInterface *anotherInterface) = 0;

    /**
     * @brief Creates a connection to another host without any prerequisite checks.
     * @param anotherInterface The interface to create the connection to.
     */
    virtual void createConnection(NetworkInterface *anotherInterface) = 0;

    /**
     * @brief Sets the host. Needed when a prototype is copied for several hosts.
     * @param host The host where the network interface resides.
     */
    virtual void setHost(DTNHost *host);

    /**
     * @brief Gets the type name of this interface.
     * @return The interface type as a string.
     */
    std::string getInterfaceType() const;

    /**
     * @brief Sets the connection listeners for this interface.
     * @param cListeners A vector of connection listeners.
     */
    void setClisteners(const std::vector<ConnectionListener *> &cListeners);

    /**
     * @brief Returns the unique network interface address.
     * @return The address (integer).
     */
    int getAddress() const;

    /**
     * @brief Returns the transmit range of this network layer.
     * @return The transmit range in meters.
     */
    double getTransmitRange() const;

    /**
     * @brief Returns the transmit speed of this network layer.
     * @return The transmit speed in bytes per second (Bps).
     */
    int getTransmitSpeed() const;

    /**
     * @brief Returns a list of currently active connections.
     * @return A vector of active Connection pointers.
     */
    std::vector<Connection *> getConnections() const;

    /**
     * @brief Checks if this interface is currently in scanning mode.
     * @note Modifies internal state (lastScanTime).
     * @return True if the interface is scanning; false if not.
     */
    bool isScanning();

    /**
     * @brief Callback triggered when a monitored module value changes.
     * @param key Identifier of the changed value.
     * @param newValue New value wrapped in a type-safe std::any container.
     */
    void moduleValueChanged(const std::string &key, const std::any &newValue) override;

    /**
     * @brief Disconnects a connection between this and another host.
     * @param anotherInterface The other host's network interface to disconnect.
     */
    virtual void destroyConnection(NetworkInterface *anotherInterface);

    /**
     * @brief Returns the DTNHost owning this interface.
     * @return Pointer to the DTNHost.
     */
    DTNHost *getHost() const;

    /**
     * @brief Returns the current location of the host.
     * @return The Coord object representing the location.
     */
    Coord getLocation() const;

    /**
     * @brief Returns a string representation of the network interface.
     * @return Formatted string containing address, host, and connection info.
     */
    virtual std::string toString() const;

  protected:
    DTNHost *host;                         ///< The host where this interface belongs.
    std::string interfaceType;             ///< Type of the interface (e.g., "Bluetooth").
    std::vector<Connection *> connections; ///< List of currently connected hosts.
    double transmitRange;                  ///< Radio range of the interface.
    int transmitSpeed;                     ///< Transmission speed.

    interfaces::ConnectivityOptimizer *optimizer; ///< Optimizer structure (e.g., Grid) for fast neighbor lookups.

    /**
     * @brief Internal method to establish a connection.
     * @param con The connection object.
     * @param anotherInterface The interface being connected to.
     */
    virtual void connect(Connection *con, NetworkInterface *anotherInterface);

    /**
     * @brief Internal method to tear down a connection.
     * @param con The connection object to remove.
     * @param anotherInterface The interface being disconnected from.
     */
    virtual void disconnect(Connection *con, NetworkInterface *anotherInterface);

    /**
     * @brief Checks if another interface is within the radio range of this interface.
     * @param anotherInterface The interface to check against.
     * @return True if within range, false otherwise.
     */
    bool isWithinRange(NetworkInterface *anotherInterface) const;

    /**
     * @brief Checks if the given interface is already connected to this host.
     * @param netinterface The interface to check.
     * @return True if connected, false otherwise.
     */
    bool isConnected(NetworkInterface *netinterface) const;

    /**
     * @brief Validates that a configuration value is non-negative.
     * @param value The value to check.
     * @param settingName The name of the setting (for error logging).
     * @throws std::runtime_error (or ConfigurationError) if the value is negative.
     */
    void ensurePositiveValue(double value, const std::string &settingName) const;

  private:
    /** @name Connection States */
    ///@{
    static constexpr int CON_UP = 1;
    static constexpr int CON_DOWN = 2;
    ///@}

    /** Thread-safe counter for generating unique network addresses. */
    inline static std::atomic<int> nextAddress{0};

    /** Random number generator for initial scan time scattering. */
    inline static std::mt19937 rng{12345};

    std::vector<ConnectionListener *> cListeners; ///< Listeners subscribed to connection events.
    int address;                                  ///< Unique address of this network interface.
    double scanInterval;                          ///< Scanning interval in simulated seconds.
    double lastScanTime;                          ///< Timestamp of the last scan.

    /**
     * @brief Generates a new unique network interface address.
     * @return The incremented address.
     */
    static int getNextNetAddress();

    /**
     * @brief Notifies all listeners about a connection state change.
     * @param type Type of change (CON_UP or CON_DOWN).
     * @param otherHost The host on the other end of the connection.
     */
    void notifyConnectionListeners(int type, DTNHost *otherHost);

    /**
     * @brief Removes a connection by its index in the internal array.
     * @param index The array index.
     * @param anotherInterface The interface of the other host.
     */
    void removeConnectionByIndex(size_t index, NetworkInterface *anotherInterface);
  };

} // namespace core