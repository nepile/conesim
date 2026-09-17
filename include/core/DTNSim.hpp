/**
 * @file DTNSim.hpp
 * @brief Simulator's main class
 * @details Adapted from The ONE simulator's DTNSim.java
 * @author Opeteer
 * @date September, 2026
 */

#pragma once

#include <string>
#include <vector>
#include <functional>

namespace core {

class DTNSim {
public:
    static const std::string BATCH_MODE_FLAG;
    static const std::string RANGE_DELIMETER;
    static const std::string SETTING_DEF_FLAG;
    static const std::string CMD_SETTING_DELIMITER;

    /**
     * @brief Type definition for reset functions.
     */
    using ResetFunction = void(*)();

    /**
     * @brief Starts the simulator with the given command line arguments.
     * @param argc Argument count
     * @param argv Argument vector
     */
    static void main(int argc, char* argv[]);

    /**
     * @brief Registers a reset function to be called between batch runs.
     * @param resetFunc The function pointer to register
     */
    static void registerForReset(ResetFunction resetFunc);

    /**
     * @brief Prints text to stdout
     * @param txt Text to print
     */
    static void print(const std::string& txt);

private:
    static std::vector<ResetFunction> resetList;

    /**
     * @brief Initializes Settings (Configuration in C++).
     * @param args Command line arguments
     * @param firstIndex Index of the first config file name
     */
    static void initSettings(const std::vector<std::string>& args, int firstIndex);

    /**
     * @brief Resets all registered components for the next batch run.
     */
    static void resetForNextRun();

    /**
     * @brief Parses the number of runs from a command line argument.
     * @param arg The argument to parse
     * @return Pair containing start run index and end run index
     */
    static std::pair<int, int> parseNrofRuns(const std::string& arg);

    /**
     * @brief Parses configuration override settings passed via command line.
     */
    static void parseCmdSettings(const std::string& arg);
};

} // namespace core
