/**
 * @file DTNSim.cpp
 * @brief Implementation of DTNSim
 * @author Opeteer
 * @date September, 2026
 */

#include "core/DTNSim.hpp"
#include "core/Configuration.hpp"
#include "core/ConfigurationError.hpp"

#include <iostream>
#include <iomanip>
#include <chrono>
#include <stdexcept>
#include <sstream>
#include <algorithm>

namespace core {

const std::string DTNSim::BATCH_MODE_FLAG = "-b";
const std::string DTNSim::RANGE_DELIMETER = ":";
const std::string DTNSim::SETTING_DEF_FLAG = "-d";
const std::string DTNSim::CMD_SETTING_DELIMITER = "@@";

std::vector<DTNSim::ResetFunction> DTNSim::resetList;

void DTNSim::main(int argc, char* argv[]) {
    bool batchMode = false;
    std::pair<int, int> nrofRuns = {0, 1};
    std::vector<std::string> args(argv + 1, argv + argc);
    int firstConfIndex = 0;
    int guiIndex = 0;
    std::string cmdSettings = "";
    
    // std::locale::global(std::locale("en_US.UTF-8")); // Usually skipped unless string parsing really requires it

    if (!args.empty()) {
        bool haveRunIndex = false;
        while (firstConfIndex < static_cast<int>(args.size())) {
            if (args[firstConfIndex] == BATCH_MODE_FLAG) {
                batchMode = true;
                if (firstConfIndex + 1 < static_cast<int>(args.size())) {
                    nrofRuns = parseNrofRuns(args[firstConfIndex + 1]);
                }
                firstConfIndex += 2;
                haveRunIndex = true;
            } else if (args[firstConfIndex] == SETTING_DEF_FLAG) {
                if (firstConfIndex + 1 < static_cast<int>(args.size())) {
                    cmdSettings = args[firstConfIndex + 1];
                }
                firstConfIndex += 2;
            } else if (!haveRunIndex) {
                try {
                    guiIndex = std::stoi(args[firstConfIndex]);
                    firstConfIndex++;
                    haveRunIndex = true;
                } catch (const std::invalid_argument&) {
                    std::cerr << "Error parsing command args. Expected run index. Got: " << args[firstConfIndex] << '\n';
                    std::exit(-1);
                }
            } else {
                break;
            }
        }
    }

    initSettings(args, firstConfIndex);

    if (!cmdSettings.empty()) {
        parseCmdSettings(cmdSettings);
        cmdSettings.clear();
    }

    if (batchMode) {
        auto startTime = std::chrono::steady_clock::now();
        for (int i = nrofRuns.first; i < nrofRuns.second; ++i) {
            print("Run " + std::to_string(i + 1) + "/" + std::to_string(nrofRuns.second));
            Configuration::setRunIndex(i);
            resetForNextRun();
            
            // TODO: Start DTNSimTextUI when implemented
            // new DTNSimTextUI().start();
            print("[Mock] DTNSimTextUI started for run " + std::to_string(i));
        }
        auto endTime = std::chrono::steady_clock::now();
        std::chrono::duration<double> duration = endTime - startTime;
        
        std::ostringstream out;
        out << std::fixed << std::setprecision(2) << duration.count();
        print("---\nAll done in " + out.str() + "s");
    } else {
        Configuration::setRunIndex(guiIndex);
        
        // TODO: Start DTNSimGUI when implemented
        // new DTNSimGUI().start();
        print("[Mock] DTNSimGUI started for run " + std::to_string(guiIndex));
    }
}

void DTNSim::initSettings(const std::vector<std::string>& args, int firstIndex) {
    if (firstIndex >= static_cast<int>(args.size())) {
        return;
    }

    try {
        Configuration::init(args[firstIndex]);
        for (int i = firstIndex + 1; i < static_cast<int>(args.size()); i++) {
            Configuration::addSettings(args[i]);
        }
    } catch (const ConfigurationError& er) {
        try {
            std::stoi(args[firstIndex]);
        } catch (const std::invalid_argument&) {
            std::cerr << "Failed to load settings: " << er.what() << '\n';
            std::exit(-1);
        }
        std::cerr << "Warning: using deprecated way of expressing run indexes. "
                  << "Run index should be the first option, or right after -b option.\n";
        std::exit(-1);
    }
}

void DTNSim::registerForReset(ResetFunction resetFunc) {
    resetList.push_back(resetFunc);
}

void DTNSim::resetForNextRun() {
    for (auto resetFunc : resetList) {
        try {
            resetFunc();
        } catch (const std::exception& e) {
            std::cerr << "Failed to reset a class: " << e.what() << '\n';
            std::exit(-1);
        }
    }
}

std::pair<int, int> DTNSim::parseNrofRuns(const std::string& arg) {
    std::pair<int, int> val = {0, 1};
    try {
        size_t delimPos = arg.find(RANGE_DELIMETER);
        if (delimPos != std::string::npos) {
            val.first = std::stoi(arg.substr(0, delimPos)) - 1;
            val.second = std::stoi(arg.substr(delimPos + 1));
        } else {
            val.first = 0;
            val.second = std::stoi(arg);
        }
    } catch (const std::invalid_argument&) {
        std::cerr << "Invalid argument '" << arg << "' for number of runs\n";
        std::exit(-1);
    }

    if (val.first < 0) {
        std::cerr << "Starting run value can't be smaller than 1\n";
        std::exit(-1);
    }
    if (val.first >= val.second) {
        std::cerr << "Starting run value can't be bigger than the last run value\n";
        std::exit(-1);
    }

    return val;
}

void DTNSim::parseCmdSettings(const std::string& arg) {
    std::vector<std::string> set;
    
    size_t pos = 0;
    size_t lastPos = 0;
    while ((pos = arg.find(CMD_SETTING_DELIMITER, lastPos)) != std::string::npos) {
        set.push_back(arg.substr(lastPos, pos - lastPos));
        lastPos = pos + CMD_SETTING_DELIMITER.length();
    }
    set.push_back(arg.substr(lastPos));

    for (const auto& setting : set) {
        size_t eqPos = setting.find("=");
        if (eqPos == std::string::npos) {
            std::cerr << "Improperly formatted command line Setting: " << setting << '\n';
            std::exit(-1);
        }
        
        std::string key = setting.substr(0, eqPos);
        std::string value = setting.substr(eqPos + 1);
        
        // Trim whitespaces
        auto trim = [](std::string& s) {
            s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
                return !std::isspace(ch);
            }));
            s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
                return !std::isspace(ch);
            }).base(), s.end());
        };
        trim(key);
        trim(value);
        
        Configuration::addSetting(key, value);
    }
}

void DTNSim::print(const std::string& txt) {
    std::cout << txt << '\n';
}

} // namespace core
