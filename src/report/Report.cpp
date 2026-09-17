/**
 * @file Report.cpp
 * @brief Implementation of the Report abstract base class.
 * @author Opeteer
 * @date September, 2026
 */

#include "report/Report.hpp"
#include "core/SimulationClock.hpp"
#include "core/SimulationScenario.hpp"
#include "core/SimulationError.hpp"
#include "core/ConfigurationError.hpp"

#include <filesystem>
#include <iomanip>
#include <sstream>
#include <algorithm>
#include <numeric>
#include <iostream>

namespace report {

const std::string Report::REPORT_NS = "Report";
const std::string Report::INTERVAL_SETTING = "interval";
const std::string Report::OUTPUT_SETTING = "output";
const std::string Report::PRECISION_SETTING = "precision";
const int Report::DEF_PRECISION = 4;
const std::string Report::REPORTDIR_SETTING = "Report.reportDir";
const std::string Report::WARMUP_S = "warmup";
const std::string Report::OUT_SUFFIX = ".csv";
const std::string Report::INTERVALLED_FORMAT = "%04d"; // Note: .csv will be appended
const std::string Report::NAN_STRING = "NaN";

Report::Report(const std::string& className)
    : prefix(""), lastOutputSuffix(0), outputInterval(-1.0), lastReportTime(0.0) {
    
    core::Configuration settings("");
    
    try {
        scenarioName = settings.valueFillString(
            settings.getConfiguration(core::SimulationScenario::SCENARIO_NS + "." + core::SimulationScenario::NAME_S)
        );
    } catch (const core::ConfigurationError&) {
        scenarioName = "default_scenario";
    }

    core::Configuration classSettings = getSettings(className);

    try {
        outputInterval = classSettings.getDouble(INTERVAL_SETTING);
    } catch (const core::ConfigurationError&) {
        outputInterval = -1.0;
    }

    try {
        warmupTime = classSettings.getDouble(WARMUP_S);
    } catch (const core::ConfigurationError&) {
        warmupTime = 0.0;
    }

    try {
        precision = classSettings.getInt(PRECISION_SETTING);
        if (precision < 0) {
            precision = 0;
        }
    } catch (const core::ConfigurationError&) {
        precision = DEF_PRECISION;
    }

    try {
        outFileName = classSettings.getConfiguration(OUTPUT_SETTING);
        outFileName = classSettings.valueFillString(outFileName);
    } catch (const core::ConfigurationError&) {
        // no output name defined -> construct one from report class' name
        std::string outDir;
        try {
            outDir = settings.getConfiguration(REPORTDIR_SETTING);
        } catch (const core::ConfigurationError&) {
            outDir = "reports/"; // default fallback
        }
        
        if (!outDir.empty() && outDir.back() != '/' && outDir.back() != '\\') {
            outDir += "/";
        }
        outFileName = outDir + scenarioName + "_" + className;
        if (outputInterval == -1.0) {
            outFileName += OUT_SUFFIX;
        }
    }

    checkDirExistence(outFileName);
}

Report::~Report() {
    done();
}

void Report::checkDirExistence(const std::string& outFileName) {
    std::filesystem::path outFile(outFileName);
    std::filesystem::path outDir = outFile.parent_path();

    if (!outDir.empty() && !std::filesystem::exists(outDir)) {
        std::error_code ec;
        if (!std::filesystem::create_directories(outDir, ec)) {
            throw core::SimulationError("Couldn't create report directory '" + outDir.string() + "': " + ec.message());
        }
    }
}

void Report::init() {
    lastReportTime = getSimTime();

    if (outputInterval > 0) {
        createSuffixedOutput(outFileName);
    } else {
        createOutput(outFileName);
    }
}

void Report::createOutput(const std::string& outFileName) {
    out = std::make_shared<std::ofstream>(outFileName);
    if (!out->is_open()) {
        throw core::SimulationError("Couldn't open file '" + outFileName + "' for report output");
    }
}

void Report::createSuffixedOutput(const std::string& outFileName) {
    std::ostringstream ss;
    ss << std::setfill('0') << std::setw(4) << lastOutputSuffix;
    std::string suffix = ss.str() + OUT_SUFFIX;
    
    createOutput(outFileName + suffix);
    lastOutputSuffix++;
}

void Report::newEvent() {
    if (outputInterval <= 0) {
        return;
    }

    if (getSimTime() > lastReportTime + outputInterval) {
        done(); // finalize the old file
        init(); // init the new file
    }
}

void Report::write(const std::string& txt) {
    if (!out || !out->is_open()) {
        init();
    }
    (*out) << prefix << txt << "\n";
}

std::string Report::format(double value) {
    std::ostringstream ss;
    ss << std::fixed << std::setprecision(precision) << value;
    return ss.str();
}

void Report::setPrefix(const std::string& txt) {
    prefix = txt;
}

std::string Report::getScenarioName() const {
    return scenarioName;
}

double Report::getSimTime() const {
    return core::SimulationClock::getTime();
}

bool Report::isWarmup() const {
    return warmupTime > core::SimulationClock::getTime();
}

void Report::addWarmupID(const std::string& id) {
    warmupIDs.insert(id);
}

void Report::removeWarmupID(const std::string& id) {
    warmupIDs.erase(id);
}

bool Report::isWarmupID(const std::string& id) const {
    if (warmupIDs.empty()) {
        return false;
    }
    return warmupIDs.find(id) != warmupIDs.end();
}

core::Configuration Report::getSettings(const std::string& className) {
    core::Configuration s(className);
    s.setSecondaryNamespace(REPORT_NS);
    return s;
}

void Report::done() {
    if (out && out->is_open()) {
        out->close();
    }
}

void Report::update() {
    throw std::runtime_error("Unimplemented method 'update'");
}

std::string Report::getAverage(const std::vector<double>& values) {
    if (values.empty()) {
        return NAN_STRING;
    }
    double sum = std::accumulate(values.begin(), values.end(), 0.0);
    return format(sum / values.size());
}

std::string Report::getIntAverage(const std::vector<int>& values) {
    if (values.empty()) {
        return NAN_STRING;
    }
    std::vector<double> dValues(values.begin(), values.end());
    return getAverage(dValues);
}

std::string Report::getMedian(std::vector<double> values) {
    if (values.empty()) {
        return NAN_STRING;
    }
    std::sort(values.begin(), values.end());
    return format(values[values.size() / 2]);
}

int Report::getIntMedian(std::vector<int> values) {
    if (values.empty()) {
        return 0;
    }
    std::sort(values.begin(), values.end());
    return values[values.size() / 2];
}

std::string Report::getVariance(const std::vector<double>& values) {
    if (values.empty()) {
        return NAN_STRING;
    }
    double sum = 0.0;
    double sum2 = 0.0;
    for (double v : values) {
        sum += v;
        sum2 += (v * v);
    }
    double e_x = sum / values.size();
    return format(sum2 / values.size() - (e_x * e_x));
}

} // namespace report
