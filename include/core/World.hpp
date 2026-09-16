/**
 * @file World.hpp
 * @brief World contains all the nodes and is responsible for updating their location and connections.
 * @details This class is adapted from The ONE simulator's World.java.
 * @author Opeteer
 * @date September, 2026
 */

#pragma once

#include <vector>
#include <memory>
#include <string>

#include "core/DTNHost.hpp"
#include "core/UpdateListener.hpp"
#include "core/SimulationClock.hpp"

// Forward Declarations for classes from the 'input' module which may not exist yet
namespace input {
    class EventQueue;
    class ScheduledUpdatesQueue;
    class ExternalEvent;
}

namespace core {

class World {
public:
    static const std::string SETTINGS_NS;
    static const std::string CELL_SIZE_MULT_S;
    static const std::string RANDOMIZE_UPDATES_S;
    
    static const int DEF_CON_CELL_SIZE_MULT;
    static const bool DEF_RANDOMIZE_UPDATES;

private:
    int sizeX;
    int sizeY;
    std::vector<std::shared_ptr<input::EventQueue>> eventQueues;
    double updateInterval;
    
    double nextQueueEventTime;
    std::shared_ptr<input::EventQueue> nextEventQueue;
    
    std::vector<std::shared_ptr<DTNHost>> hosts;
    bool simulateConnections;
    
    std::vector<std::shared_ptr<DTNHost>> updateOrder;
    bool isCancelled;
    
    std::vector<std::shared_ptr<UpdateListener>> updateListeners;
    std::shared_ptr<input::ScheduledUpdatesQueue> scheduledUpdates;

    int conCellSizeMult;

    /**
     * @brief Initializes settings fields that can be configured using Configuration class
     */
    void initSettings();

    /**
     * @brief Updates all hosts. If update order randomizing is on, calls are made in random order.
     */
    void updateHosts();

    /**
     * @brief Moves all hosts in the world for a given amount of time
     * @param timeIncrement The time how long all nodes should move
     */
    void moveHosts(double timeIncrement);

public:
    /**
     * @brief Constructor.
     */
    World(std::vector<std::shared_ptr<DTNHost>> hosts, int sizeX, int sizeY, 
          double updateInterval, std::vector<std::shared_ptr<UpdateListener>> updateListeners,
          bool simulateConnections, std::vector<std::shared_ptr<input::EventQueue>> eventQueues);

    virtual ~World() = default;

    /**
     * @brief Moves hosts in the world for the given time to initialize host positions properly.
     * @param time The total time (seconds) to move
     */
    void warmupMovementModel(double time);

    /**
     * @brief Goes through all event Queues and sets the event queue that has the next event.
     */
    void setNextEventQueue();

    /**
     * @brief Update (move, connect, disconnect etc.) all hosts in the world.
     */
    void update();

    /**
     * @brief Asynchronously cancels the currently running simulation
     */
    void cancelSim();

    /**
     * @brief Returns the hosts in a list
     */
    const std::vector<std::shared_ptr<DTNHost>>& getHosts() const;

    /**
     * @brief Returns the x-size (width) of the world 
     */
    int getSizeX() const;

    /**
     * @brief Returns the y-size (height) of the world 
     */
    int getSizeY() const;

    /**
     * @brief Returns a node from the world by its address
     * @param address The address of the node
     * @return The requested node or nullptr if it wasn't found
     */
    std::shared_ptr<DTNHost> getNodeByAddress(int address) const;

    /**
     * @brief Schedules an update request to all nodes to happen at the specified simulation time.
     * @param simTime The time of the update
     */
    void scheduleUpdate(double simTime);
};

} // namespace core
