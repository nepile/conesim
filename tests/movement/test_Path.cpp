#include <gtest/gtest.h>
#include "movement/Path.hpp"
#include "core/Coord.hpp"

using namespace movement;
using namespace core;

TEST(PathTest, DefaultConstructor) {
    Path p;
    EXPECT_FALSE(p.hasNext());
    EXPECT_TRUE(p.getCoords().empty());
}

TEST(PathTest, ConstantSpeedConstructor) {
    Path p(15.5);
    EXPECT_EQ(p.getSpeeds().size(), 1);
    EXPECT_EQ(p.getSpeeds()[0], 15.5);
}

TEST(PathTest, CopyConstructor) {
    Path original(10.0);
    original.addWaypoint(Coord(1.0, 2.0));
    original.addWaypoint(Coord(3.0, 4.0));
    original.getNextWaypoint();

    Path copy(original);

    EXPECT_EQ(copy.getCoords().size(), 2);
    EXPECT_EQ(copy.getSpeeds().size(), 1);
    
    EXPECT_TRUE(copy.hasNext());
    EXPECT_EQ(copy.getNextWaypoint(), Coord(3.0, 4.0));
}

TEST(PathTest, AddWaypointConstantSpeed) {
    Path p(10.0);
    p.addWaypoint(Coord(0, 0));
    p.addWaypoint(Coord(5, 5));

    EXPECT_EQ(p.getCoords().size(), 2);
    
    EXPECT_TRUE(p.hasNext());
    EXPECT_EQ(p.getFirstWaypoint(), Coord(0, 0));

    Coord wp1 = p.getNextWaypoint();
    EXPECT_EQ(wp1, Coord(0, 0));
    EXPECT_EQ(p.getSpeed(), 10.0);

    Coord wp2 = p.getNextWaypoint();
    EXPECT_EQ(wp2, Coord(5, 5));
    EXPECT_EQ(p.getLastWaypoint(), Coord(5, 5));

    EXPECT_FALSE(p.hasNext());
}

TEST(PathTest, AddWaypointVariableSpeed) {
    Path p;
    p.addWaypoint(Coord(1, 1), 5.0);
    p.addWaypoint(Coord(2, 2), 12.0);
    p.addWaypoint(Coord(3, 3), 8.5);

    EXPECT_EQ(p.getCoords().size(), 3);
    EXPECT_EQ(p.getSpeeds().size(), 3);

    p.getNextWaypoint();
    EXPECT_EQ(p.getSpeed(), 5.0);

    p.getNextWaypoint();
    EXPECT_EQ(p.getSpeed(), 12.0);

    p.getNextWaypoint();
    EXPECT_EQ(p.getSpeed(), 8.5);
}

TEST(PathTest, SetSpeedOverwritesPreviousSpeeds) {
    Path p;
    p.addWaypoint(Coord(1, 1), 5.0);
    p.addWaypoint(Coord(2, 2), 10.0);

    p.setSpeed(20.0);

    EXPECT_EQ(p.getSpeeds().size(), 1);
    EXPECT_EQ(p.getSpeeds()[0], 20.0);
}

#ifndef NDEBUG
TEST(PathTest, AssertFailsOnEmptyWaypointAsk) {
    Path p(10.0);
    EXPECT_DEATH(p.getLastWaypoint(), "No waypoint asked");
}
#endif