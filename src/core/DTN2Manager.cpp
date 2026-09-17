/**
 * @file DTN2Manager.cpp
 * @brief Implementation of DTN2Manager
 * @author Opeteer
 * @date September, 2026
 */

#include "core/DTN2Manager.hpp"
#include "core/Configuration.hpp"
#include "core/ConfigurationError.hpp"
#include "core/Debug.hpp"

#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <regex>

namespace core {

std::map<std::shared_ptr<DTNHost>, std::shared_ptr<ecla::CLAParser>> DTN2Manager::CLAs;
std::vector<std::shared_ptr<DTN2Manager::EIDHost>> DTN2Manager::EID_to_host;
std::map<std::string, std::shared_ptr<ecla::Bundle>> DTN2Manager::bundles;

std::shared_ptr<report::DTN2Reporter> DTN2Manager::reporter = nullptr;
std::shared_ptr<input::DTN2Events> DTN2Manager::events = nullptr;

void DTN2Manager::setup(std::shared_ptr<World> world) {
    CLAs.clear();
    EID_to_host.clear();
    bundles.clear();

    // Check if DTN2Reporter and DTN2Events have been loaded.
    // If not, we do nothing here.
    if (!reporter || !events) {
        return;
    }

    core::Configuration conf("DTN2");
    std::string fname;
    try {
        fname = conf.getConfiguration("configFile");
    } catch (const core::ConfigurationError&) {
        return;
    }

    if (!std::filesystem::exists(fname)) {
        return;
    }

    std::ifstream in(fname);
    if (!in.is_open()) {
        core::Debug::p("Could not load requested DTN2 configuration file '" + fname + "'");
        return;
    }

    // Create a directory to hold copies of the bundles
    if (!std::filesystem::exists("bundles")) {
        std::filesystem::create_directory("bundles");
    }

    std::string line;
    while (std::getline(in, line)) {
        // Trim leading spaces
        line.erase(0, line.find_first_not_of(" \t\r\n"));
        if (line.empty() || line[0] == '#') {
            continue;
        }

        std::stringstream ss(line);
        std::string token;
        std::vector<std::string> attrs;
        while (ss >> token) {
            attrs.push_back(token);
        }

        if (attrs.size() >= 5) {
            int nodeID = std::stoi(attrs[0]);
            std::string nodeEID = attrs[1];
            std::string dtnd_host = attrs[2];
            int dtnd_port = std::stoi(attrs[3]);
            // int console_port = std::stoi(attrs[4]);

            // Find the host
            std::shared_ptr<DTNHost> h = world->getNodeByAddress(nodeID);

            // Add to the EID -> Host mapping
            auto e = std::make_shared<EIDHost>(nodeEID, nodeID, h);
            EID_to_host.push_back(e);

            // TODO: Configure and start the CLA when 'ecla' module is implemented
            /*
            auto p = std::make_shared<ecla::CLAParser>(dtnd_host, dtnd_port, "ONE");
            auto ph = events->getParserHandler(nodeID, dtnd_host, console_port);
            p->setListener(ph);
            std::thread t([p]() { p->run(); });
            t.detach();
            
            // Save reference to the CLA
            CLAs[h] = p;
            */
        }
    }
}

void DTN2Manager::setReporter(std::shared_ptr<report::DTN2Reporter> rep) {
    reporter = rep;
}

std::shared_ptr<report::DTN2Reporter> DTN2Manager::getReporter() {
    return reporter;
}

void DTN2Manager::setEvents(std::shared_ptr<input::DTN2Events> evts) {
    events = evts;
}

std::shared_ptr<input::DTN2Events> DTN2Manager::getEvents() {
    return events;
}

std::shared_ptr<ecla::CLAParser> DTN2Manager::getParser(std::shared_ptr<DTNHost> host) {
    auto it = CLAs.find(host);
    if (it != CLAs.end()) {
        return it->second;
    }
    return nullptr;
}

std::vector<std::shared_ptr<DTN2Manager::EIDHost>> DTN2Manager::getHosts(const std::string& EID) {
    std::vector<std::shared_ptr<EIDHost>> result;
    try {
        std::regex e(EID);
        for (const auto& host : EID_to_host) {
            if (std::regex_match(host->EID, e)) {
                result.push_back(host);
            }
        }
    } catch (const std::regex_error&) {
        // Fallback if regex is invalid
        for (const auto& host : EID_to_host) {
            if (host->EID == EID) {
                result.push_back(host);
            }
        }
    }
    return result;
}

void DTN2Manager::addBundle(const std::string& id, std::shared_ptr<ecla::Bundle> bundle) {
    bundles[id] = bundle;
}

std::shared_ptr<ecla::Bundle> DTN2Manager::getBundle(const std::string& id) {
    auto it = bundles.find(id);
    if (it != bundles.end()) {
        auto bundle = it->second;
        bundles.erase(it);
        return bundle;
    }
    return nullptr;
}

} // namespace core
