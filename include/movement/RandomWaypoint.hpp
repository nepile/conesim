/**
 * @file RandomWaypoint.hpp
 * @brief Random waypoint movement model.
 * @details Creates zig-zag paths within the simulation area.
 * @author Opeteer
 * @date September, 2026
 */

#pragma once

#include <memory>

#include "movement/MovementModel.hpp"
#include "core/Coord.hpp"
#include "core/Configuration.hpp"

// Forward declaration if Path is not yet fully included from MovementModel
namespace movement {
    class Path;
}

namespace movement {

class RandomWaypoint : public MovementModel {
private:
    /** how many waypoints should there be per path */
    static const int PATH_LENGTH = 1;

protected:
    core::Coord lastWaypoint;

    /**
     * @brief Copy-constructor. Creates a new RandomWaypoint based on a prototype.
     * @param rwp The RandomWaypoint prototype.
     */
    RandomWaypoint(const RandomWaypoint& rwp);

    /**
     * @brief Generates a random coordinate within the map boundaries.
     * @return A randomly generated Coord.
     */
    virtual core::Coord randomCoord();

public:
    /**
     * @brief Creates a new RandomWaypoint based on a Configuration object's settings.
     * @param settings The Configuration object.
     */
    explicit RandomWaypoint(const core::Configuration& settings);

    virtual ~RandomWaypoint() = default;

    /**
     * @brief Returns a possible (random) placement for a host.
     * @return Random position on the map.
     */
    core::Coord getInitialLocation() override;

    /**
     * @brief Returns a new path by this movement model.
     * @return A new path.
     */
    Path getPath() override;

    /**
     * @brief Creates a replicate of the movement model.
     * @return A shared_ptr to a new RandomWaypoint.
     */
    std::shared_ptr<MovementModel> replicate() const override;
};

} // namespace movement
