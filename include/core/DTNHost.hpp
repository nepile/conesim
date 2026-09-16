/**
 * @file DTNHost.hpp
 * @brief Header for the DTNHost class in ONE Simulator C++ port.
 * @author Neville
 * @date September 2026
 * 
 * Copyright 2010 Aalto University, ComNet
 * Released under GPLv3. See LICENSE.txt for details. 
 */

#ifndef DTNHOST_HPP
#define DTNHOST_HPP

#include <string>
#include <vector>
#include <list>
#include <map>
#include <set>
#include <memory>

#include "core/Coord.hpp"
#include "routing/community/Duration.hpp" 

namespace core {
    class MessageRouter;
    class MovementModel;
    class Path;
    class MessageListener;
    class MovementListener;
    class NetworkInterface;
    class ModuleCommunicationBus;
    class Connection;
    class Message;
    class RoutingInfo;

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

        // Ownership: Host have the router, movement, and its path 
        std::shared_ptr<MessageRouter> router;
        std::shared_ptr<MovementModel> movement;
        std::shared_ptr<Path> path;
        
        double speed;
        double nextTimeToMove;
        std::string name;
        std::vector<int> color;

        // Observer pattern: Host don't have the listener, it is just save it (raw pointer)
        std::vector<MessageListener*> msgListeners;
        std::vector<MovementListener*> movListeners;
        
        // host has a interface
        std::vector<std::shared_ptr<NetworkInterface>> net;
        
        // Host just connected to bus, and it hasn't it
        ModuleCommunicationBus* comBus;

        /**
         * @brief Returns a new network interface address and increments it.
         * @return The next address.
         */
        static int getNextAddress();

        /**
         * @brief Set a router for this host
         * @param router The router to set
         */
        void setRouter(std::shared_ptr<MessageRouter> router);

        /**
         * @brief Sets the next destination and speed to correspond the next waypoint on the path.
         * @return True if there was a next waypoint to set, false if node still should wait
         */
        bool setNextWaypoint();

    public:
        // ====================================================================
        // ADDITIONAL TESTING (Special variabel for Machine Learning / RL / Custom)
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
        
        /**
         * @brief Creates a new DTNHost.
         *
         * @param msgLs Message listeners
         * @param movLs Movement listeners
         * @param groupId GroupID of this host
         * @param interf List of NetworkInterfaces for the class
         * @param comBus Module communication bus object
         * @param mmProto Prototype of the movement model of this host
         * @param mRouterProto Prototype of the message router of this host
         */
        DTNHost(const std::vector<MessageListener*>& msgLs,
                const std::vector<MovementListener*>& movLs,
                const std::string& groupId, 
                const std::vector<std::shared_ptr<NetworkInterface>>& interf,
                ModuleCommunicationBus* comBus,
                std::shared_ptr<MovementModel> mmProto, 
                std::shared_ptr<MessageRouter> mRouterProto);

        ~DTNHost() = default;

        // ====================================================================
        // CORE METHODS
        // ====================================================================

        /**
         * @brief Reset the host and its interfaces (Static context)
         */
        static void reset();

        /**
         * @brief Returns true if this node is active (false if not)
         * @return true if this node is active
         */
        bool isActive() const;

        /**
         * @brief Returns the router of this host
         * @return the router of this host
         */
        std::shared_ptr<MessageRouter> getRouter() const;

        /**
         * @brief Returns the network-layer address of this host.
         */
        int getAddress() const;

        void setAddress(int address);

        /**
         * @brief Returns this hosts's ModuleCommunicationBus
         */
        ModuleCommunicationBus* getComBus() const;

        void connectionUp(Connection* con);
        void connectionDown(Connection* con);

        /**
         * @brief Returns a copy of the list of connections this host has with other hosts
         */
        std::vector<Connection*> getConnections() const;

        Coord getLocation() const;
        std::shared_ptr<Path> getPath() const;
        
        void setLocation(const Coord& location);
        void setName(const std::string& name);
        void setColor(const std::vector<int>& color);

        std::vector<std::shared_ptr<Message>> getMessageCollection() const;
        int getNrofMessages() const;
        double getBufferOccupancy() const;
        std::shared_ptr<RoutingInfo> getRoutingInfo() const;

        std::vector<std::shared_ptr<NetworkInterface>> getInterfaces() const;

    protected:
        std::shared_ptr<NetworkInterface> getInterface(int interfaceNo) const;
        std::shared_ptr<NetworkInterface> getInterface(const std::string& interfacetype) const;

    public:
        void forceConnection(DTNHost* anotherHost, const std::string& interfaceId, bool up);

        /**
         * @brief for tests only --- do not use!!!
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

        /**
         * @brief Checks if a host is the same as this host by comparing pointer memory address.
         */
        bool equals(const DTNHost* otherHost) const;

        /**
         * @brief Compares two DTNHosts by their addresses. (Parity with Java's compareTo)
         */
        int compareTo(const DTNHost* h) const;

        /**
         * @brief C++ idiomatic way to implement Comparable<DTNHost>.
         */
        bool operator<(const DTNHost& other) const;

        // ====================================================================
        // ADDITIONAL METHOD
        // ====================================================================

        /**
         * @brief additional method for add Duration to list
         */
        void addDuration(const routing::community::Duration& dur);

        /**
         * @brief additional method for get string for representation of intervals
         */
        std::string getNodeIntervals() const;
    };

} // namespace core

#endif // DTNHOST_HPP