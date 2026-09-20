/**
 * @file MessageRouter.cpp
 * @brief Implementation of the MessageRouter base class.
 * @details Core routing logic, buffer management, and message lifecycle handling.
 * @author Frathol
 * @date September 2026
 */

#include "routing/MessageRouter.hpp"
#include "core/DTNHost.hpp"
#include "core/Message.hpp"
#include "core/Connection.hpp"
#include "core/MessageListener.hpp"
#include "core/Configuration.hpp"
#include "core/Application.hpp"

// Define a dummy RoutingInfo class locally if not fully implemented yet
namespace routing
{
  class RoutingInfo
  {
  public:
    RoutingInfo() = default;
  };
}

#include <iostream>
#include <algorithm>

namespace routing
{

  // ============================================================================
  // Constructors & Initialization
  // ============================================================================

  /**
   * @brief Constructs a MessageRouter with configuration from Settings.
   * @param s Reference to the Settings object.
   */
  MessageRouter::MessageRouter(core::Configuration &s)
      : msgTtl(300), // Default TTL 300 minutes, adjust via Settings
        host(nullptr),
        bufferSize(10000000), // Default buffer 10MB, adjust via Settings
        sendQueueMode(Q_MODE_RANDOM),
        nodeSelfish(0)
  {
    // Example of how Settings might be read (adjust according to your API):
    // if (s.contains(B_SIZE_S)) this->bufferSize = s.getInt(B_SIZE_S);
    // if (s.contains(MSG_TTL_S)) this->msgTtl = s.getInt(MSG_TTL_S);
    // if (s.contains(SEND_QUEUE_MODE_S)) this->sendQueueMode = s.getInt(SEND_QUEUE_MODE_S);
  }

  /**
   * @brief Copy constructor for replicating router prototypes.
   * @param r The prototype router.
   */
  MessageRouter::MessageRouter(const MessageRouter &r)
      : msgTtl(r.msgTtl),
        host(nullptr),
        bufferSize(r.bufferSize),
        sendQueueMode(r.sendQueueMode),
        nodeSelfish(r.nodeSelfish)
  {
    // Note: We deliberately do NOT copy messages or listeners during replication.
    // Each host needs an empty buffer and its own unique state at initialization.
  }

  /**
   * @brief Initializes the router and binds it to a host.
   * @param host Pointer to the host that owns this router.
   * @param mListeners List of message listeners to notify.
   */
  void MessageRouter::init(core::DTNHost *host, const std::vector<std::shared_ptr<core::MessageListener>> &mListeners)
  {
    this->host = host;
    this->mListeners = mListeners;
  }

  /**
   * @brief Called every simulation tick. Base implementation updates applications.
   */
  void MessageRouter::update()
  {
    // Implement Application updates if required
    // for (auto& appPair : applications) {
    //     for (auto& app : appPair.second) {
    //         app->update(this->host);
    //     }
    // }
  }

  /**
   * @brief Retrieves the maximum buffer size.
   * @return Size in bytes.
   */
  int MessageRouter::getBufferSize() const
  {
    return this->bufferSize;
  }

  /**
   * @brief Calculates available space in the message buffer.
   * @return Free space in bytes.
   */
  int MessageRouter::getFreeBufferSize() const
  {
    int usedSpace = 0;
    for (const auto &pair : this->messages)
    {
      usedSpace += pair.second->getSize();
    }
    return this->bufferSize - usedSpace;
  }

  /**
   * @brief Gets the total number of messages currently in the buffer.
   * @return Integer count of messages.
   */
  int MessageRouter::getNrofMessages() const
  {
    return static_cast<int>(this->messages.size());
  }

  /**
   * @brief Retrieves all messages currently stored in the buffer.
   * @return A vector of shared pointers to the messages.
   */
  std::vector<std::shared_ptr<core::Message>> MessageRouter::getMessageCollection() const
  {
    std::vector<std::shared_ptr<core::Message>> collection;
    collection.reserve(this->messages.size());
    for (const auto &pair : this->messages)
    {
      collection.push_back(pair.second);
    }
    return collection;
  }

  /**
   * @brief Gets the host that owns this router.
   * @return Pointer to the DTNHost.
   */
  core::DTNHost *MessageRouter::getHost() const
  {
    return this->host;
  }

  /**
   * @brief Returns routing information/metrics (e.g., for reporting tools).
   * @return A RoutingInfo object.
   */
  RoutingInfo MessageRouter::getRoutingInfo() const
  {
    return RoutingInfo(); // Base implementation returns an empty info object
  }

  /**
   * @brief Identifies the router instance as a string.
   * @return String representation.
   */
  std::string MessageRouter::toString() const
  {
    return this->host ? "Router_" + this->host->toString() : "Uninitialized_Router";
  }

  // ============================================================================
  // Message Handling & Lifecycles
  // ============================================================================

  /**
   * @brief Validates and initiates reception of an incoming message.
   * @param m Pointer to the incoming message.
   * @param from The host sending the message.
   * @return A status code (e.g., RCV_OK, DENIED_NO_SPACE).
   */
  int MessageRouter::receiveMessage(core::Message *m, core::DTNHost *from)
  {
    if (!m)
      return DENIED_UNSPECIFIED;

    // Check if we have already delivered this message (prevent loops)
    if (isDeliveredMessage(*m))
    {
      return DENIED_DELIVERED;
    }

    // Check if we already have it in the buffer
    if (hasMessage(m->getId()))
    {
      return DENIED_OLD;
    }

    // Check if we have enough buffer space
    if (getFreeBufferSize() < m->getSize())
    {
      return DENIED_NO_SPACE;
    }

    // Accepted: Put it into the incoming transfer buffer
    // Assuming core::Message inherits std::enable_shared_from_this
    putToIncomingBuffer(m->shared_from_this(), from);

    return RCV_OK;
  }

