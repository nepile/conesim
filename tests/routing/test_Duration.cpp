/**
 * @file DurationTest.cpp
 * @brief Unit tests for the Duration class.
 * @author Neville
 * @date September, 2026
 */

#include "routing/community/Duration.hpp"
#include <gtest/gtest.h>

using namespace conesim::routing::community;

/**
 * @brief Test fixture for Duration class testing.
 */
class DurationTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Setup code if needed before each test
    }

    void TearDown() override {
        // Cleanup code if needed after each test
    }
};

/**
 * @test Verifies that the constructor correctly initializes the start and end values.
 */
TEST_F(DurationTest, ConstructorInitializesCorrectly) {
    double expectedStart = 10.5;
    double expectedEnd = 25.3;
    
    Duration duration(expectedStart, expectedEnd);
    
    // Menggunakan EXPECT_DOUBLE_EQ karena membandingkan tipe data floating-point (double)
    EXPECT_DOUBLE_EQ(duration.getStart(), expectedStart);
    EXPECT_DOUBLE_EQ(duration.getEnd(), expectedEnd);
}

/**
 * @test Verifies that getDuration() returns the correctly formatted string.
 */
TEST_F(DurationTest, GetDurationReturnsCorrectFormat) {
    Duration duration(1.0, 5.0);
    
    // std::to_string() secara default menghasilkan 6 angka di belakang koma
    std::string expectedString = "Start: 1.000000, End: 5.000000";
    
    EXPECT_EQ(duration.getDuration(), expectedString);
}

/**
 * @test Verifies behavior with negative and zero values.
 */
TEST_F(DurationTest, HandlesZerosAndNegatives) {
    Duration duration(-5.5, 0.0);
    
    EXPECT_DOUBLE_EQ(duration.getStart(), -5.5);
    EXPECT_DOUBLE_EQ(duration.getEnd(), 0.0);
}
