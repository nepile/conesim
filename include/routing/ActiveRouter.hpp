/**
 * @file ActiveRouter.hpp
 * @brief Header definition of the ActiveRouter base class.
 * @details Superclass for all active routers (e.g., Epidemic, Prophet).
 *          It provides convenience methods for buffer management, TTL checking,
 *          and actively watching/initiating sending connections during updates.
 * @author Frathol
 * @date September, 2026
 */

#pragma once

#include "routing/MessageRouter.hpp"
#include <vector>
#include <string>
#include <memory>
#include <utility>

namespace core {
    class DTNHost;
    class Message;
    class Connection;
    class Configuration;
    class MessageListener;
}

using DTNHost = core::DTNHost;
using Message = core::Message;
using Connection = core::Connection;
using Settings = core::Configuration;
using MessageListener = core::MessageListener;

namespace routing
{

  /**
   * @class ActiveRouter
   * @brief Abstract base class for routers that actively initiate transfers.
   */
  class ActiveRouter : public MessageRouter
  {
  public:
    /** @name Settings Constants */
    ///@{
    /** Setting ID for deleting delivered messages. Boolean value. */
    inline static const std::string DELETE_DELIVERED_S = "deleteDelivered";

    /** Prefix attached to all response message IDs. */
    inline static const std::string RESPONSE_PREFIX = "R_";
    ///@}

    /** How often TTL check (discarding old messages) is performed (in simulated seconds). */
    static constexpr int TTL_CHECK_INTERVAL = 60;

    /**
     * @brief Constructor to initialize the active router.
     * @param s Reference to the Settings object.
     */
    explicit ActiveRouter(Settings &s);

    /**
     * @brief Copy constructor.
     * @param r The prototype router to copy settings from.
     */
    ActiveRouter(const ActiveRouter &r);

    virtual ~ActiveRouter() = default;

    /**
     * @brief Initializes the router and its sending connections list.
     * @param host The host this router belongs to.
     * @param mListeners Message listeners.
     */
    void init(DTNHost *host, const std::vector<std::shared_ptr<MessageListener>> &mListeners) override;

    /**
     * @brief Called when a connection's state changes.
     * @details Subclasses may override this; the default implementation does nothing.
     * @param con The connection that changed.
     */
    void changedConnection(Connection *con) override;

    /**
     * @brief Performs periodic checks (e.g., finalizing ready transfers, TTL checks).
     */
    void update() override;

    bool requestDeliverableMessages(Connection *con) override;
    bool createNewMessage(std::shared_ptr<Message> m) override;
    int receiveMessage(Message *m, DTNHost *from) override;
    Message *messageTransferred(const std::string &id, DTNHost *from) override;

    /**
     * @brief Checks if this router is currently transferring data.
     * @return True if transferring or some transfer hasn't been finalized.
     */
    virtual bool isTransferring() const;

    /**
     * @brief Checks if the router is currently sending a specific message.
     * @param msgId The ID of the message.
     * @return True if the message is being sent.
     */
    bool isSending(const std::string &msgId) const;

  protected:
    /** Should messages that final recipient marks as delivered be deleted from the buffer? */
    bool deleteDelivered;

    /** Connections that are currently being used for sending data. */
    std::vector<Connection *> sendingConnections;

    /** Simulation time when the last TTL check was performed. */
    double lastTtlCheck;

    /**
     * @brief Gets a list of connections this host currently has.
     * @return Vector of active connections.
     */
    std::vector<Connection *> getConnections() const;

    /**
     * @brief Tries to start a transfer of a message using a connection.
     * @param m The message to transfer.
     * @param con The connection to use.
     * @return The status code of the transfer attempt.
     */
    virtual int startTransfer(std::shared_ptr<Message> m, Connection *con);

    /**
     * @brief Rudimentary check to see if the router can start a transfer.
     * @return True if it has messages and connections.
     */
    virtual bool canStartTransfer() const;

    /**
     * @brief Checks if the router is ready and willing to receive a message.
     * @param m The message to check.
     * @return A status code (e.g., RCV_OK, DENIED_NO_SPACE).
     */
    virtual int checkReceiving(const Message &m);

    /**
     * @brief Removes oldest messages to make room for a new message.
     * @param size Size of the incoming message.
     * @return True if enough space could be freed.
     */
    virtual bool makeRoomForMessage(int size);

    /**
     * @brief Drops messages whose TTL is zero or less.
     */
    virtual void dropExpiredMessages();

    /**
     * @brief Tries to make room for a new message (wrapper around makeRoomForMessage).
     * @param size Size of the new message.
     */
    virtual void makeRoomForNewMessage(int size);

    /**
     * @brief Gets the oldest message in the buffer based on receive time.
     * @param excludeMsgBeingSent If true, ignores messages currently being transferred.
     * @return Pointer to the oldest message, or nullptr if none found.
     */
    virtual Message *getOldestMessage(bool excludeMsgBeingSent);

    /**
     * @brief Finds messages destined for directly connected hosts.
     * @return A vector of pairs, matching deliverable messages to their respective connections.
     */
    std::vector<std::pair<Message *, Connection *>> getMessagesForConnected();

    /**
     * @brief Tries to send messages to connections based on a provided tuple list.
     * @param tuples The list of message-connection pairs to try.
     * @return The pair that successfully started a transfer, or {nullptr, nullptr}.
     */
    std::pair<Message *, Connection *> tryMessagesForConnected(
        const std::vector<std::pair<Message *, Connection *>> &tuples);

    /**
     * @brief Tries to send a list of messages sequentially over a single connection.
     * @param con The connection to use.
     * @param messages The list of messages.
     * @return The message that started transferring, or nullptr.
     */
    Message *tryAllMessages(Connection *con, const std::vector<std::shared_ptr<Message>> &messages);

    /**
     * @brief Tries to send a list of messages to a list of connections.
     * @param messages The messages to try.
     * @param connections The connections to try.
     * @return The connection that accepted a transfer, or nullptr.
     */
    Connection *tryMessagesToConnections(const std::vector<std::shared_ptr<Message>> &messages,
                                         const std::vector<Connection *> &connections);

    /**
     * @brief Tries all buffered messages against all active connections.
     * @return The connection that started a transfer, or nullptr.
     */
    Connection *tryAllMessagesToAllConnections();

    /**
     * @brief Exchanges deliverable messages with connected neighbors.
     * @return A connection that started a transfer, or nullptr.
     */
    Connection *exchangeDeliverableMessages();

    /**
     * @brief Shuffles the order of messages in a list.
     * @param messages The vector of messages to shuffle.
     */
    void shuffleMessages(std::vector<std::shared_ptr<Message>> &messages);

    /**
     * @brief Adds a connection to the active monitoring list.
     * @param con The connection to add.
     */
    void addToSendingConnections(Connection *con);

    /**
     * @brief Hook called just before a transfer is aborted (e.g., connection lost).
     * @param con The connection whose transfer aborted.
     */
    virtual void transferAborted(Connection *con);

    /**
     * @brief Hook called just before a transfer is finalized successfully.
     * @param con The connection whose transfer finished.
     */
    virtual void transferDone(Connection *con);
  };

} // namespace routing