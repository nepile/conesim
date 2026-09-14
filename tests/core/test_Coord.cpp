#include <gtest/gtest.h>
#include <cmath>
#include <sstream>
#include <unordered_set>

#include "core/Coord.hpp"

using namespace conesim;

class CoordTest : public ::testing::Test {
protected:
    void SetUp() override {
      //future setup code ya bang
    }
};

TEST_F(CoordTest, ConstructorSetsInitialLocation) {
    Coord c(3.0, 4.0);
    EXPECT_DOUBLE_EQ(c.getX(), 3.0);
    EXPECT_DOUBLE_EQ(c.getY(), 4.0);
}

TEST_F(CoordTest, SetLocationWithXY) {
    Coord c(0.0, 0.0);
    c.setLocation(5.5, -2.25);
    EXPECT_DOUBLE_EQ(c.getX(), 5.5);
    EXPECT_DOUBLE_EQ(c.getY(), -2.25);
}

TEST_F(CoordTest, SetLocationFromOtherCoord) {
    Coord a(1.0, 1.0);
    Coord b(9.0, -3.0);
    a.setLocation(b);
    EXPECT_DOUBLE_EQ(a.getX(), 9.0);
    EXPECT_DOUBLE_EQ(a.getY(), -3.0);
}

TEST_F(CoordTest, TranslateMovesPointByOffset) {
    Coord c(2.0, 2.0);
    c.translate(1.5, -0.5);
    EXPECT_DOUBLE_EQ(c.getX(), 3.5);
    EXPECT_DOUBLE_EQ(c.getY(), 1.5);
}

TEST_F(CoordTest, DistanceBetweenTwoCoordinates) {
    Coord a(0.0, 0.0);
    Coord b(3.0, 4.0);
    EXPECT_DOUBLE_EQ(a.distance(b), 5.0); 
}

TEST_F(CoordTest, DistanceToSelfIsZero) {
    Coord a(7.0, -2.0);
    EXPECT_DOUBLE_EQ(a.distance(a), 0.0);
}

TEST_F(CoordTest, ToStringFormatsWithTwoDecimals) {
    Coord c(1.5, -2.333);
    EXPECT_EQ(c.toString(), "(1.50,-2.33)");
}

TEST_F(CoordTest, CloneProducesEqualButIndependentCopy) {
    Coord original(6.0, 8.0);
    Coord copy = original.clone();

    EXPECT_TRUE(copy.equals(original));

    copy.translate(1.0, 1.0);
    EXPECT_FALSE(copy.equals(original)); 
}

TEST_F(CoordTest, EqualsReturnsTrueForSameLocation) {
    Coord a(1.0, 2.0);
    Coord b(1.0, 2.0);
    EXPECT_TRUE(a.equals(b));
    EXPECT_TRUE(a == b);
}

TEST_F(CoordTest, EqualsReturnsFalseForDifferentLocation) {
    Coord a(1.0, 2.0);
    Coord b(1.0, 2.1);
    EXPECT_FALSE(a.equals(b));
    EXPECT_TRUE(a != b);
}

TEST_F(CoordTest, EqualsIsTrueForSameInstance) {
    Coord a(1.0, 2.0);
    EXPECT_TRUE(a.equals(a));
}

TEST_F(CoordTest, CompareToOrdersBySmallerYFirst) {
    Coord lower(5.0, 1.0);
    Coord higher(0.0, 2.0);
    EXPECT_LT(lower.compareTo(higher), 0);
    EXPECT_GT(higher.compareTo(lower), 0);
}

TEST_F(CoordTest, CompareToOrdersBySmallerXWhenYEqual) {
    Coord left(1.0, 3.0);
    Coord right(4.0, 3.0);
    EXPECT_LT(left.compareTo(right), 0);
    EXPECT_GT(right.compareTo(left), 0);
}

TEST_F(CoordTest, CompareToReturnsZeroForEqualCoordinates) {
    Coord a(2.0, 2.0);
    Coord b(2.0, 2.0);
    EXPECT_EQ(a.compareTo(b), 0);
}

TEST_F(CoordTest, LessThanOperatorMatchesCompareTo) {
    Coord a(1.0, 3.0);
    Coord b(4.0, 3.0);
    EXPECT_TRUE(a < b);
    EXPECT_FALSE(b < a);
}

TEST_F(CoordTest, AreCloseReturnsTrueWithinRange) {
    Coord a(0.0, 0.0);
    Coord b(0.5, 0.5);
    EXPECT_TRUE(Coord::areClose(a, b, 1.0));
}

TEST_F(CoordTest, AreCloseReturnsFalseOutsideRange) {
    Coord a(0.0, 0.0);
    Coord b(2.0, 2.0);
    EXPECT_FALSE(Coord::areClose(a, b, 1.0));
}

TEST_F(CoordTest, HashAllowsUseInUnorderedSet) {
    std::unordered_set<Coord> coords;
    coords.insert(Coord(1.0, 1.0));
    coords.insert(Coord(1.0, 1.0)); 
    coords.insert(Coord(2.0, 2.0));

    EXPECT_EQ(coords.size(), 2u);
    EXPECT_NE(coords.find(Coord(1.0, 1.0)), coords.end());
}