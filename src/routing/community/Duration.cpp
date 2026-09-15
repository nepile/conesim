/**
 * @file Duration.cpp
 * @brief Implementation of the Duration class member functions.
 * @author Neville
 * @date September, 2026
 */

#include "routing/community/Duration.hpp"

#include <string>

namespace conesim::routing::community {

    Duration::Duration(double start, double end) 
        : start(start), end(end) {} 

    double Duration::getStart() const {
        return start;
    }

    double Duration::getEnd() const {
        return end;
    }

    std::string Duration::getDuration() const {
        return "Start: " + std::to_string(start) + ", End: " + std::to_string(end);
    }
} // namespace conesim::routing::community
