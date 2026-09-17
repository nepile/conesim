#include <gtest/gtest.h>
#include "report/Report.hpp"
#include "core/Configuration.hpp"
#include "core/SimulationClock.hpp"

#include <fstream>
#include <filesystem>
#include <vector>

using namespace report;
using namespace core;

// A concrete implementation of the abstract Report class for testing
class TestReport : public Report {
public:
    TestReport(const std::string& className) : Report(className) {}

    // Expose protected methods for testing
    void publicInit() { init(); }
    void publicWrite(const std::string& txt) { write(txt); }
    void publicSetPrefix(const std::string& txt) { setPrefix(txt); }
    void publicAddWarmupID(const std::string& id) { addWarmupID(id); }
    bool publicIsWarmupID(const std::string& id) const { return isWarmupID(id); }
    std::string publicFormat(double value) { return format(value); }

    void update() override {
        // Implementation for the virtual method
    }
};

class ReportTest : public ::testing::Test {
protected:
    std::string testConfigFile = "test_report.cfg";
    std::string testReportDir = "test_reports_dir";

    void SetUp() override {
        // Create configuration
        std::ofstream out(testConfigFile);
        out << "Scenario.name = TestScenario\n"
            << "Report.reportDir = " << testReportDir << "\n"
            << "TestReport.precision = 3\n"
            << "TestReport.warmup = 100\n";
        out.close();

        Configuration::init(testConfigFile);
        SimulationClock::reset();
    }

    void TearDown() override {
        SimulationClock::reset();
        std::remove(testConfigFile.c_str());
        
        // Clean up the generated report directory
        if (std::filesystem::exists(testReportDir)) {
            std::filesystem::remove_all(testReportDir);
        }
    }
};

TEST_F(ReportTest, WriteAndOutputTest) {
    TestReport report("TestReport");
    
    report.publicSetPrefix("[PREFIX] ");
    report.publicWrite("Hello World");
    report.done(); // flush and close

    std::string expectedFilename = testReportDir + "/TestScenario_TestReport.csv";
    EXPECT_TRUE(std::filesystem::exists(expectedFilename));

    // Verify content
    std::ifstream in(expectedFilename);
    std::string content;
    std::getline(in, content);
    EXPECT_EQ(content, "[PREFIX] Hello World");
}

TEST_F(ReportTest, WarmupTest) {
    TestReport report("TestReport");
    
    // Check warmup ID storage
    report.publicAddWarmupID("node1");
    EXPECT_TRUE(report.publicIsWarmupID("node1"));
    EXPECT_FALSE(report.publicIsWarmupID("node2"));
}

TEST_F(ReportTest, MathUtilitiesTest) {
    TestReport report("TestReport"); // Uses precision = 3

    // Average
    std::vector<double> doubles = {1.5, 2.5, 3.5};
    EXPECT_EQ(report.getAverage(doubles), "2.500");

    // Int Average
    std::vector<int> ints = {1, 2, 6};
    EXPECT_EQ(report.getIntAverage(ints), "3.000");

    // Median
    std::vector<double> medDoubles = {10.0, 5.0, 20.0};
    EXPECT_EQ(report.getMedian(medDoubles), "10.000");

    // Int Median
    std::vector<int> medInts = {10, 5, 20};
    EXPECT_EQ(report.getIntMedian(medInts), 10);

    // Variance
    std::vector<double> varDoubles = {2.0, 4.0, 4.0, 4.0, 5.0, 5.0, 7.0, 9.0};
    // E(X) = 40/8 = 5
    // Var(X) = E(X^2) - (E(X))^2
    // X^2 = {4, 16, 16, 16, 25, 25, 49, 81} -> sum = 232
    // 232 / 8 = 29
    // Var = 29 - 25 = 4.0
    EXPECT_EQ(report.getVariance(varDoubles), "4.000");
}

TEST_F(ReportTest, FormatPrecisionTest) {
    TestReport report("TestReport"); // From config, precision is 3
    EXPECT_EQ(report.publicFormat(3.14159), "3.142");
    EXPECT_EQ(report.publicFormat(3.1), "3.100");
}
