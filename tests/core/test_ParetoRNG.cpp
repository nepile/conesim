/**
 * @file test_ParetoRNG.cpp
 * @brief Unit tests for ParetoRNG generator.
 * @author Neville
 * @date September 2026
 */

#include <gtest/gtest.h>
#include <random>
#include <limits>
#include <cmath>

#include "core/ParetoRNG.hpp"

using namespace core;

class ParetoRNGTest : public ::testing::Test {
protected:
    const unsigned int DEFAULT_SEED = 1337;
    std::mt19937 rng;

    void SetUp() override {
        rng.seed(DEFAULT_SEED);
    }
};

TEST_F(ParetoRNGTest, ReturnsInfinityWhenXmIsMinusOne) {
    double k = 1.5;
    double minValue = -1.0;
    double maxValue = 100.0;

    ParetoRNG pareto(rng, k, minValue, maxValue);
    double val = pareto.getDouble();

    EXPECT_TRUE(std::isinf(val));
    EXPECT_GT(val, 0.0);
}

TEST_F(ParetoRNGTest, AllowsValuesBeyondStandardMaxWhenMaxValueIsMinusOne) {
    double k = 1.2;
    double minValue = 10.0;
    double maxValue = -1.0;

    ParetoRNG pareto(rng, k, minValue, maxValue);

    for (int i = 0; i < 1000; ++i) {
        double val = pareto.getDouble();
        EXPECT_FALSE(std::isnan(val));
        EXPECT_GE(val, minValue);
    }
}

TEST_F(ParetoRNGTest, RespectsMinAndMaxBounds) {
    double k = 2.0;
    double minValue = 5.0;
    double maxValue = 50.0;

    ParetoRNG pareto(rng, k, minValue, maxValue);

    for (int i = 0; i < 2000; ++i) {
        double val = pareto.getDouble();
        EXPECT_GE(val, minValue);
        EXPECT_LE(val, maxValue);
    }
}

TEST_F(ParetoRNGTest, AdvancesExternalEngineStateByReference) {
    double k = 1.5;
    double minValue = 2.0;
    double maxValue = 20.0;

    std::mt19937 controlRng(DEFAULT_SEED);

    ParetoRNG pareto(rng, k, minValue, maxValue);
    pareto.getDouble();

    EXPECT_NE(rng, controlRng);
}

TEST_F(ParetoRNGTest, ProducesDeterministicSequenceForSameSeed) {
    double k = 1.8;
    double minValue = 10.0;
    double maxValue = 100.0;

    std::mt19937 rngA(42);
    std::mt19937 rngB(42);

    ParetoRNG paretoA(rngA, k, minValue, maxValue);
    ParetoRNG paretoB(rngB, k, minValue, maxValue);

    for (int i = 0; i < 100; ++i) {
        EXPECT_DOUBLE_EQ(paretoA.getDouble(), paretoB.getDouble());
    }
}