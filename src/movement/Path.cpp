#include "movement/Path.hpp"
#include <cassert>
#include <sstream>
#include <iomanip>

namespace movement {

Path::Path() : nextWpIndex(0) {
    coords.reserve(10);
    speeds.reserve(1);
}

Path::Path(const Path& path) 
    : coords(path.coords), speeds(path.speeds), nextWpIndex(path.nextWpIndex) {
}

Path::Path(double speed) : Path() {
    setSpeed(speed);
}

void Path::setSpeed(double speed) {
    speeds.clear();
    speeds.push_back(speed);
}

const std::vector<core::Coord>& Path::getCoords() const {
    return this->coords;
}

void Path::addWaypoint(const core::Coord& wp) {
    assert(this->speeds.size() <= 1 && "This method should be used only for paths with constant speed");
    this->coords.push_back(wp);
}

void Path::addWaypoint(const core::Coord& wp, double speed) {
    this->coords.push_back(wp);
    this->speeds.push_back(speed);
}

core::Coord Path::getNextWaypoint() {
    assert(hasNext() && "Path didn't have next waypoint");
    return coords[nextWpIndex++];
}

core::Coord Path::getFirstWaypoint() const {
    assert(hasNext() && "Path didn't have next waypoint");
    return coords.front();
}

core::Coord Path::getLastWaypoint() const {
    assert(nextWpIndex != 0 && "No waypoint asked");
    return coords.back();
}

bool Path::hasNext() const {
    return nextWpIndex < static_cast<int>(coords.size());
}

double Path::getSpeed() const {
    assert(!speeds.empty() && "No speed set");
    assert(nextWpIndex != 0 && "No waypoint asked");
    
    if (speeds.size() == 1) {
        return speeds[0];
    } else {
        return speeds[nextWpIndex - 1];
    }
}

std::string Path::toString() const {
    std::ostringstream oss;
    for (size_t i = 0; i < coords.size(); i++) {
        oss << "->" << coords[i].toString(); 
        if (speeds.size() > 1 && i < speeds.size()) {
            oss << "@" << std::fixed << std::setprecision(2) << speeds[i] << " ";
        }
    }
    return oss.str();
}

const std::vector<double>& Path::getSpeeds() const {
    return this->speeds;
}

} // namespace movement