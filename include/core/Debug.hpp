/**
 * @file Debug.hpp
 * @brief Utility for formatted debugging output with simulation timestamps and execution timing.
 * @author Neville
 * @date September
 */

#pragma once

#include <iostream>
#include <string>

namespace core {

/**
 * @class Debug
 * @brief Static utility class for conditional debug printing and performance profiling.
 * 
 * Ported and adapted from Aalto University ComNet's The ONE simulator (core.Debug).
 * Provides stream redirection, level-based filtering, simulation clock timestamping,
 * and execution time benchmarking.
 */
class Debug {
private:
    static std::ostream* out;        ///< Target output stream (default: &std::cout)
    static int debugLevel;           ///< Active filter threshold level
    static long long timingStart;    ///< Epoch millisecond start timestamp (-1 if idle)
    static std::string timingCause;  ///< Label describing the action being measured

public:
    /**
     * @brief Sets the global debugging threshold level.
     * @details Messages submitted with a level strictly lower than this threshold will be discarded.
     * @param level New debug verbosity level.
     */
    static void setDebugLevel(int level);

    /**
     * @brief Redirects debugging output to a designated output stream.
     * @param outStrm Reference to the target output stream (e.g., std::cout, std::ofstream, std::ostringstream).
     */
    static void setPrintStream(std::ostream& outStrm);

    /**
     * @brief Prints a standard debug message at default level 0 without timestamping.
     * @param txt The text message to output.
     */
    static void p(const std::string& txt);

    /**
     * @brief Prints a debug message at a designated severity level without timestamping.
     * @param txt The text message to output.
     * @param level Severity level of this specific message.
     */
    static void p(const std::string& txt, int level);

    /**
     * @brief Prints a fully configured debug message to the configured output stream.
     * @param txt The text message to output.
     * @param level Severity level of this message.
     * @param timestamp If true, prepends the current simulation clock value in brackets [@time].
     */
    static void p(const std::string& txt, int level, bool timestamp);

    /**
     * @brief Prints a debug message with a timestamp at a designated level.
     * @param txt The text message to output.
     * @param level Severity level of this specific message.
     */
    static void pt(const std::string& txt, int level);

    /**
     * @brief Prints a debug message with a timestamp at default level 0.
     * @param txt The text message to output.
     */
    static void pt(const std::string& txt);

    /**
     * @brief Begins measuring the elapsed real time of an operation.
     * @details If a previous timing session was not closed, doneTiming() is triggered automatically.
     * @param cause Description or name of the operation being measured.
     * @see doneTiming()
     */
    static void startTiming(const std::string& cause);

    /**
     * @brief Finalizes an active timing benchmark and prints the elapsed duration.
     * @details Duration is computed in fractional seconds and printed via pt() at level 0.
     * @see startTiming(const std::string&)
     */
    static void doneTiming();
};

} // namespace core