  /**
   * @brief Confirms the successful transfer of a message.
   * @param id The message ID.
   * @param from The host that sent it.
   * @return Pointer to the received message.
   */
  core::Message *MessageRouter::messageTransferred(const std::string &id, core::DTNHost *from)
  {
    std::shared_ptr<core::Message> m = removeFromIncomingBuffer(id, from);
    if (m)
    {
      addToMessages(m, false);
      return m.get();
    }
    return nullptr;
  }

  /**
   * @brief Injects a brand new message (e.g., created by an application) into the router.
   * @param m Shared pointer to the new message.
   * @return True if successfully buffered, false if no space.
   */
  bool MessageRouter::createNewMessage(std::shared_ptr<core::Message> m)
  {
    if (getFreeBufferSize() < m->getSize())
    {
      return false;
    }
    addToMessages(m, true);
    return true;
  }

  /**
   * @brief Removes a message from the host's buffer entirely.
   * @param id The ID of the message to delete.
   * @param drop True if the message was dropped (TTL expired / buffer full), false if delivered.
   */
  void MessageRouter::deleteMessage(const std::string &id, bool drop)
  {
    std::shared_ptr<core::Message> m = removeFromMessages(id);
    if (m && drop)
    {
      // Notify listeners about the drop
      for (auto &listener : mListeners)
      {
        // listener->messageDeleted(id, this->host, drop); // Example notification
      }
    }
  }

  /**
   * @brief Cancels an ongoing incoming message transfer.
   * @param id The message ID.
   * @param from The sending host.
   * @param bytesRemaining Unsent bytes at the time of abortion.
   */
  void MessageRouter::messageAborted(const std::string &id, core::DTNHost *from, int bytesRemaining)
  {
    removeFromIncomingBuffer(id, from);
  }

  /**
   * @brief Base logic for sending a message. (Usually overridden by child routing algorithms).
   * @param id The message ID.
   * @param to The destination host.
   */
  void MessageRouter::sendMessage(const std::string &id, core::DTNHost *to)
  {
    // Base implementation is intentionally empty.
    // Specific routing algorithms (Epidemic, PRoPHET, etc.) implement this.
  }

  /**
   * @brief Base logic for polling connections. (Usually overridden by child routers).
   * @param con Pointer to the connection.
   * @return True if a transfer was started.
   */
  bool MessageRouter::requestDeliverableMessages(core::Connection *con)
  {
    return false; // Base implementation does not push messages.
  }

  // ============================================================================
  // Internal Buffer Management
  // ============================================================================

  core::Message *MessageRouter::getMessage(const std::string &id)
  {
    auto it = this->messages.find(id);
    return (it != this->messages.end()) ? it->second.get() : nullptr;
  }

  bool MessageRouter::hasMessage(const std::string &id) const
  {
    return this->messages.find(id) != this->messages.end();
  }

  bool MessageRouter::isDeliveredMessage(const core::Message &m) const
  {
    return this->deliveredMessages.find(m.getId()) != this->deliveredMessages.end();
  }

  void MessageRouter::putToIncomingBuffer(std::shared_ptr<core::Message> m, core::DTNHost *from)
  {
    this->incomingMessages[m->getId()] = m;
  }

  std::shared_ptr<core::Message> MessageRouter::removeFromIncomingBuffer(const std::string &id, core::DTNHost *from)
  {
    auto it = this->incomingMessages.find(id);
    if (it != this->incomingMessages.end())
    {
      std::shared_ptr<core::Message> m = it->second;
      this->incomingMessages.erase(it);
      return m;
    }
    return nullptr;
  }

  bool MessageRouter::isIncomingMessage(const std::string &id) const
  {
    return this->incomingMessages.find(id) != this->incomingMessages.end();
  }

  void MessageRouter::addToMessages(std::shared_ptr<core::Message> m, bool newMessage)
  {
    this->messages[m->getId()] = m;

    // If it's reached its final destination, track it
    if (m->getTo() == this->host)
    {
      this->deliveredMessages[m->getId()] = m;
    }
  }

  std::shared_ptr<core::Message> MessageRouter::removeFromMessages(const std::string &id)
  {
    auto it = this->messages.find(id);
    if (it != this->messages.end())
    {
      std::shared_ptr<core::Message> m = it->second;
      this->messages.erase(it);
      return m;
    }
    return nullptr;
  }

  /**
   * @brief Helper for queue sorting logic.
   * @param m1 First message.
   * @param m2 Second message.
   * @return -1 if m1 < m2, 1 if m1 > m2, 0 otherwise.
   */
  int MessageRouter::compareByQueueMode(const core::Message &m1, const core::Message &m2) const
  {
    if (sendQueueMode == Q_MODE_FIFO)
    {
      if (m1.getReceiveTime() < m2.getReceiveTime())
        return -1;
      if (m1.getReceiveTime() > m2.getReceiveTime())
        return 1;
    }
    return 0; // Random mode relies on std::shuffle, not comparison
  }

  void MessageRouter::addApplication(std::shared_ptr<core::Application> app)
  {
    if (app)
    {
      this->applications[app->getAppID()].push_back(app);
    }
  }

  std::vector<std::shared_ptr<core::Application>> MessageRouter::getApplications(const std::string &ID) const
  {
    auto it = this->applications.find(ID);
    if (it != this->applications.end())
    {
      return it->second;
    }
    return std::vector<std::shared_ptr<core::Application>>();
  }

} // namespace routing