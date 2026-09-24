/**
 * @file SimpleBroadcastInterface.cpp
 * @brief Implementation of SimpleBroadcastInterface.
 * @details Adapted from The ONE simulator's SimpleBroadcastInterface.java.
 * @author Neville
 * @date September, 2026
 */

#include "interfaces/SimpleBroadcastInterface.hpp"
#include "interfaces/ConnectivityOptimizer.hpp"
#include "core/CBRConnection.hpp"
#include "core/DTNHost.hpp"

#include <cassert>
#include <algorithm>
#include <vector>

namespace interfaces {

SimpleBroadcastInterface::SimpleBroadcastInterface(const core::Configuration& config)
    : core::NetworkInterface(config) {
}

SimpleBroadcastInterface::SimpleBroadcastInterface(const SimpleBroadcastInterface& ni)
    : core::NetworkInterface(ni) {
}

core::NetworkInterface* SimpleBroadcastInterface::replicate() {
    return new SimpleBroadcastInterface(*this);
}

void SimpleBroadcastInterface::connect(core::NetworkInterface* anotherInterface) {
    if (anotherInterface == nullptr || this == anotherInterface) {
        return;
    }

    core::DTNHost* anotherHost = anotherInterface->getHost();
    if (this->host == nullptr || anotherHost == nullptr) {
        return;
    }

    if (isScanning()
            && anotherHost->isActive()
            && isWithinRange(anotherInterface)
            && !isConnected(anotherInterface)) {
        // new contact within range
        // connection speed is the lower one of the two speeds
        int conSpeed = std::min(this->transmitSpeed, anotherInterface->getTransmitSpeed());

        core::Connection* con = new core::CBRConnection(
            this->host, this,
            anotherHost, anotherInterface,
            conSpeed
        );
        core::NetworkInterface::connect(con, anotherInterface);
    }
}

void SimpleBroadcastInterface::update() {
    if (this->optimizer == nullptr) {
        return; // nothing to do
    }

    // First break the old ones
    this->optimizer->updateLocation(this);
    for (size_t i = 0; i < this->connections.size();) {
        core::Connection* con = this->connections[i];
        core::NetworkInterface* anotherInterface = con->getOtherInterface(this);

        // all connections should be up at this stage
        assert(con->isUp() && "Connection was down!");

        if (!isWithinRange(anotherInterface)) {
            disconnect(con, anotherInterface);
            this->connections.erase(this->connections.begin() + i);
        } else {
            ++i;
        }
    }

    // Then find new possible connections
    std::vector<core::NetworkInterface*> nearInterfaces = this->optimizer->getNearInterfaces(this);
    for (core::NetworkInterface* interf : nearInterfaces) {
        connect(interf);
    }
}

void SimpleBroadcastInterface::createConnection(core::NetworkInterface* anotherInterface) {
    if (anotherInterface == nullptr || this == anotherInterface) {
        return;
    }

    core::DTNHost* anotherHost = anotherInterface->getHost();
    if (this->host == nullptr || anotherHost == nullptr) {
        return;
    }

    if (!isConnected(anotherInterface)) {
        // connection speed is the lower one of the two speeds
        int conSpeed = std::min(this->transmitSpeed, anotherInterface->getTransmitSpeed());

        core::Connection* con = new core::CBRConnection(
            this->host, this,
            anotherHost, anotherInterface,
            conSpeed
        );
        core::NetworkInterface::connect(con, anotherInterface);
    }
}

std::string SimpleBroadcastInterface::toString() const {
    return "SimpleBroadcastInterface " + core::NetworkInterface::toString();
}

} // namespace interfaces
