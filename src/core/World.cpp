/**
 * @file World.cpp
 * @brief Implementation of the World class containing all nodes and simulation logic.
 * @details Adapted from The ONE simulator's World.java.
 * @author Opeteer
 * @date September, 2026
 */

#include "core/World.hpp"
#include "core/Configuration.hpp"
#include "core/ConfigurationError.hpp"
#include "core/SimulationClock.hpp"
#include "core/SimulationError.hpp"

#include <algorithm>
#include <random>
#include <cassert>
#include <iostream>

namespace core {

const std::string World::SETTINGS_NS = "Optimization";
const std::string World::CELL_SIZE_MULT_S = "cellSizeMult";
const std::string World::RANDOMIZE_UPDATES_S = "randomizeUpdateOrder";

const int World::DEF_CON_CELL_SIZE_MULT = 5;
const bool World::DEF_RANDOMIZE_UPDATES = true;

World::World(std::vector<std::shared_ptr<DTNHost>> hosts, int sizeX, int sizeY, 
      double updateInterval, std::vector<std::shared_ptr<UpdateListener>> updateListeners,
      bool simulateConnections, std::vector<std::shared_ptr<input::EventQueue>> eventQueues)
    : sizeX(sizeX), sizeY(sizeY), eventQueues(eventQueues),
      updateInterval(updateInterval), hosts(hosts), 
      simulateConnections(simulateConnections),
      isCancelled(false), updateListeners(updateListeners)
{
    // TODO: Initialize ScheduledUpdatesQueue when input module is available
    // scheduledUpdates = std::make_shared<input::ScheduledUpdatesQueue>();

    setNextEventQueue();
    initSettings();
}

void World::initSettings() {
    core::Configuration config(SETTINGS_NS);
    bool randomizeUpdates = DEF_RANDOMIZE_UPDATES;

    if (config.contains(RANDOMIZE_UPDATES_S)) {
        randomizeUpdates = config.getBoolean(RANDOMIZE_UPDATES_S);
    }

    if (randomizeUpdates) {
        // creates the update order array that can be shuffled
        this->updateOrder = this->hosts;
    } else {
        // empty vector means "don't randomize" in this implementation
        this->updateOrder.clear();
    }

    if (config.contains(CELL_SIZE_MULT_S)) {
        conCellSizeMult = config.getInt(CELL_SIZE_MULT_S);
    } else {
        conCellSizeMult = DEF_CON_CELL_SIZE_MULT;
    }

    // check that values are within limits
    if (conCellSizeMult < 2) {
        throw core::ConfigurationError("Too small value (" + std::to_string(conCellSizeMult) +
                ") for " + SETTINGS_NS + "." + CELL_SIZE_MULT_S);
    }
}

void World::warmupMovementModel(double time) {
    if (time <= 0) {
        return;
    }

    auto clock = core::SimulationClock::getInstance();

    while (core::SimulationClock::getTime() < -updateInterval) {
        moveHosts(updateInterval);
        clock->advance(updateInterval);
    }

    double finalStep = -core::SimulationClock::getTime();

    moveHosts(finalStep);
    clock->setTime(0);
}

void World::setNextEventQueue() {
    // TODO: Implement EventQueue traversal when input module is available
    /*
    std::shared_ptr<input::EventQueue> nextQueue = scheduledUpdates;
    double earliest = nextQueue->nextEventsTime();

    for (auto& eq : eventQueues) {
        if (eq->nextEventsTime() < earliest) {
            nextQueue = eq;
            earliest = eq->nextEventsTime();
        }
    }

    this->nextEventQueue = nextQueue;
    this->nextQueueEventTime = earliest;
    */
    this->nextQueueEventTime = 1e9; // Dummy value to prevent infinite loop for now
}

void World::update() {
    auto clock = core::SimulationClock::getInstance();
    double runUntil = core::SimulationClock::getTime() + this->updateInterval;

    setNextEventQueue();

    // process all events that are due until next interval update
    while (this->nextQueueEventTime <= runUntil) {
        // TODO: Enable EventQueue processing when input module is available
        /*
        clock->setTime(this->nextQueueEventTime);
        auto ee = this->nextEventQueue->nextEvent();
        ee->processEvent(this);
        updateHosts(); // update all hosts after every event
        setNextEventQueue();
        */
        break; // Break immediately since nextQueueEventTime is a dummy value
    }

    moveHosts(this->updateInterval);
    clock->setTime(runUntil);

    updateHosts();

    std::vector<DTNHost*> rawHosts;
    rawHosts.reserve(this->hosts.size());
    for (const auto& h : this->hosts) {
        rawHosts.push_back(h.get());
    }

    // inform all update listeners
    for (auto& ul : this->updateListeners) {
        ul->updated(rawHosts);
    }
}

void World::updateHosts() {
    if (this->updateOrder.empty()) { // randomizing is off
        for (auto& host : hosts) {
            if (this->isCancelled) {
                break;
            }
            host->update(simulateConnections);
        }
    } else { // update order randomizing is on
        assert(this->updateOrder.size() == this->hosts.size() && 
               "Nrof hosts has changed unexpectedly");
               
        std::mt19937 rng(core::SimulationClock::getIntTime());
        std::shuffle(this->updateOrder.begin(), this->updateOrder.end(), rng);
        
        for (auto& host : this->updateOrder) {
            if (this->isCancelled) {
                break;
            }
            host->update(simulateConnections);
        }           
    }
}

void World::moveHosts(double timeIncrement) {
    for (auto& host : hosts) {
        host->move(timeIncrement);
    }
}

void World::cancelSim() {
    this->isCancelled = true;
}

const std::vector<std::shared_ptr<DTNHost>>& World::getHosts() const {
    return this->hosts;
}

int World::getSizeX() const {
    return this->sizeX;
}

int World::getSizeY() const {
    return this->sizeY;
}

std::shared_ptr<DTNHost> World::getNodeByAddress(int address) const {
    if (address < 0 || address >= static_cast<int>(hosts.size())) {
        throw core::SimulationError("No host for address " + std::to_string(address) + 
            ". Address range of 0-" + std::to_string(hosts.size()-1) + " is valid");
    }

    std::shared_ptr<DTNHost> node = this->hosts[address];
    assert(node->getAddress() == address && "Node indexing failed.");

    return node; 
}

void World::scheduleUpdate(double simTime) {
    // TODO: Implement ScheduledUpdatesQueue when input module is available
    // scheduledUpdates->addUpdate(simTime);
}

} // namespace core
