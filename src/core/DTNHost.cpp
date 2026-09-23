/**
 * @file DTNHost.cpp
 * @brief Implementation of the DTNHost class.
 * @details Represents nodes like agents, cars, or humans that can move and carry
 *          messages in the simulation network. Acts as a central hub for routing,
 *          movement, and network interfaces.
 * @author Frathol
 * @date September, 2026
 */

#include "core/DTNHost.hpp"
#include "routing/MessageRouter.hpp"
#include "movement/MovementModel.hpp"
#include "movement/Path.hpp"
#include "core/MessageListener.hpp"
#include "core/MovementListener.hpp"
#include "core/NetworkInterface.hpp"
#include "core/ModuleCommunicationBus.hpp"
#include "core/Connection.hpp"
#include "core/Message.hpp"
#include "routing/RoutingInfo.hpp"
#include "core/SimulationClock.hpp"

#include <stdexcept>
#include <iostream>
#include <sstream>

namespace core {

int DTNHost::nextAddress = 0;

/**
 * @brief Resets the static address counter for DTNHost.
 * @details Used primarily during simulation resets to ensure deterministic address generation.
 */
void DTNHost::reset() {
    nextAddress = 0;
}

/**
 * @brief Generates and returns a unique network-layer address for a new host.
 * @return The next available integer address.
 */
int DTNHost::getNextAddress() {
    return nextAddress++;
}

/**
 * @brief Constructs a new DTNHost by replicating provided prototypes.
 * @param msgLs List of message listeners subscribing to this host.
 * @param movLs List of movement listeners subscribing to this host.
 * @param groupId The group identifier string for this host.
 * @param interf List of network interface prototypes to attach.
 * @param comBus Pointer to the module communication bus.
 * @param mmProto Prototype of the movement model.
 * @param mRouterProto Prototype of the message router.
 */
DTNHost::DTNHost(const std::vector<MessageListener*>& msgLs,
                 const std::vector<MovementListener*>& movLs,
                 const std::string& groupId,
                 const std::vector<std::shared_ptr<NetworkInterface>>& interf,
                 ModuleCommunicationBus* comBus,
                 std::shared_ptr<movement::MovementModel> mmProto,
                 std::shared_ptr<routing::MessageRouter> mRouterProto)
    : address(getNextAddress()),
      location(0, 0),
      destination(0, 0),
      path(nullptr),
      speed(0.0),
      msgListeners(msgLs),
      movListeners(movLs),
      comBus(comBus),
      msgReceived(0),
      msgTransferred(0),
      totalContactTime(0.0)
{
    this->name = groupId + std::to_string(this->address);

    // Replicate network interfaces
    for (const auto& i : interf) {
        std::shared_ptr<NetworkInterface> ni(i->replicate());
        ni->setHost(this);
        this->net.push_back(ni);
    }

    // Create instances by replicating the prototypes
    this->movement = mmProto->replicate();

    // Replicate router and initialize
    std::shared_ptr<routing::MessageRouter> newRouter(mRouterProto->replicate());
    setRouter(newRouter);

    this->location = this->movement->getInitialLocation();
    this->nextTimeToMove = this->movement->nextPathAvailable();

    // Inform movement listeners about the initial location
    for (MovementListener* l : this->movListeners) {
        if (l) l->initialLocation(*this, this->location);
    }
}

// ============================================================================
// Core State & Setters
// ============================================================================

/**
 * @brief Checks if the host's movement model is currently active.
 * @return True if active, false otherwise.
 */
bool DTNHost::isActive() const {
    return this->movement->isActive();
}

/**
 * @brief Binds a message router to this host.
 * @param router Shared pointer to the new MessageRouter.
 */
void DTNHost::setRouter(std::shared_ptr<routing::MessageRouter> router) {
    this->router = router;
}

/**
 * @brief Retrieves the host's active message router.
 * @return Shared pointer to the MessageRouter.
 */
std::shared_ptr<routing::MessageRouter> DTNHost::getRouter() const { 
    return this->router; 
}

/**
 * @brief Gets the network-layer address of this host.
 * @return Integer address.
 */
int DTNHost::getAddress() const { 
    return this->address; 
}

/**
 * @brief Overrides the network-layer address of this host.
 * @param addr The new address.
 */
void DTNHost::setAddress(int addr) { 
    this->address = addr; 
}

/**
 * @brief Retrieves the communication bus attached to this host.
 * @return Pointer to the ModuleCommunicationBus.
 */
ModuleCommunicationBus* DTNHost::getComBus() const { 
    return this->comBus; 
}

// ============================================================================
// Connection Events
// ============================================================================

/**
 * @brief Notifies the host's router that a connection has been established.
 * @param con Pointer to the new connection.
 */
void DTNHost::connectionUp(Connection* con) {
    if (this->router) this->router->changedConnection(con);
}

/**
 * @brief Notifies the host's router that a connection has been severed.
 * @param con Pointer to the broken connection.
 */
void DTNHost::connectionDown(Connection* con) {
    if (this->router) this->router->changedConnection(con);
}

/**
 * @brief Aggregates and retrieves a list of all active connections across all interfaces.
 * @return Vector of Connection pointers.
 */
std::vector<Connection*> DTNHost::getConnections() const {
    std::vector<Connection*> lc;
    for (const auto& i : this->net) {
        std::vector<Connection*> iconns = i->getConnections();
        lc.insert(lc.end(), iconns.begin(), iconns.end());
    }
    return lc;
}

// ============================================================================
// Movement & Location
// ============================================================================

/**
 * @brief Gets the current physical location of the host.
 * @return A Coord object representing the location.
 */
Coord DTNHost::getLocation() const { 
    return this->location; 
}

/**
 * @brief Retrieves the current movement path of the host.
 * @return Shared pointer to the active Path.
 */
std::shared_ptr<movement::Path> DTNHost::getPath() const { 
    return this->path; 
}

/**
 * @brief Manually overrides the host's current location.
 * @param loc The new Coord location.
 */
void DTNHost::setLocation(const Coord& loc) { 
    this->location = loc; 
}

/**
 * @brief Sets a custom name for this host.
 * @param name The new name string.
 */
void DTNHost::setName(const std::string& name) { 
    this->name = name; 
}

/**
 * @brief Sets a custom UI rendering color for this host.
 * @param color Vector representing RGB color values.
 */
void DTNHost::setColor(const std::vector<int>& color) { 
    this->color = color; 
}

/**
 * @brief Retrieves the custom UI rendering color for this host.
 * @return Vector representing RGB color values.
 */
std::vector<int> DTNHost::getColor() const { 
    return this->color; 
}

/**
 * @brief Prompts the host to update its network interfaces and router states.
 * @param simulateConnections If true, also triggers updates on all network interfaces.
 */
void DTNHost::update(bool simulateConnections) {
    if (!isActive()) return;

    if (simulateConnections) {
        for (auto& i : this->net) {
            i->update();
        }
    }
    if (this->router) this->router->update();
}

/**
 * @brief Advances the host's location along its trajectory based on elapsed time.
 * @param timeIncrement Time elapsed since the last movement update.
 */
void DTNHost::move(double timeIncrement) {
    // Note: Assuming SimClock::getTime() is globally accessible, otherwise inject time.
    // if (!isActive() || SimClock::getTime() < this->nextTimeToMove) return;

    if (!this->path || !this->path->hasNext()) {
        if (!setNextWaypoint()) {
            return;
        }
    }

    double possibleMovement = timeIncrement * this->speed;
    double dist = this->location.distance(this->destination);

    while (possibleMovement >= dist) {
        this->location.setLocation(this->destination);
        possibleMovement -= dist;

        if (!setNextWaypoint()) {
            return; 
        }
        dist = this->location.distance(this->destination);
    }

    double dx = (possibleMovement / dist) * (this->destination.getX() - this->location.getX());
    double dy = (possibleMovement / dist) * (this->destination.getY() - this->location.getY());
    this->location.translate(dx, dy);
}

/**
 * @brief Retrieves and sets the next destination waypoint from the movement model.
 * @return True if a new waypoint was set successfully, false otherwise.
 */
bool DTNHost::setNextWaypoint() {
    if (!this->path) {
        // Assume movement->getPath() returns movement::Path object, wrap in shared_ptr
        this->path = std::make_shared<movement::Path>(this->movement->getPath());
    }

    if (!this->path || !this->path->hasNext()) {
        this->nextTimeToMove = this->movement->nextPathAvailable();
        this->path.reset();
        return false;
    }

    this->destination = this->path->getNextWaypoint();
    this->speed = this->path->getSpeed();

    for (MovementListener* l : this->movListeners) {
        if (l) l->newDestination(*this, this->destination, this->speed);
    }

    return true;
}

// ============================================================================
// Message Handling
// ============================================================================

/**
 * @brief Retrieves all messages currently stored in the host's buffer.
 * @return A vector of shared pointers to the messages.
 */
std::vector<std::shared_ptr<Message>> DTNHost::getMessageCollection() const {
    return this->router ? this->router->getMessageCollection() : std::vector<std::shared_ptr<Message>>();
}

/**
 * @brief Returns the total number of messages currently held by the host.
 * @return The integer count of messages.
 */
int DTNHost::getNrofMessages() const {
    return this->router ? this->router->getNrofMessages() : 0;
}

/**
 * @brief Calculates the buffer utilization percentage of the host's router.
 * @return A double between 0.0 and 100.0 representing occupancy.
 */
double DTNHost::getBufferOccupancy() const {
    if (!this->router) return 0.0;
    double bSize = this->router->getBufferSize();
    double freeBuffer = this->router->getFreeBufferSize();
    if (bSize == 0.0) return 0.0;
    return 100.0 * ((bSize - freeBuffer) / bSize);
}

/**
 * @brief Retrieves routing information and metrics from the underlying router.
 * @return Shared pointer to the RoutingInfo structure.
 */
std::shared_ptr<routing::RoutingInfo> DTNHost::getRoutingInfo() const {
    if (!this->router) return nullptr;
    // router->getRoutingInfo() returns by value, wrap it in shared_ptr to match header
    return std::make_shared<routing::RoutingInfo>(this->router->getRoutingInfo()); 
}

/**
 * @brief Orders the router to initiate sending a message to a target host.
 * @param id The unique string ID of the message.
 * @param to Pointer to the destination DTNHost.
 */
void DTNHost::sendMessage(const std::string& id, DTNHost* to) {
    if (this->router) this->router->sendMessage(id, to);
}

/**
 * @brief Invoked when this host begins receiving a message from a peer.
 * @param m Shared pointer to the incoming message.
 * @param from Pointer to the sending DTNHost.
 * @return A status code evaluating acceptance or rejection.
 */
int DTNHost::receiveMessage(std::shared_ptr<Message> m, DTNHost* from) {
    if (!this->router) return -1;

    int retVal = this->router->receiveMessage(m.get(), from);

    if (retVal == routing::MessageRouter::RCV_OK) { 
        m->addNodeOnPath(this);
    }
    return retVal;
}

/**
 * @brief Polls the router to see if it wishes to push deliverable messages over a connection.
 * @param con Pointer to the connection to evaluate.
 * @return True if the router initiated a transfer, false otherwise.
 */
bool DTNHost::requestDeliverableMessages(Connection* con) {
    return this->router ? this->router->requestDeliverableMessages(con) : false;
}

/**
 * @brief Informs the host that an outgoing message transfer has successfully completed.
 * @param id The unique string ID of the message.
 * @param from Pointer to the peer host involved in the transfer.
 */
void DTNHost::messageTransferred(const std::string& id, DTNHost* from) {
    if (this->router) this->router->messageTransferred(id, from);
}

/**
 * @brief Informs the host that an ongoing message transfer was interrupted.
 * @param id The unique string ID of the message.
 * @param from Pointer to the peer host involved in the transfer.
 * @param bytesRemaining The number of bytes left untransferred before abortion.
 */
void DTNHost::messageAborted(const std::string& id, DTNHost* from, int bytesRemaining) {
    if (this->router) this->router->messageAborted(id, from, bytesRemaining);
}

/**
 * @brief Forces a new message directly into the host's router.
 * @param m Shared pointer to the new message.
 */
void DTNHost::createNewMessage(std::shared_ptr<Message> m) {
    if (this->router) this->router->createNewMessage(m);
}

/**
 * @brief Instructs the router to discard a specific message from its buffer.
 * @param id The unique string ID of the message to delete.
 * @param drop True if dropped due to constraints (e.g. TTL, buffer full), false for delivery.
 */
void DTNHost::deleteMessage(const std::string& id, bool drop) {
    if (this->router) this->router->deleteMessage(id, drop);
}

// ============================================================================
// Interfaces & Connection Utilities
// ============================================================================

/**
 * @brief Retrieves all network interfaces bound to this host.
 * @return A vector of shared pointers to NetworkInterfaces.
 */
std::vector<std::shared_ptr<NetworkInterface>> DTNHost::getInterfaces() const {
    return this->net;
}

/**
 * @brief Retrieves a specific network interface by its 1-based index.
 * @param interfaceNo The 1-based index of the interface.
 * @return Shared pointer to the requested NetworkInterface.
 * @throws std::out_of_range If the index is invalid.
 */
std::shared_ptr<NetworkInterface> DTNHost::getInterface(int interfaceNo) const {
    if (interfaceNo < 1 || interfaceNo > static_cast<int>(this->net.size())) {
        throw std::out_of_range("No such interface index: " + std::to_string(interfaceNo));
    }
    return this->net[interfaceNo - 1];
}

/**
 * @brief Retrieves a specific network interface by its technology string.
 * @param interfacetype The type string (e.g. "WiFi").
 * @return Shared pointer to the requested NetworkInterface, or nullptr if not found.
 */
std::shared_ptr<NetworkInterface> DTNHost::getInterface(const std::string& interfacetype) const {
    for (const auto& ni : this->net) {
        if (ni->getInterfaceType() == interfacetype) {
            return ni;
        }
    }
    return nullptr;
}

/**
 * @brief Forcibly triggers a physical connection event between this host and another.
 * @param anotherHost Pointer to the target host.
 * @param interfaceId The string ID of the interface type to bind.
 * @param up True to initiate a connection, false to tear it down.
 * @throws std::runtime_error If the specified interfaces don't exist or mismatch.
 */
void DTNHost::forceConnection(DTNHost* anotherHost, const std::string& interfaceId, bool up) {
    std::shared_ptr<NetworkInterface> ni = nullptr;
    std::shared_ptr<NetworkInterface> no = nullptr;

    if (!interfaceId.empty()) {
        ni = getInterface(interfaceId);
        no = anotherHost->getInterface(interfaceId);

        if (!ni || !no) {
            throw std::runtime_error("Tried to use a nonexisting interfacetype: " + interfaceId);
        }
    } else {
        ni = getInterface(1);
        no = anotherHost->getInterface(1);

        if (ni->getInterfaceType() != no->getInterfaceType()) {
            throw std::runtime_error("Interface types do not match. Please specify interface type explicitly.");
        }
    }

    if (up) {
        ni->createConnection(no.get());
    } else {
        ni->destroyConnection(no.get());
    }
}

/**
 * @brief Deprecated alias for forcing a connection via the default interface.
 * @deprecated Use forceConnection() instead.
 * @param h Pointer to the target host.
 */
void DTNHost::connect(DTNHost* h) {
    std::cerr << "WARNING: using deprecated DTNHost.connect(DTNHost)\n"
              << "Use DTNHost.forceConnection(DTNHost, \"\", true) instead\n";
    forceConnection(h, "", true);
}

/**
 * @brief Serializes the host's identifier.
 * @return A string corresponding to the host's name.
 */
std::string DTNHost::toString() const {
    return this->name;
}

// ============================================================================
// Comparison & Equality
// ============================================================================

/**
 * @brief Checks if two DTNHost pointers point to the exact same instance in memory.
 * @param otherHost Pointer to the comparison host.
 * @return True if they point to the same memory block.
 */
bool DTNHost::equals(const DTNHost* otherHost) const {
    return this == otherHost;
}

/**
 * @brief Compares this host's network address against another's.
 * @param h Pointer to the comparison host.
 * @return Negative if this < h, 0 if equal, positive if this > h.
 */
int DTNHost::compareTo(const DTNHost* h) const {
    return this->address - h->getAddress();
}

/**
 * @brief Enables sorting and standard C++ container placement for DTNHosts based on address.
 * @param other Reference to the comparison host.
 * @return True if this host's address is strictly less than the other's.
 */
bool DTNHost::operator<(const DTNHost& other) const {
    return this->address < other.address;
}

// ============================================================================
// Additional ML/Testing Metrics
// ============================================================================

/**
 * @brief Appends a contact duration block for Machine Learning / RL metric tracking.
 * @param dur The Duration object representing contact time bounds.
 */
void DTNHost::addDuration(const routing::community::Duration& dur) {
    this->intervals.push_back(dur);
}

/**
 * @brief Serializes the recorded contact durations into a formatted string block.
 * @return A string representing logged contact intervals.
 */
std::string DTNHost::getNodeIntervals() const {
    std::ostringstream oss;
    for (const auto& d : this->intervals) {
        // Retrieves duration boundaries safely using the provided getters
        oss << "<" << d.getStart() << ", " << d.getEnd() << "> ";
    }
    return oss.str();
}

} // namespace core