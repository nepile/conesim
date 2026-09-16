/**
 * @file MovementModel.hpp
 * @brief Superclass/Abstraction for all movement models.
 * @details Superclass for all active movement (e.g., RandomWaypoint, RandomWalk).
 *          This class is adapted from The ONE simulator's MovementModel.java.
 *          All subclasses must implement getPath(), getInitialLocation(),
 *          and replicate().
 * @author Opeteer
 * @date September, 2026
 */

#pragma once

#include <string>
#include <vector>
#include <memory>
#include <random>

#include "core/Configuration.hpp"
#include "core/Coord.hpp"

// Forward declarations for classes that might not be implemented yet
// Might be Modified in the future
namespace core {
    class ModuleCommunicationBus;
}

namespace movement {

class Path;
class ActivenessHandler;

class MovementModel {
public:
    static const std::string SPEED;
    static const std::string WAIT_TIME;

    static const std::vector<double> DEF_SPEEDS;
    static const std::vector<double> DEF_WAIT_TIMES;

    static const std::string MOVEMENT_MODEL_NS;
    static const std::string WORLD_SIZE;
    static const std::string RNG_SEED;

protected:
    // Random number generator shared by all movement models
    static std::mt19937 rng;
    static bool rngInitialized;

    std::shared_ptr<ActivenessHandler> ah;

    double minSpeed;
    double maxSpeed;
    double minWaitTime;
    double maxWaitTime;

    int maxX;
    int maxY;

    std::shared_ptr<core::ModuleCommunicationBus> comBus;

    /**
     * @brief Checks that the minimum setting is not bigger than the maximum and that both are positive.
     * @param name Name of the setting
     * @param min The minimum setting
     * @param max The maximum setting
     */
    static void checkMinAndMaxSetting(const std::string& name, double min, double max);

public:
    /**
     * @brief Empty constructor for testing purposes.
     */
    MovementModel();

    /**
     * @brief Creates a new MovementModel based on a Configuration object's settings.
     * @param settings The Configuration object where the settings are read from
     */
    explicit MovementModel(const core::Configuration& settings);

    /**
     * @brief Copy-constructor. Creates a new MovementModel based on the given prototype.
     * @param mm The MovementModel prototype to base the new object on
     */
    MovementModel(const MovementModel& mm);

    virtual ~MovementModel() = default;

    int getMaxX() const;
    int getMaxY() const;

    virtual bool isActive() const;
    virtual double nextPathAvailable() const;

    void setComBus(std::shared_ptr<core::ModuleCommunicationBus> bus);
    std::shared_ptr<core::ModuleCommunicationBus> getComBus() const;

    virtual std::string toString() const;

    /**
     * @brief Returns a new path by this movement model.
     * @return A new path.
     */
    virtual Path getPath() = 0;

    /**
     * @brief Returns a new initial placement for a node.
     * @return The initial coordinates.
     */
    virtual core::Coord getInitialLocation() = 0;

    /**
     * @brief Creates a replicate of the movement model.
     * @return A shared_ptr to a new movement model with the same settings as this model.
     */
    virtual std::shared_ptr<MovementModel> replicate() const = 0;

    /**
     * @brief Resets all static fields to default values.
     */
    static void reset();

protected:
    virtual double generateSpeed();
    virtual double generateWaitTime();
};

} // namespace movement
