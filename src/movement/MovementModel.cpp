/**
 * @file MovementModel.cpp
 * @brief Implementation of the MovementModel base class.
 * @author Opeteer
 * @date September, 2026
 */

#include "movement/MovementModel.hpp"
#include "core/SimulationClock.hpp"
#include "core/SimulationError.hpp"
#include <chrono>

namespace movement {

const std::string MovementModel::SPEED = "speed";
const std::string MovementModel::WAIT_TIME = "waitTime";

const std::vector<double> MovementModel::DEF_SPEEDS = { 1.0, 1.0 };
const std::vector<double> MovementModel::DEF_WAIT_TIMES = { 0.0, 0.0 };

const std::string MovementModel::MOVEMENT_MODEL_NS = "MovementModel";
const std::string MovementModel::WORLD_SIZE = "worldSize";
const std::string MovementModel::RNG_SEED = "rngSeed";

std::mt19937 MovementModel::rng;
bool MovementModel::rngInitialized = false;

void MovementModel::checkMinAndMaxSetting(const std::string& name, double min, double max) {
    if (min < 0.0 || max < 0.0) {
        throw core::SimulationError("MovementModel." + name + " (in Settings)" +
                " has a value less than zero (" + std::to_string(min) + ", " + std::to_string(max) + ")");
    }
    if (min > max) {
        throw core::SimulationError("MovementModel." + name + " (in Settings)" +
                " min is bigger than max (" + std::to_string(min) + ", " + std::to_string(max) + ")");
    }
}

MovementModel::MovementModel() 
    : ah(nullptr), minSpeed(1.0), maxSpeed(1.0), 
      minWaitTime(0.0), maxWaitTime(0.0), 
      maxX(0), maxY(0), comBus(nullptr) 
{
}

MovementModel::MovementModel(const core::Configuration& settings) 
    : ah(nullptr), comBus(nullptr) 
{
    // ah = std::make_shared<ActivenessHandler>(settings); // TODO: implement

    std::vector<double> speeds;
    if (settings.contains(SPEED)) {
        speeds = settings.getCsvDoubles(SPEED, 2);
    } else {
        speeds = DEF_SPEEDS;
    }
    minSpeed = speeds[0];
    maxSpeed = speeds[1];
    checkMinAndMaxSetting(SPEED, minSpeed, maxSpeed);

    std::vector<double> times;
    if (settings.contains(WAIT_TIME)) {
        times = settings.getCsvDoubles(WAIT_TIME, 2);
    } else {
        times = DEF_WAIT_TIMES;
    }
    minWaitTime = times[0];
    maxWaitTime = times[1];
    checkMinAndMaxSetting(WAIT_TIME, minWaitTime, maxWaitTime);

    core::Configuration s = settings;
    s.setNamespace(MOVEMENT_MODEL_NS);
    std::vector<int> worldSize = s.getCsvInts(WORLD_SIZE, 2);
    this->maxX = worldSize[0];
    this->maxY = worldSize[1];
}

MovementModel::MovementModel(const MovementModel& mm)
    : ah(mm.ah),
      minSpeed(mm.minSpeed),
      maxSpeed(mm.maxSpeed),
      minWaitTime(mm.minWaitTime),
      maxWaitTime(mm.maxWaitTime),
      maxX(mm.maxX),
      maxY(mm.maxY),
      comBus(nullptr) 
{
}

int MovementModel::getMaxX() const {
    return this->maxX;
}

int MovementModel::getMaxY() const {
    return this->maxY;
}

double MovementModel::generateSpeed() {
    if (!rngInitialized) {
        return 1.0;
    }
    std::uniform_real_distribution<double> dist(minSpeed, maxSpeed);
    return dist(rng);
}

double MovementModel::generateWaitTime() {
    if (!rngInitialized) {
        return 0.0;
    }
    std::uniform_real_distribution<double> dist(minWaitTime, maxWaitTime);
    return dist(rng);
}

bool MovementModel::isActive() const {
    // if (ah) return ah->isActive();
    return true; 
}

double MovementModel::nextPathAvailable() const {
    return core::SimulationClock::getTime() + const_cast<MovementModel*>(this)->generateWaitTime();
}

void MovementModel::setComBus(std::shared_ptr<core::ModuleCommunicationBus> bus) {
    this->comBus = bus;
}

std::shared_ptr<core::ModuleCommunicationBus> MovementModel::getComBus() const {
    return this->comBus;
}

std::string MovementModel::toString() const {
    return "MovementModel";
}

void MovementModel::reset() {
    core::Configuration s(MOVEMENT_MODEL_NS);
    if (s.contains(RNG_SEED) && (s.getInt(RNG_SEED) != 0)) {
        int seed = s.getInt(RNG_SEED);
        rng.seed(seed);
    } else {
        std::random_device rd;
        rng.seed(rd());
    }
    rngInitialized = true;
}

} // namespace movement
