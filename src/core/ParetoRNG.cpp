/**
 * @file ParetoRNG.cpp
 * @brief Implementation of the Pareto distribution generator.
 * @author Neville
 * @date September 2026
 */

#include "core/ParetoRNG.hpp"

#include <cmath>
#include <limits>
#include <random>

namespace conesim {

ParetoRNG::ParetoRNG(std::mt19937& rng, double k, double minValue, double maxValue)
    : rng(rng),
      dist(0.0, 1.0),
      xm(minValue),
      k(k),
      maxValue(maxValue == -1.0 ? std::numeric_limits<double>::infinity() : maxValue) {}

double ParetoRNG::getDouble() {
    if (xm == -1.0) {
        return std::numeric_limits<double>::infinity();
    }

    double x;
    do {
        x = xm * std::pow(1.0 - dist(rng), -1.0 / k);
    } while (x > maxValue);

    return x;
}

} // namespace conesim