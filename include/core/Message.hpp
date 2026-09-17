/**
 * @file Message.hpp
 * @brief Header for the Message class in ONE Simulator C++ port.
 * @author Neville Jeremy Onorato Laia
 * 
 * Copyright 2010 Aalto University, ComNet
 * Released under GPLv3. See LICENSE.txt for details. 
 */

#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <any>
#include "SimulationError.hpp"

namespace core {

    // Forward declaration to overcome circular dependency
    class DTNHost;

    /**
     * @class Message
     * @brief A message that is created at a node or passed between nodes.
     * 
     * Represents a discrete chunk of data transferred across the network.
     */
    class Message : public std::enable_shared_from_this<Message> {
    public:
        /** @brief Value for infinite TTL of message */
        static constexpr int INFINITE_TTL = -1;

    private:
        /** @brief Who the message is (originally) from */
        DTNHost* from;
        
        /** @brief Who the message is (originally) to */
        DTNHost* to;
        
        /** @brief Identifier of the message */
        std::string id;
        
        /** @brief Size of the message (bytes) */
        int size;
        
        /** @brief List of nodes this message has passed */
        std::vector<DTNHost*> path; 
        
        /** @brief Next unique identifier to be given */
        static int nextUniqueId;
        
        /** @brief Unique ID of this message */
        int uniqueId;
        
        /** @brief The time this message was received */
        double timeReceived;
        
        /** @brief The time when this message was created */
        double timeCreated;
        
        /** @brief Initial TTL of the message */
        int initTtl;
        
        /** 
         * @brief if a response to this message is required, this is the size of the 
         * response message (or 0 if no response is requested) 
         */
        int responseSize;
        
        /** @brief if this message is a response message, this is set to the request msg */
        std::shared_ptr<Message> requestMsg;
        
        /** 
         * @brief Container for generic message properties. 
         * @details Using std::any (C++17) as an equivalent to Java's Object.
         */
        std::map<std::string, std::any> properties;
        
        /** @brief Application ID of the application that created the message */
        std::string appID;

    protected:
        /**
         * @brief Deep copies message data from other message.
         * 
         * @param m The message where the data is copied from
         */
        void copyFrom(const Message& m);

    public:
        /**
         * @brief Creates a new Message.
         * 
         * @param from Who the message is (originally) from
         * @param to Who the message is (originally) to
         * @param id Message identifier (must be unique for message but
         *  will be the same for all replicates of the message)
         * @param size Size of the message (in bytes)
         */
        Message(DTNHost* from, DTNHost* to, const std::string& id, int size);

        /** @brief Default virtual destructor */
        virtual ~Message() = default;

        /**
         * @brief Returns the node this message is originally from
         */
        DTNHost* getFrom() const;

        /**
         * @brief Returns the node this message is originally to
         */
        DTNHost* getTo() const;

        /**
         * @brief Returns the ID of the message
         */
        std::string getId() const;
        
        /**
         * @brief Returns an ID that is unique per message instance 
         */
        int getUniqueId() const;
        
        /**
         * @brief Returns the size of the message (in bytes)
         */
        int getSize() const;

        /**
         * @brief Adds a new node on the list of nodes this message has passed
         * @param node The node to add
         */
        void addNodeOnPath(DTNHost* node);
        
        /**
         * @brief Returns a list of nodes this message has passed so far
         */
        std::vector<DTNHost*> getHops() const;
        
        /**
         * @brief Returns the amount of hops this message has passed
         */
        int getHopCount() const;
        
        /** 
         * @brief Returns the time to live (minutes) of the message or INT_MAX
         * if the TTL is infinite.
         */
        int getTtl() const;
        
        /**
         * @brief Sets the initial TTL (time-to-live) for this message.
         * @param ttl The time-to-live to set
         */
        void setTtl(int ttl);
        
        /**
         * @brief Sets the time when this message was received.
         * @param time The time to set
         */
        void setReceiveTime(double time);
        
        /**
         * @brief Returns the time when this message was received
         */
        double getReceiveTime() const;
        
        /**
         * @brief Returns the time when this message was created
         */
        double getCreationTime() const;
        
        /**
         * @brief If this message is a response to a request, sets the request message
         * @param request The request message
         */
        void setRequest(std::shared_ptr<Message> request);
        
        /**
         * @brief Returns the message this message is response to or null if not a response
         */
        std::shared_ptr<Message> getRequest() const;
        
        /**
         * @brief Returns true if this message is a response message
         */
        bool isResponse() const;
        
        /**
         * @brief Sets the requested response message's size.
         * @param size Size of the response message
         */
        void setResponseSize(int size);
        
        /**
         * @brief Returns the size of the requested response message or 0
         */
        int getResponseSize() const;
        
        /**
         * @brief Returns a string representation of the message
         */
        std::string toString() const;

        /**
         * @brief Adds a generic property for this message.
         * 
         * @param key The key which is used to lookup the value
         * @param value The value to store (std::any handles multiple types)
         * @throws std::runtime_error (SimError equivalent) if key already exists
         */
        void addProperty(const std::string& key, const std::any& value);
        
        /**
         * @brief Returns an object that was stored to this message using the given key.
         * 
         * @param key The key used to lookup the object
         * @return The stored object or empty std::any if it isn't found
         */
        std::any getProperty(const std::string& key) const;
        
        /**
         * @brief Updates a value for an existing property.
         * 
         * @param key The key which is used to lookup the value
         * @param value The new value to store
         */
        void updateProperty(const std::string& key, const std::any& value);
        
        /**
         * @brief Returns a replicate of this message.
         * @return A replicate of the message
         */
        std::shared_ptr<Message> replicate() const;
        
        /**
         * @brief Compares two messages by their ID (alphabetically).
         * @details Replaces Java's compareTo().
         */
        bool operator<(const Message& m) const;
        
        /**
         * @brief Resets all static fields to default values
         */
        static void reset();

        /**
         * @brief Gets the appID
         */
        std::string getAppID() const;

        /**
         * @brief Sets the appID
         * @param appID the appID to set
         */
        void setAppID(const std::string& appID);
    };

} // namespace core