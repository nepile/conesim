/**
 * @file SimulationScenario.hpp
 * @brief A simulation scenario used for getting and storing the settings of a simulation run.
 * @details This class is adapted from The ONE simulator's SimScenario.java.
 *          It handles the creation of hosts, event queues, and listeners. 
 *          Note that dynamic class loading via Reflection is not natively supported in C++,
 *          so object creation (like createHosts) will require a Factory pattern later.
 * @author Opeteer
 * @date September, 2026
 */

#pragma once

#include <string>
#include <vector>
#include <memory>

#include "core/Configuration.hpp"
#include "core/DTNHost.hpp"
#include "core/ConnectionListener.hpp"
#include "core/MessageListener.hpp"
#include "core/MovementListener.hpp"
#include "core/UpdateListener.hpp"

// Forward Declarations for classes that might not exist yet
namespace core {
    class World;
    class NetworkInterface;
    class Application;
    class EventQueueHandler;
    class EventQueue;
}

namespace movement {
    class MovementModel;
    class SimMap;
}

namespace routing {
    class MessageRouter;
}

namespace core {

class SimulationScenario {
public:
    static const std::string SCENARIO_NS;
    static const std::string NROF_GROUPS_S;
    static const std::string NROF_INTTYPES_S;
    static const std::string NAME_S;
    static const std::string END_TIME_S;
    static const std::string UP_INT_S;
    static const std::string SIM_CON_S;

    static const std::string INTTYPE_NS;
    static const std::string INTTYPE_S;
    static const std::string INTNAME_S;

    static const std::string APPTYPE_NS;
    static const std::string APPTYPE_S;
    static const std::string APPCOUNT_S;

    static const std::string GROUP_NS;
    static const std::string GROUP_ID_S;
    static const std::string NROF_HOSTS_S;
    static const std::string SCAN_INTERVAL_S;
    static const std::string MOVEMENT_MODEL_S;
    static const std::string ROUTER_S;
    static const std::string NROF_INTERF_S;
    static const std::string INTERFACENAME_S;
    static const std::string GAPPNAME_S;
    static const std::string GROUP_COLOR_S;

private:
    static const std::string MM_PACKAGE;
    static const std::string ROUTING_PACKAGE;
    static const std::string INTTYPE_PACKAGE;
    static const std::string APP_PACKAGE;

    static std::shared_ptr<SimulationScenario> myinstance;

protected:
    std::shared_ptr<World> world;
    std::vector<std::shared_ptr<DTNHost>> hosts;
    
    std::string name;
    int nrofGroups;
    int worldSizeX;
    int worldSizeY;
    double maxHostRange;
    double endTime;
    double updateInterval;
    bool simulateConns;

    std::shared_ptr<movement::SimMap> simMap;
    std::shared_ptr<EventQueueHandler> eqHandler;

    std::vector<std::shared_ptr<ConnectionListener>> connectionListeners;
    std::vector<std::shared_ptr<MessageListener>> messageListeners;
    std::vector<std::shared_ptr<MovementListener>> movementListeners;
    std::vector<std::shared_ptr<UpdateListener>> updateListeners;
    // std::vector<std::shared_ptr<ApplicationListener>> appListeners;

    /**
     * @brief Makes sure that a value is positive.
     * @param value Value to check
     * @param settingName Name of the setting (for error message)
     */
    void ensurePositiveValue(double value, const std::string& settingName);

    /**
     * @brief Creates hosts for the scenario.
     */
    virtual void createHosts();

    /**
     * @brief Constructor that creates a scenario based on Configuration.
     */
    SimulationScenario();

public:
    virtual ~SimulationScenario() = default;

    /**
     * @brief Returns the SimulationScenario instance and creates one if it doesn't exist yet.
     */
    static std::shared_ptr<SimulationScenario> getInstance();

    static void reset();

    std::string getName() const;
    bool simulateConnections() const;
    
    int getWorldSizeX() const;
    int getWorldSizeY() const;
    
    double getEndTime() const;
    double getUpdateInterval() const;
    double getMaxHostRange() const;

    std::vector<std::shared_ptr<EventQueue>> getExternalEvents() const;
    std::shared_ptr<movement::SimMap> getMap() const;

    void addConnectionListener(std::shared_ptr<ConnectionListener> cl);
    void addMessageListener(std::shared_ptr<MessageListener> ml);
    void addMovementListener(std::shared_ptr<MovementListener> ml);
    void addUpdateListener(std::shared_ptr<UpdateListener> ul);

    const std::vector<std::shared_ptr<UpdateListener>>& getUpdateListeners() const;
    const std::vector<std::shared_ptr<DTNHost>>& getHosts() const;
    std::shared_ptr<World> getWorld() const;
};

} // namespace core
