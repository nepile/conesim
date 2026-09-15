/**
 * @file Duration.hpp
 * @brief Definition of the Duration class for tracking route start and end times.
 * @author Neville
 * @date September, 2026
 */

#pragma once

#include <string>

namespace routing::community {

/**
 * @class Duration
 * @brief Represents a time duration with double precision.
 * 
 * This class stores the start and end timestamps of a route and provides
 * utilities to access them or retrieve a formatted string representation.
 */
class Duration {
private:
    double start; ///< The starting time point.
    double end;   ///< The ending time point.

public:
    /**
     * @brief Constructs a new Duration object.
     * @param start The starting time of the route.
     * @param end The ending time of the route.
     */
    Duration(double start, double end);

    /**
     * @brief Gets the starting time.
     * @return The starting time as a double.
     */
    double getStart() const;

    /**
     * @brief Gets the ending time.
     * @return The ending time as a double.
     */
    double getEnd() const; 

    /**
     * @brief Formats the duration information into a readable string.
     * @return A standard string in the format "Start: [start], End: [end]".
     */
    std::string getDuration() const;
};

} // namespace routing::community
