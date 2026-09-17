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

// Forward declare core classes inside the core namespace
namespace core {
    class DTNHost;
    class Message;
    class Connection;
    class MessageListener;
    class RoutingInfo;
}

class Settings;
class Application;

namespace routing
{

  /**
   * @class MessageRouter
   * @brief Base abstract class for all message routers.
   */
  class MessageRouter
  {
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
    static constexpr int RCV_OK = 0;
    static constexpr int TRY_LATER_BUSY = 1;
    static constexpr int DENIED_OLD = -1;
    static constexpr int DENIED_NO_SPACE = -2;
    static constexpr int DENIED_TTL = -3;
    static constexpr int DENIED_DELIVERED = -4;
    static constexpr int DENIED_UNSPECIFIED = -999;
    ///@}

    /** @name Settings Constants
     * String keys used for reading configurations from the Settings file.
     * Declared as 'inline' (C++17/20) for safe, direct header initialization.
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
    explicit MessageRouter(Settings &s);

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

    virtual bool createNewMessage(std::shared_ptr<core::Message> m);
    virtual void deleteMessage(const std::string &id, bool drop);
    virtual void messageAborted(const std::string &id, core::DTNHost *from, int bytesRemaining);
    virtual void sendMessage(const std::string &id, core::DTNHost *to);
    virtual bool requestDeliverableMessages(core::Connection *con);

    int getBufferSize() const;
    int getFreeBufferSize() const;
    int getNrofMessages() const;
    std::vector<std::shared_ptr<core::Message>> getMessageCollection() const;
    core::DTNHost *getHost() const;
    RoutingInfo getRoutingInfo() const;
    std::string toString() const;

    void addApplication(std::shared_ptr<Application> app);
    std::vector<std::shared_ptr<Application>> getApplications(const std::string &ID) const;

  protected:
    std::vector<std::shared_ptr<core::MessageListener>> mListeners;
    std::unordered_map<std::string, std::shared_ptr<core::Message>> deliveredMessages;
    int msgTtl;

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
     * @tparam T The data type in the vector (e.g., std::shared_ptr<Message> or std::pair).
     * @param list The vector to be sorted or shuffled.
     */
    template <typename T>
    void sortByQueueMode(std::vector<T> &list)
    {
      if (sendQueueMode == Q_MODE_RANDOM)
      {
        // Assuming SimClock (SimulationClock) has its own RNG, or use std::random
        std::mt19937 rng(12345); // Replace seed with SimClock::getIntTime()
        std::shuffle(list.begin(), list.end(), rng);
      }
      else if (sendQueueMode == Q_MODE_FIFO)
      {
        // Implementation details using lambdas will be handled in the .cpp context
      }
    }

    int compareByQueueMode(const Message &m1, const Message &m2) const;

  private:
    std::unordered_map<std::string, std::shared_ptr<Message>> incomingMessages;
    std::unordered_map<std::string, std::shared_ptr<Message>> messages;
    DTNHost *host;
    int bufferSize;
    int sendQueueMode;
    std::unordered_map<std::string, std::vector<std::shared_ptr<Application>>> applications;
    int nodeSelfish;
    std::vector<int> nodeList;
  };

} // namespace routing