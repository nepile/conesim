#include "movement/RandomWaypoint.hpp"
#include "movement/Path.hpp"
#include <cassert>

namespace movement {

RandomWaypoint::RandomWaypoint(const core::Configuration& settings) 
    : MovementModel(settings), lastWaypoint(0.0, 0.0)
{
}

RandomWaypoint::RandomWaypoint(const RandomWaypoint& rwp) 
    : MovementModel(rwp), lastWaypoint(rwp.lastWaypoint) 
{
}

core::Coord RandomWaypoint::getInitialLocation() {
    assert(rngInitialized && "MovementModel not initialized!");
    core::Coord c = randomCoord();
    this->lastWaypoint = c;
    return c;
}

Path RandomWaypoint::getPath() {
    Path p(generateSpeed());
    
    p.addWaypoint(lastWaypoint);
    
    core::Coord c = lastWaypoint;
    for (int i = 0; i < PATH_LENGTH; i++) {
        c = randomCoord();
        p.addWaypoint(c);
    }
    
    this->lastWaypoint = c;
    return p;
}

std::shared_ptr<MovementModel> RandomWaypoint::replicate() const {
    // std::make_shared tidak dapat mengakses constructor berstatus protected (protected copy-constructor).
    // Oleh karena itu, kita membuat instance dengan `new` dan lalu membungkusnya dalam shared_ptr.
    return std::shared_ptr<MovementModel>(new RandomWaypoint(*this));
}

core::Coord RandomWaypoint::randomCoord() {
    std::uniform_real_distribution<double> distX(0, getMaxX());
    std::uniform_real_distribution<double> distY(0, getMaxY());
    return core::Coord(distX(rng), distY(rng));
}

} // namespace movement
