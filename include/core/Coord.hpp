/**
 * @file Coord.hpp
 * @brief Definition of conesim::Coord (2D coordinate representation).
 * @author Agra
 * @date September
 */

#pragma once

#include <string>

namespace conesim
{

    class Coord
    {
    private:
        double x;
        double y;

    public:
        Coord(double x, double y);

        void setLocation(double x, double y);

        void setLocation(const Coord &c);

        void translate(double dx, double dy);

        double distance(const Coord &other) const;

        double getX() const;

        double getY() const;

        std::string toString() const;

        Coord clone() const;

        bool equals(const Coord &c) const;

        bool operator==(const Coord &other) const;

        bool operator!=(const Coord &other) const;

        int compareTo(const Coord &other) const;

        bool operator<(const Coord &other) const;

        static bool areClose(const Coord &c1, const Coord &c2, double range);
    };

} // namespace conesim

namespace std
{
    template <>
    struct hash<conesim::Coord>
    {
        std::size_t operator()(const conesim::Coord &c) const noexcept;
    };
}