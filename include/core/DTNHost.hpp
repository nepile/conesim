/**
 * @file DTNHost.hpp
 * @brief Header for the DTNHost class in ONE Simulator C++ port.
 * @author Neville / Team
 * @date September 2026
 * 
 * Copyright 2010 Aalto University, ComNet
 * Released under GPLv3. See LICENSE.txt for details. 
 */

#pragma once

#include <string>
#include <vector>
#include <list>
#include <map>
#include <set>
#include <memory>

#include "core/Coord.hpp"
#include "routing/community/Duration.hpp" 

// ============================================================================
// Correct Namespace Forward Declarations
// ============================================================================
namespace movement {
    class MovementModel;
    class Path;
}

namespace routing {
    class MessageRouter;
    class RoutingInfo;
}

namespace core {
    class MessageListener;
    class MovementListener;
    class NetworkInterface;
    class ModuleCommunicationBus;
    class Connection;
    class Message;

    /**
     * @class DTNHost
     * @brief A DTN capable host.
     * 
     * This class represents nodes like agents, cars, or humans that can move
     * and carry messages in the network.
     */
    class DTNHost {
    private:
        static int nextAddress;
        int address;

        Coord location;     /**< Where is the host */
        Coord destination;  /**< Where is it going */

        // Ownership: Host has the router, movement, and its path 
        std::shared_ptr<routing::MessageRouter> router;
        std::shared_ptr<movement::MovementModel> movement;
        std::shared_ptr<movement::Path> path;
        
        double speed;
        double nextTimeToMove;
        std::string name;
        std::vector<int> color;

        // Observer pattern: Host doesn't own the listener, it just saves raw pointers
        std::vector<MessageListener*> msgListeners;
        std::vector<MovementListener*> movListeners;
        
        // Host has interfaces
        std::vector<std::shared_ptr<NetworkInterface>> net;
        
        // Host connects to bus, but does not own it
        ModuleCommunicationBus* comBus;

        static int getNextAddress();
        void setRouter(std::shared_ptr<routing::MessageRouter> router);
        bool setNextWaypoint();

    public:
        // ====================================================================
        // ADDITIONAL TESTING (Special variables for Machine Learning / RL / Custom)
        // ====================================================================
        std::list<routing::community::Duration> intervals;
        std::vector<double> congestionRatio;
        std::vector<double> dataInContact;
        std::vector<double> ema;
        std::vector<double> dummyForReward;

        std::map<routing::community::Duration, int> dataReceivedInDuration;
        std::map<routing::community::Duration, int> dataTransferredInDuration;
        int msgReceived;
        int msgTransferred;
        std::set<DTNHost*> setofHosts;
        std::map<DTNHost*, routing::community::Duration> durPerNode;
        std::map<DTNHost*, std::vector<routing::community::Duration>> listDurPerNode;
        double totalContactTime;

        // ====================================================================
        // CONSTRUCTOR & DESTRUCTOR
        // ====================================================================
        
        DTNHost(const std::vector<MessageListener*>& msgLs,
                const std::vector<MovementListener*>& movLs,
                const std::string& groupId, 
                const std::vector<std::shared_ptr<NetworkInterface>>& interf,
                ModuleCommunicationBus* comBus,
                std::shared_ptr<movement::MovementModel> mmProto, 
                std::shared_ptr<routing::MessageRouter> mRouterProto);

        ~DTNHost() = default;

        // ====================================================================
        // CORE METHODS
        // ====================================================================

        static void reset();
        bool isActive() const;
        
        std::shared_ptr<routing::MessageRouter> getRouter() const;
        int getAddress() const;
        void setAddress(int address);
        ModuleCommunicationBus* getComBus() const;

        void connectionUp(Connection* con);
        void connectionDown(Connection* con);
        std::vector<Connection*> getConnections() const;

        Coord getLocation() const;
        std::shared_ptr<movement::Path> getPath() const;
        
        void setLocation(const Coord& location);
        void setName(const std::string& name);
        void setColor(const std::vector<int>& color);

        std::vector<std::shared_ptr<Message>> getMessageCollection() const;
        int getNrofMessages() const;
        double getBufferOccupancy() const;
        std::shared_ptr<routing::RoutingInfo> getRoutingInfo() const;

        std::vector<std::shared_ptr<NetworkInterface>> getInterfaces() const;

    protected:
        std::shared_ptr<NetworkInterface> getInterface(int interfaceNo) const;
        std::shared_ptr<NetworkInterface> getInterface(const std::string& interfacetype) const;

    public:
        void forceConnection(DTNHost* anotherHost, const std::string& interfaceId, bool up);

        /**
         * @deprecated Use forceConnection instead.
         */
        void connect(DTNHost* h);

        void update(bool simulateConnections);
        void move(double timeIncrement);

        void sendMessage(const std::string& id, DTNHost* to);
        int receiveMessage(std::shared_ptr<Message> m, DTNHost* from);
        bool requestDeliverableMessages(Connection* con);
        void messageTransferred(const std::string& id, DTNHost* from);
        void messageAborted(const std::string& id, DTNHost* from, int bytesRemaining);
        void createNewMessage(std::shared_ptr<Message> m);
        void deleteMessage(const std::string& id, bool drop);

        std::string toString() const;
        std::vector<int> getColor() const;

        // ====================================================================
        // COMPARISON & EQUALITY
        // ====================================================================

        bool equals(const DTNHost* otherHost) const;
        int compareTo(const DTNHost* h) const;
        bool operator<(const DTNHost& other) const;

        // ====================================================================
        // ADDITIONAL METHODS
        // ====================================================================

        void addDuration(const routing::community::Duration& dur);
        std::string getNodeIntervals() const;
    };

} // namespace core