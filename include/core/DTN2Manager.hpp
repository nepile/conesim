/**
 * @file DTN2Manager.hpp
 * @brief Manages the external convergence layer connections to dtnd.
 * @details Parses the configuration file and sets up the CLAParsers and EID->host mappings.
 * @author Opeteer
 * @date September, 2026
 */

#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>

#include "core/DTNHost.hpp"
#include "core/World.hpp"

// Forward declarations for missing external or unconnected modules
namespace ecla {
    class Bundle;
    class CLAParser;
}

namespace report {
    class DTN2Reporter;
}

namespace input {
    class DTN2Events;
}

namespace core {

class DTN2Manager {
public:
    /**
     * @struct EIDHost
     * @brief EID to DTNHost mapping elements.
     */
    struct EIDHost {
        std::string EID;
        int host_id;
        std::shared_ptr<DTNHost> host;
        
        EIDHost(const std::string& eid, int host_id, std::shared_ptr<DTNHost> host)
            : EID(eid), host_id(host_id), host(host) {}
    };

private:
    static std::map<std::shared_ptr<DTNHost>, std::shared_ptr<ecla::CLAParser>> CLAs;
    static std::vector<std::shared_ptr<EIDHost>> EID_to_host;
    static std::map<std::string, std::shared_ptr<ecla::Bundle>> bundles;
    
    static std::shared_ptr<report::DTN2Reporter> reporter;
    static std::shared_ptr<input::DTN2Events> events;

public:
    /**
     * @brief Sets up the dtnd connections by parsing the configuration file.
     * @param world reference to the world that contains the nodes
     */
    static void setup(std::shared_ptr<World> world);

    /**
     * @brief Sets the DTN2Reporter object used to pass messages from ONE to dtnd.
     * @param rep the reporter object to save reference to
     */
    static void setReporter(std::shared_ptr<report::DTN2Reporter> rep);

    /**
     * @brief Returns reference to the DTN2Reporter object.
     * @return reference to the active DTN2Reporter object
     */
    static std::shared_ptr<report::DTN2Reporter> getReporter();

    /**
     * @brief Sets the DTN2Events object.
     * @param evts the active events object to use
     */
    static void setEvents(std::shared_ptr<input::DTN2Events> evts);

    /**
     * @brief Returns the DTN2Events object.
     * @return the currently active events object
     */
    static std::shared_ptr<input::DTN2Events> getEvents();

    /**
     * @brief Returns the ECL parser associated with the host.
     * @param host the host who's parser to return
     * @return the host's parser
     */
    static std::shared_ptr<ecla::CLAParser> getParser(std::shared_ptr<DTNHost> host);

    /**
     * @brief Returns a Collection of DTNHost objects corresponding to the given EID.
     * @param EID EID of the host
     * @return the host corresponding to the EID
     */
    static std::vector<std::shared_ptr<EIDHost>> getHosts(const std::string& EID);

    /**
     * @brief Stores a reference to a bundle corresponding to the given message.
     * @param id the id of the message
     * @param bundle the bundle associated with the message
     */
    static void addBundle(const std::string& id, std::shared_ptr<ecla::Bundle> bundle);

    /**
     * @brief Returns the bundle associated with the given message id.
     * @param id the message id
     * @return the bundle associated with the message
     */
    static std::shared_ptr<ecla::Bundle> getBundle(const std::string& id);
};

} // namespace core
