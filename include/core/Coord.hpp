/**
 * @file Coord.hpp
 * @brief Definition of conesim::Coord (2D coordinate representation).
 * @author Agra
 * @date September
 * 
 * Ported from Aalto University, ComNet (Java) to C++
 */

#pragma once

#include <string>
#include <functional>

namespace core {

/**
 * Class to hold 2D coordinates and perform simple arithmetics and
 * transformations
 */
class Coord {
private:
    double x;
    double y;

public:
    /**
     * Constructor.
     * @param x Initial X-coordinate
     * @param y Initial Y-coordinate
     */
    Coord(double x, double y);

    // Copy Constructor (opsional, tapi baik untuk kejelasan)
    Coord(const Coord& other) = default;

    /**
     * Sets the location of this coordinate object
     */
    void setLocation(double x, double y);

    /**
     * Sets this coordinate's location to be equal to other
     * coordinates location
     */
    void setLocation(const Coord& c);

    /**
     * Moves the point by dx and dy
     */
    void translate(double dx, double dy);

    /**
     * Returns the distance to another coordinate
     */
    double distance(const Coord& other) const;

    /**
     * Returns the x coordinate
     */
    double getX() const;

    /**
     * Returns the y coordinate
     */
    double getY() const;

    /**
     * Returns a text representation of the coordinate (rounded to 2 decimals)
     */
    std::string toString() const;

    /**
     * Returns a clone of this coordinate.
     * Note: In C++, returning by value naturally creates a copy.
     */
    Coord clone() const;

    /**
     * Checks if this coordinate's location is equal to other coordinate's
     */
    bool equals(const Coord& c) const;

    bool operator==(const Coord& o) const;

    /**
     * Returns a hash code for this coordinate
     */
    std::size_t hashCode() const;

    /**
     * Compares this coordinate to other coordinate. 
     * @return -1, 0 or 1
     */
    int compareTo(const Coord& other) const;

    bool operator<(const Coord& other) const;

    /**
     * Checks whether two coordinates are close enough to be considered close.
     */
    static bool areClose(const Coord& c1, const Coord& c2, double range);
};

} // namespace core

// Spesialisasi std::hash to makeCoord bisa dipakai di unordered_map/unordered_set
namespace std {
    template <>
    struct hash<core::Coord> {
        std::size_t operator()(const core::Coord& c) const {
            return c.hashCode();
        }
    };
}