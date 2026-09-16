/**
 * @file SimulationScenario.cpp
 * @brief Implementation of the SimulationScenario class.
 * @details This file implements the creation and parsing of scenario settings.
 * @author Opeteer
 * @date September, 2026
 */

#include "core/SimulationScenario.hpp"
#include "core/ConfigurationError.hpp"

namespace core {

const std::string SimulationScenario::SCENARIO_NS = "Scenario";
const std::string SimulationScenario::NROF_GROUPS_S = "nrofHostGroups";
const std::string SimulationScenario::NROF_INTTYPES_S = "nrofInterfaceTypes";
const std::string SimulationScenario::NAME_S = "name";
const std::string SimulationScenario::END_TIME_S = "endTime";
const std::string SimulationScenario::UP_INT_S = "updateInterval";
const std::string SimulationScenario::SIM_CON_S = "simulateConnections";

const std::string SimulationScenario::INTTYPE_NS = "Interface";
const std::string SimulationScenario::INTTYPE_S = "type";
const std::string SimulationScenario::INTNAME_S = "name";

const std::string SimulationScenario::APPTYPE_NS = "Application";
const std::string SimulationScenario::APPTYPE_S = "type";
const std::string SimulationScenario::APPCOUNT_S = "nrofApplications";

const std::string SimulationScenario::GROUP_NS = "Group";
const std::string SimulationScenario::GROUP_ID_S = "groupID";
const std::string SimulationScenario::NROF_HOSTS_S = "nrofHosts";
const std::string SimulationScenario::SCAN_INTERVAL_S = "scanInterval";
const std::string SimulationScenario::MOVEMENT_MODEL_S = "movementModel";
const std::string SimulationScenario::ROUTER_S = "router";
const std::string SimulationScenario::NROF_INTERF_S = "nrofInterfaces";
const std::string SimulationScenario::INTERFACENAME_S = "interface";
const std::string SimulationScenario::GAPPNAME_S = "application";
const std::string SimulationScenario::GROUP_COLOR_S = "color";

const std::string SimulationScenario::MM_PACKAGE = "movement.";
const std::string SimulationScenario::ROUTING_PACKAGE = "routing.";
const std::string SimulationScenario::INTTYPE_PACKAGE = "interfaces.";
const std::string SimulationScenario::APP_PACKAGE = "applications.";

std::shared_ptr<SimulationScenario> SimulationScenario::myinstance = nullptr;

SimulationScenario::SimulationScenario() {
    // Constructor placeholder
}

std::shared_ptr<SimulationScenario> SimulationScenario::getInstance() {
    if (myinstance == nullptr) {
        myinstance = std::shared_ptr<SimulationScenario>(new SimulationScenario());
    }
    return myinstance;
}

void SimulationScenario::reset() {
    myinstance = nullptr;
}

void SimulationScenario::ensurePositiveValue(double value, const std::string& settingName) {
    if (value < 0) {
        throw core::ConfigurationError("Negative value (" + std::to_string(value) + 
                                       ") not accepted for setting " + settingName);
    }
}

void SimulationScenario::createHosts() {
    // TODO: Implement host creation logic.
    // In Java, this method uses reflection (Settings.createInitializedObject) 
    // to dynamically instantiate MovementModels and MessageRouters based on class names.
    // In C++, we will need to implement a Factory pattern to achieve similar behavior.
}

std::string SimulationScenario::getName() const { return name; }
bool SimulationScenario::simulateConnections() const { return simulateConns; }
int SimulationScenario::getWorldSizeX() const { return worldSizeX; }
int SimulationScenario::getWorldSizeY() const { return worldSizeY; }
double SimulationScenario::getEndTime() const { return endTime; }
double SimulationScenario::getUpdateInterval() const { return updateInterval; }
double SimulationScenario::getMaxHostRange() const { return maxHostRange; }

std::vector<std::shared_ptr<EventQueue>> SimulationScenario::getExternalEvents() const {
    return {};
}

std::shared_ptr<movement::SimMap> SimulationScenario::getMap() const {
    return simMap;
}

void SimulationScenario::addConnectionListener(std::shared_ptr<ConnectionListener> cl) {
    connectionListeners.push_back(cl);
}

void SimulationScenario::addMessageListener(std::shared_ptr<MessageListener> ml) {
    messageListeners.push_back(ml);
}

void SimulationScenario::addMovementListener(std::shared_ptr<MovementListener> ml) {
    movementListeners.push_back(ml);
}

void SimulationScenario::addUpdateListener(std::shared_ptr<UpdateListener> ul) {
    updateListeners.push_back(ul);
}

const std::vector<std::shared_ptr<UpdateListener>>& SimulationScenario::getUpdateListeners() const {
    return updateListeners;
}

const std::vector<std::shared_ptr<DTNHost>>& SimulationScenario::getHosts() const {
    return hosts;
}

std::shared_ptr<World> SimulationScenario::getWorld() const {
    return world;
}

} // namespace core
