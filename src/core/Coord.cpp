/**
 * @file Coord.cpp
 * @brief Implementation of conesim::Coord (see Coord.hpp).
 * @author Agra
 * @date September
 */

#include "Coord.hpp"

#include <cmath>
#include <iomanip>
#include <sstream>

namespace conesim {

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
    std::ostringstream oss;
    oss << "(" << std::fixed << std::setprecision(2) << x
        << "," << std::fixed << std::setprecision(2) << y << ")";
    return oss.str();
}

Coord Coord::clone() const {
    return Coord(*this);
}

bool Coord::equals(const Coord& c) const {
    if (&c == this) {
        return true;
    }
    return (x == c.x && y == c.y);
}

bool Coord::operator==(const Coord& other) const {
    return equals(other);
}

bool Coord::operator!=(const Coord& other) const {
    return !equals(other);
}

int Coord::compareTo(const Coord& other) const {
    if (this->y < other.y) {
        return -1;
    } else if (this->y > other.y) {
        return 1;
    } else if (this->x < other.x) {
        return -1;
    } else if (this->x > other.x) {
        return 1;
    } else {
        return 0;
    }
}

bool Coord::operator<(const Coord& other) const {
    return compareTo(other) < 0;
}

bool Coord::areClose(const Coord& c1, const Coord& c2, double range) {
    return std::abs(c1.getX() - c2.getX()) < range &&
           std::abs(c1.getY() - c2.getY()) < range;
}

} // namespace conesim

namespace std {
    std::size_t hash<conesim::Coord>::operator()(const conesim::Coord& c) const noexcept {
        std::ostringstream oss;
        oss << c.getX() << "," << c.getY();
        return std::hash<std::string>()(oss.str());
    }
}