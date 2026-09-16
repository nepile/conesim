/**
 * @file Coord.cpp
 * @brief Implementation of core::Coord (see Coord.hpp).
 * @author Agra
 * @date September
 */

#include "core/Coord.hpp"
#include <cmath>
#include <cstdio>

namespace core {

Coord::Coord(double x, double y) {
    setLocation(x, y);
}

void Coord::setLocation(double x, double y) {
    this->x = x;
    this->y = y;
}

void Coord::setLocation(const Coord& c) {
    this->x = c.x;
    this->y = c.y;
}

void Coord::translate(double dx, double dy) {
    this->x += dx;
    this->y += dy;
}

double Coord::distance(const Coord& other) const {
    double dx = this->x - other.x;
    double dy = this->y - other.y;
    
    return std::sqrt(dx * dx + dy * dy);
}

double Coord::getX() const {
    return this->x;
}

double Coord::getY() const {
    return this->y;
}

std::string Coord::toString() const {
    char buffer[64];
    std::snprintf(buffer, sizeof(buffer), "(%.2f,%.2f)", x, y);
    return std::string(buffer);
}

Coord Coord::clone() const {
    return Coord(*this);
}

bool Coord::equals(const Coord& c) const {
    if (&c == this) {
        return true;
    }
    return (this->x == c.x && this->y == c.y);
}

bool Coord::operator==(const Coord& o) const {
    return equals(o);
}

std::size_t Coord::hashCode() const {
    std::string hashStr = std::to_string(x) + "," + std::to_string(y);
    return std::hash<std::string>{}(hashStr);
}

int Coord::compareTo(const Coord& other) const {
    if (this->y < other.y) {
        return -1;
    }
    else if (this->y > other.y) {
        return 1;
    }
    else if (this->x < other.x) {
        return -1;
    }
    else if (this->x > other.x) {
        return 1;
    }
    else {
        return 0;
    }
}

bool Coord::operator<(const Coord& other) const {
    return compareTo(other) < 0;
}

bool Coord::areClose(const Coord& c1, const Coord& c2, double range) {
    return std::abs(c1.getX() - c2.getX()) < range && std::abs(c1.getY() - c2.getY()) < range;
}

} // namespace core