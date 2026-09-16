#pragma once

#include <vector>
#include <string>

#include "core/Coord.hpp" 

namespace movement {

/**
 * A Path between multiple Coordinates.
 */
class Path {
private:
    /** coordinates of the path */
    std::vector<core::Coord> coords;
    
    /** speeds in the path legs */
    std::vector<double> speeds;
    
    /** Storing the index of the next waypoint coordinate */
    int nextWpIndex;

public:
    /** Creates a path with zero speed. */
    Path();

    /** 
     * Copy constructor. Creates a copy of this path with a shallow copy of
     * the coordinates and speeds.
     */
    Path(const Path& path);

    /** Creates a path with constant speed */
    explicit Path(double speed);

    /** Sets a constant speed for the whole path. */
    void setSpeed(double speed);

    /** Returns a reference to the coordinates of this path */
    const std::vector<core::Coord>& getCoords() const;

    /** Adds a new waypoint to the end of the path. */
    void addWaypoint(const core::Coord& wp);

    /** Adds a new waypoint with a speed towards that waypoint */
    void addWaypoint(const core::Coord& wp, double speed);

    /** Returns the next waypoint on this path */
    core::Coord getNextWaypoint();

    /** Returns the first waypoint on this path. */
    core::Coord getFirstWaypoint() const;

    /** Returns the last waypoint on this path. */
    core::Coord getLastWaypoint() const;

    /** Returns true if the path has more waypoints, false if not */
    bool hasNext() const;

    /** Returns the speed towards the next waypoint */
    double getSpeed() const;

    /** Returns a string presentation of the path's coordinates */
    std::string toString() const;

    /** Returns a reference to the speeds of this path */
    const std::vector<double>& getSpeeds() const;
};

} // namespace movement