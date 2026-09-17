/**
 * @file Report.hpp
 * @brief Abstract superclass for all reports.
 * @details Adapted from The ONE simulator's Report.java
 * @author Opeteer
 * @date September, 2026
 */

#pragma once

#include <string>
#include <vector>
#include <set>
#include <fstream>
#include <memory>

#include "core/Configuration.hpp"

namespace report {

class Report {
public:
    static const std::string REPORT_NS;
    static const std::string INTERVAL_SETTING;
    static const std::string OUTPUT_SETTING;
    static const std::string PRECISION_SETTING;
    static const int DEF_PRECISION;
    static const std::string REPORTDIR_SETTING;
    static const std::string WARMUP_S;
    static const std::string OUT_SUFFIX;
    static const std::string INTERVALLED_FORMAT;
    static const std::string NAN_STRING;

protected:
    std::shared_ptr<std::ofstream> out;
    std::string prefix;
    int precision;
    double warmupTime;
    std::set<std::string> warmupIDs;

    int lastOutputSuffix;
    double outputInterval;
    double lastReportTime;
    std::string outFileName;
    std::string scenarioName;

    /**
     * @brief Constructor.
     * @param className The name of the report class (used for settings and default filename).
     */
    Report(const std::string& className);

    /**
     * @brief Checks that a directory for a file exists or creates it.
     */
    void checkDirExistence(const std::string& outFileName);

    /**
     * @brief Initializes the report output.
     */
    virtual void init();

    /**
     * @brief Creates a new output file.
     */
    void createOutput(const std::string& outFileName);

    /**
     * @brief Creates a number-suffixed output file.
     */
    void createSuffixedOutput(const std::string& outFileName);

    /**
     * @brief Should be called before every new event the report logs.
     */
    virtual void newEvent();

    /**
     * @brief Writes a line to report using defined prefix and out writer.
     */
    virtual void write(const std::string& txt);

    /**
     * @brief Formats a double value according to precision setting.
     */
    std::string format(double value);

    /**
     * @brief Sets a prefix that will be inserted before every line.
     */
    void setPrefix(const std::string& txt);

    /**
     * @brief Returns the name of the scenario.
     */
    std::string getScenarioName() const;

    /**
     * @brief Returns the current simulation time.
     */
    double getSimTime() const;

    /**
     * @brief Returns true if the warm up period is still ongoing.
     */
    bool isWarmup() const;

    /**
     * @brief Adds a new ID to the warm up ID set.
     */
    void addWarmupID(const std::string& id);

    /**
     * @brief Removes a warm up ID from the warm up ID set.
     */
    void removeWarmupID(const std::string& id);

    /**
     * @brief Returns true if the given ID is in the warm up ID set.
     */
    bool isWarmupID(const std::string& id) const;

    /**
     * @brief Returns a Configuration object initialized for the report class.
     */
    core::Configuration getSettings(const std::string& className);

public:
    virtual ~Report();

    /**
     * @brief Called when the simulation is done or next report is needed.
     */
    virtual void done();

    /**
     * @brief Required by UpdateListener or event framework in Java
     */
    virtual void update();

    // Utility methods
    std::string getAverage(const std::vector<double>& values);
    std::string getIntAverage(const std::vector<int>& values);
    std::string getMedian(std::vector<double> values); // By value to allow sorting
    int getIntMedian(std::vector<int> values);         // By value to allow sorting
    std::string getVariance(const std::vector<double>& values);
};

} // namespace report
