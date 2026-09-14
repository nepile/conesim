#pragma once

class Message;
class Host;

namespace conesim {

/**
 * @file MessageListener.hpp
 * @brief Interface for monitoring message lifecycle and transmission events.
 */

/**
 * @class MessageListener
 * @brief Abstract listener interface notified of message-related actions in the network.
 *
 * Classes that track routing performance, message delivery delays, hop counts,
 * or message drop rates should implement this interface and register with the
 * event dispatcher.
 */
class MessageListener {
public:
    /**
     * @brief Virtual destructor to ensure safe polymorphic cleanup.
     */
    virtual ~MessageListener();

    /**
     * @brief Callback invoked when a new message is generated in the simulation.
     *
     * @param msg The message that was created.
     */
    virtual void newMessage(Message msg) = 0;

    /**
     * @brief Callback invoked when a message starts transmitting across a link.
     *
     * @param msg The message initiating transmission.
     * @param from The sending host.
     * @param to The receiving host.
     */
    virtual void messageTransferStarted(Message msg, Host from, Host to) = 0;

    /**
     * @brief Callback invoked when a message is deleted or dropped from a host's buffer.
     *
     * @param msg The message being removed.
     * @param where The host where the deletion occurred.
     * @param dropped True if the message was dropped due to buffer overflow or TTL expiration;
     *                false if deleted intentionally (e.g., after final delivery).
     */
    virtual void messageDeleted(Message msg, Host where, bool dropped) = 0;

    /**
     * @brief Callback invoked when an in-flight message transmission is interrupted before completion.
     *
     * @param msg The message whose transfer was aborted.
     * @param from The host that was transmitting.
     * @param to The intended recipient host.
     */
    virtual void messageTransferAborted(Message msg, Host from, Host to) = 0;

    /**
     * @brief Callback invoked when a message transfer successfully completes.
     *
     * @param msg The message that was delivered.
     * @param from The host that sent the message.
     * @param to The host that received the message.
     * @param firstDelivery True if this transfer marks the first time the message
     *                      reached its final destination; false otherwise.
     */
    virtual void messageTransferred(Message msg, Host from, Host to, bool firstDelivery) = 0;
};

} // namespace conesim