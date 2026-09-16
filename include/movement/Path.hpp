/**
 * @file Path.hpp
 * @brief Defines the Path class representing a sequence of coordinates and traversal speeds.
 */

#pragma once

#include <vector>
#include <string>

#include "core/Coord.hpp" 

namespace movement {

/**
 * @class Path
 * @brief A Path between multiple Coordinates.
 * 
 * This class encapsulates a sequence of waypoints and the speeds at which 
 * the path legs are traversed by a node in the simulation.
 */
class Path {
private:
    /** @brief Coordinates representing the waypoints of the path. */
    std::vector<core::Coord> coords;
    
    /** @brief Speeds applied to each leg of the path. */
    std::vector<double> speeds;
    
    /** @brief Index pointing to the next waypoint coordinate to be visited. */
    int nextWpIndex;

public:
    /** 
     * @brief Default constructor. Creates a path with zero speed and no waypoints. 
     */
    Path();

    /** 
     * @brief Copy constructor. 
     * 
     * Creates a copy of this path with a shallow copy of the coordinates and speeds.
     * 
     * @param path The Path object to copy from.
     */
    Path(const Path& path);

    /** 
     * @brief Creates a path with a constant speed.
     * 
     * @param speed The constant speed to be applied to the entire path.
     */
    explicit Path(double speed);

    /** 
     * @brief Sets a constant speed for the whole path.
     * 
     * Any previously set varying speeds will be discarded and replaced by this single speed.
     * 
     * @param speed The new constant speed for the path.
     */
    void setSpeed(double speed);

    /** 
     * @brief Returns a reference to the coordinates of this path.
     * 
     * @return A constant reference to the vector containing the path's coordinates.
     */
    const std::vector<core::Coord>& getCoords() const;

    /** 
     * @brief Adds a new waypoint to the end of the path.
     * 
     * This method assumes the path uses a constant speed.
     * 
     * @param wp The waypoint coordinate to add.
     */
    void addWaypoint(const core::Coord& wp);

    /** 
     * @brief Adds a new waypoint with a specific speed towards that waypoint.
     * 
     * @param wp The waypoint coordinate to add.
     * @param speed The speed used to travel towards this newly added waypoint.
     */
    void addWaypoint(const core::Coord& wp, double speed);

    /** 
     * @brief Retrieves the next waypoint on this path and advances the internal index.
     * 
     * @return The next waypoint coordinate.
     */
    core::Coord getNextWaypoint();

    /** 
     * @brief Retrieves the first waypoint on this path.
     * 
     * @return The first waypoint coordinate.
     */
    core::Coord getFirstWaypoint() const;

    /** 
     * @brief Retrieves the last waypoint on this path.
     * 
     * @return The last waypoint coordinate.
     */
    core::Coord getLastWaypoint() const;

    /** 
     * @brief Checks if the path has more waypoints to visit.
     * 
     * @return true if there are remaining waypoints, false otherwise.
     */
    bool hasNext() const;

    /** 
     * @brief Retrieves the speed towards the next waypoint.
     * 
     * @return The speed value towards the next waypoint.
     */
    double getSpeed() const;

    /** 
     * @brief Returns a string representation of the path's coordinates and speeds.
     * 
     * @return A formatted string representing the path.
     */
    std::string toString() const;

    /** 
     * @brief Retrieves a reference to the speeds of this path.
     * 
     * @return A constant reference to the vector of speeds.
     */
    const std::vector<double>& getSpeeds() const;
};

} // namespace movement