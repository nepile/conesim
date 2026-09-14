/**
 * @file ParetoRNG.hpp
 * @brief Random number generator adhering to a Pareto distribution.
 * @author Neville
 * @date September 2026
 */

#pragma once

#include <random>

namespace conesim {

/**
 * @class ParetoRNG
 * @brief Generates pseudo-random double values based on a Pareto (power-law) distribution.
 * 
 * Ported from Aalto University ComNet's The ONE simulator (core.ParetoRNG).
 * Uses inverse transform sampling on a uniform real distribution driven by an external
 * Mersenne Twister engine.
 */
class ParetoRNG {
private:
    std::mt19937& rng;                           ///< Reference to external PRNG engine
    std::uniform_real_distribution<double> dist; ///< Uniform real distribution in range [0.0, 1.0)
    double xm;                                   ///< Minimum value parameter (scale, Xm)
    double k;                                    ///< Shape parameter (tail index/alpha)
    double maxValue;                             ///< Upper bound truncation threshold

public:
    /**
     * @brief Constructs a new Pareto random number generator.
     * @param rng Reference to the base Mersenne Twister generator engine.
     * @param k The shape parameter (alpha/index) of the Pareto distribution.
     * @param minValue The scale parameter (Xm) representing the minimum possible value. If set to -1, getDouble() yields infinity.
     * @param maxValue The maximum allowable value (truncation bound). Pass -1 for positive infinity.
     */
    ParetoRNG(std::mt19937& rng, double k, double minValue, double maxValue);

    /**
     * @brief Generates and returns a Pareto-distributed double precision value.
     * @return A Pareto-distributed value bounded by [xm, maxValue], or infinity if xm == -1.
     */
    double getDouble();
};

} // namespace conesim