/**
 * @file MessageRouter.hpp
 * @brief Header definition of the MessageRouter base class.
 * @details Serves as the core abstract superclass for all message routers in the ONE Sim engine.
 *          It manages message queues, buffers, active connections, and the lifecycle of
 *          message transmission and reception between nodes.
 * @author Frathol
 * @date September, 2026
 */

#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include <memory>
#include <climits>
#include <algorithm>
#include <random>
#include "core/Application.hpp"

namespace core {
    class DTNHost;
    class Message;
    class Connection;
    class MessageListener;
    class Configuration;
    // class Application;
}

namespace routing {

    class RoutingInfo;

    /**
     * @class MessageRouter
     * @brief Base abstract class for all message routers.
     * @details Provides the core functionality for buffering, queuing, and 
     *          transferring messages across DTN hosts.
     */
    class MessageRouter {
    public:
        /** @name Queue Modes
         * Message queue ordering modes.
         */
        ///@{
        static constexpr int Q_MODE_RANDOM = 1; ///< Random order (Default)
        static constexpr int Q_MODE_FIFO = 2;   ///< First In First Out
        ///@}
        
        /** @name Receive Return Values
         * Status codes returned when attempting to receive a message.
         */
        ///@{
        static constexpr int RCV_OK = 0;                  ///< Message accepted
        static constexpr int TRY_LATER_BUSY = 1;          ///< Receiver is busy, try again later
        static constexpr int DENIED_OLD = -1;             ///< Message is too old/TTL expired
        static constexpr int DENIED_NO_SPACE = -2;        ///< Not enough buffer space
        static constexpr int DENIED_TTL = -3;             ///< TTL drop
        static constexpr int DENIED_DELIVERED = -4;       ///< Message already delivered to destination
        static constexpr int DENIED_UNSPECIFIED = -999;   ///< Denied for unknown reasons
        ///@}

        /** @name Settings Constants
         * String keys used for reading configurations from the Settings file.
         */
        ///@{
        inline static const std::string B_SIZE_S = "bufferSize";
        inline static const std::string MSG_TTL_S = "msgTtl";
        inline static const std::string SEND_QUEUE_MODE_S = "sendQueue";
        inline static const std::string NODE_RANDOM = "randomNode";
        ///@}

        /**
         * @brief Constructor to initialize the message router.
         * @param s Reference to the Settings object containing configuration values.
         */
        explicit MessageRouter(core::Configuration &s);

        /**
         * @brief Copy constructor.
         * @param r The router prototype from which configuration values will be copied.
         */
        MessageRouter(const MessageRouter &r);

        virtual ~MessageRouter() = default;

        /**
         * @brief Initializes the router and binds it to a specific host.
         * @param host The host owning this router.
         * @param mListeners List of listeners for message-related events.
         */
        virtual void init(core::DTNHost *host, const std::vector<std::shared_ptr<core::MessageListener>> &mListeners);

        /**
         * @brief Called every simulation tick to update the router status.
         */
        virtual void update();

        /**
         * @brief Informs the router about a change in connection state.
         * @param con The connection that changed.
         */
        virtual void changedConnection(core::Connection *con) = 0;

        /**
         * @brief Creates a replica of this router with empty buffers and routing tables.
         * @return Pointer to the new replicated router instance.
         */
        virtual MessageRouter *replicate() = 0;

        /**
         * @brief Tries to start receiving a message from another host.
         * @param m The incoming message.
         * @param from The sending host.
         * @return RCV_OK if accepted, or a DENIED_* / TRY_LATER_BUSY code if rejected.
         */
        virtual int receiveMessage(core::Message *m, core::DTNHost *from);

        /**
         * @brief Called after a message is successfully transferred to confirm delivery.
         * @param id ID of the transferred message.
         * @param from The sending host (previous hop).
         * @return The message that this host received.
         */
        virtual core::Message *messageTransferred(const std::string &id, core::DTNHost *from);
        
        /**
         * @brief Injects a newly created message into the router.
         * @param m Shared pointer to the new message.
         * @return True if successful, false otherwise.
         */
        virtual bool createNewMessage(std::shared_ptr<core::Message> m);

        /**
         * @brief Removes a message from the router's buffer.
         * @param id The ID of the message to delete.
         * @param drop True if dropped due to buffer limits/TTL, false if successfully delivered.
         */
        virtual void deleteMessage(const std::string &id, bool drop);

        /**
         * @brief Informs the router that an ongoing transfer was interrupted.
         * @param id ID of the message.
         * @param from The host that was sending the message.
         * @param bytesRemaining Unsent bytes.
         */
        virtual void messageAborted(const std::string &id, core::DTNHost *from, int bytesRemaining);

        /**
         * @brief Orders the router to initiate sending a message to a target host.
         * @param id The unique string ID of the message.
         * @param to Pointer to the destination DTNHost.
         */
        virtual void sendMessage(const std::string &id, core::DTNHost *to);

        /**
         * @brief Polls the router to push deliverable messages over a connection.
         * @param con Pointer to the connection to evaluate.
         * @return True if the router initiated a transfer, false otherwise.
         */
        virtual bool requestDeliverableMessages(core::Connection *con);

        int getBufferSize() const;
        int getFreeBufferSize() const;
        int getNrofMessages() const;
        std::vector<std::shared_ptr<core::Message>> getMessageCollection() const;
        core::DTNHost *getHost() const;
        RoutingInfo getRoutingInfo() const;
        std::string toString() const;

        void addApplication(std::shared_ptr<core::Application> app);
        std::vector<std::shared_ptr<core::Application>> getApplications(const std::string &ID) const;

    protected:
        std::vector<std::shared_ptr<core::MessageListener>> mListeners; ///< Message event listeners
        std::unordered_map<std::string, std::shared_ptr<core::Message>> deliveredMessages; ///< Tracked delivered messages
        int msgTtl; ///< Global Time-To-Live for messages in minutes

        core::Message *getMessage(const std::string &id);
        bool hasMessage(const std::string &id) const;
        bool isDeliveredMessage(const core::Message &m) const;

        void putToIncomingBuffer(std::shared_ptr<core::Message> m, core::DTNHost *from);
        std::shared_ptr<core::Message> removeFromIncomingBuffer(const std::string &id, core::DTNHost *from);
        bool isIncomingMessage(const std::string &id) const;

        void addToMessages(std::shared_ptr<core::Message> m, bool newMessage);
        std::shared_ptr<core::Message> removeFromMessages(const std::string &id);

        /**
         * @brief Sorts or shuffles the given list according to the current sending queue mode.
         * @tparam T The data type in the vector (e.g., std::shared_ptr<Message>).
         * @param list The vector to be sorted or shuffled.
         */
        template <typename T>
        void sortByQueueMode(std::vector<T> &list) {
            if (sendQueueMode == Q_MODE_RANDOM) {
                // Ideally replace 12345 with a simulation-wide seed like SimClock::getIntTime()
                std::mt19937 rng(12345);
                std::shuffle(list.begin(), list.end(), rng);
            }
        }

        int compareByQueueMode(const core::Message &m1, const core::Message &m2) const;

    private:
        std::unordered_map<std::string, std::shared_ptr<core::Message>> incomingMessages;
        std::unordered_map<std::string, std::shared_ptr<core::Message>> messages;
        core::DTNHost *host;
        int bufferSize;
        int sendQueueMode;
        std::unordered_map<std::string, std::vector<std::shared_ptr<core::Application>>> applications;
        int nodeSelfish;
        std::vector<int> nodeList;
    };

} // namespace routing