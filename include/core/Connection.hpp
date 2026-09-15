/**
 * @file Connection.hpp
 * @brief Header for the Connection class in ONE Simulator C++ port.
 * @author Neville
 * 
 * Copyright 2010 Aalto University, ComNet
 * Released under GPLv3. See LICENSE.txt for details. 
 */

#pragma once

#include <memory>
#include <string>

namespace conesim {
    class DTNHost;
    class NetworkInterface;
    class Message;
    
    /**
     * @class Connection
     * @brief A connection between two DTN nodes.
     * 
     * Abstract base class that represents a physical or logical connection 
     * between two network interfaces. It handles message transfers, connection
     * states, and tracks the amount of data transferred.
     */
    class Connection {
    protected:
        /** @brief The node in the other side of the connection. */
        DTNHost* toNode;
        /** @brief The interface in the other side of the connection. */
        NetworkInterface* toInterface;
        /** @brief The node that initiated the connection. */
        DTNHost* fromNode;
        /** @brief The interface that initiated the connection. */
        NetworkInterface* fromInterface;
        /** @brief The node from which the current message originates. */
        DTNHost* msgFromNode;

        /** @brief The state of the connection (true if up, false if down). */
        bool _isUp;
        
        /** @brief The message that this connection is currently transferring. */
        std::shared_ptr<Message> msgOnFly;
        
        /** @brief Total number of bytes this connection has transferred. */
        int bytesTransferred;

        /**
         * @brief Clears the message that is currently being transferred.
         * @details Calls to getMessage() will return an empty shared_ptr after this.
         */
        virtual void clearMsgOnFly();

    public:
        /**
         * @brief Creates a new connection between nodes and sets the connection state to "up".
         * 
         * @param fromNode The node that initiated the connection.
         * @param fromInterface The interface that initiated the connection.
         * @param toNode The node in the other side of the connection.
         * @param toInterface The interface in the other side of the connection.
         */
        Connection(DTNHost* fromNode, NetworkInterface* fromInterface, 
                   DTNHost* toNode, NetworkInterface* toInterface);

        /**
         * @brief Virtual destructor to ensure proper cleanup of derived classes.
         */
        virtual ~Connection() = default;

        /**
         * @brief Returns true if the connection is up.
         * 
         * @return State of the connection.
         */
        bool isUp() const;

        /**
         * @brief Returns true if the given node is the initiator of the connection, false otherwise.
         * 
         * @param node The node to check.
         * @return True if the given node is the initiator of the connection.
         */
        bool isInitiator(DTNHost* node) const;

        /**
         * @brief Sets the state of the connection.
         * 
         * @param state True if the connection is up, false if not.
         */
        void setUpState(bool state);

        /**
         * @brief Sets a message that this connection is currently transferring.
         * @details If message passing is controlled by external events, this method is not needed
         * (but then e.g. finalizeTransfer() and isMessageTransferred() will not work either). 
         * Only one message at a time can be transferred using one connection.
         * 
         * @param from The node initiating the transfer.
         * @param m The message to transfer.
         * @return The value returned by MessageRouter::receiveMessage().
         */
        virtual int startTransfer(DTNHost* from, std::shared_ptr<Message> m) = 0;

        /**
         * @brief Calculate the current transmission speed from the information
         * given by the interfaces, and calculate the missing data amount.
         */
        virtual void update();

        /**
         * @brief Aborts the transfer of the currently transferred message.
         */
        virtual void abortTransfer();

        /**
         * @brief Returns the amount of bytes to be transferred before ongoing transfer is ready.
         * 
         * @return The amount of bytes to be transferred, or 0 if there's no ongoing transfer 
         * or it has finished already.
         */
        virtual int getRemainingByteCount() const = 0;

        /**
         * @brief Finalizes the transfer of the currently transferred message.
         * @details The message that was being transferred cannot be retrieved from this 
         * connection after calling this method (using getMessage()).
         */
        virtual void finalizeTransfer();

        /**
         * @brief Returns true if the current message transfer is done.
         * 
         * @return True if the transfer is done, false if not.
         */
        virtual bool isMessageTransferred() const = 0;

        /**
         * @brief Returns true if the connection is ready to transfer a message.
         * @details Connection must be up and there must be no message currently being transferred.
         * 
         * @return True if the connection is ready to transfer a message.
         */
        bool isReadyForTransfer() const;

        /**
         * @brief Gets the message that this connection is currently transferring.
         * 
         * @return The message, or an empty shared_ptr if no message is being transferred.
         */ 
        std::shared_ptr<Message> getMessage() const;

        /** 
         * @brief Gets the current connection speed.
         * 
         * @return The transmission speed in Bps.
         */
        virtual double getSpeed() const = 0;  

        /**
         * @brief Returns the total amount of bytes this connection has transferred so far.
         * @details Includes all previous transfers and the current ongoing transfer.
         * 
         * @return Total bytes transferred.
         */
        int getTotalBytesTransferred() const;

        /**
         * @brief Returns the node in the other end of the connection.
         * 
         * @param node The node in this end of the connection.
         * @return The requested node.
         */
        DTNHost* getOtherNode(DTNHost* node) const;

        /**
         * @brief Returns the interface in the other end of the connection.
         * 
         * @param i The interface in this end of the connection.
         * @return The requested interface.
         */
        NetworkInterface* getOtherInterface(NetworkInterface* i) const;

        /**
         * @brief Returns a String presentation of the connection.
         * 
         * @return Formatted string describing the connection state.
         */
        virtual std::string toString() const;
    };
} // namespace core