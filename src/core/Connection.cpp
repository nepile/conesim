/**
 * @file Connection.cpp
 * @brief Implementation of the Connection base class.
 * @details Represents a physical or logical connection between two network interfaces.
 *          Handles message transfers, connection states, and byte transfer tracking.
 * @author Frathol
 * @date September, 2026
 */

#include "core/Connection.hpp"
#include "core/DTNHost.hpp"
#include "core/NetworkInterface.hpp"
#include "core/Message.hpp"

#include <stdexcept>
#include <sstream>

namespace core
{

  /**
   * @brief Constructs a new connection and sets its state to active (up).
   * @param fromNode The host initiating the connection.
   * @param fromInterface The interface initiating the connection.
   * @param toNode The destination host.
   * @param toInterface The destination interface.
   */
  Connection::Connection(DTNHost *fromNode, NetworkInterface *fromInterface,
                         DTNHost *toNode, NetworkInterface *toInterface)
      : toNode(toNode),
        toInterface(toInterface),
        fromNode(fromNode),
        fromInterface(fromInterface),
        msgFromNode(nullptr),
        _isUp(true),
        msgOnFly(nullptr),
        bytesTransferred(0)
  {
  }

  /**
   * @brief Checks if the connection is currently active.
   * @return True if the connection is up, false otherwise.
   */
  bool Connection::isUp() const
  {
    return this->_isUp;
  }

  /**
   * @brief Verifies if the specified node is the one that initiated this connection.
   * @param node Pointer to the DTNHost to check.
   * @return True if the node is the initiator, false otherwise.
   */
  bool Connection::isInitiator(DTNHost *node) const
  {
    return node == this->fromNode;
  }

  /**
   * @brief Manually sets the active state of the connection.
   * @param state True to set the connection up, false to set it down.
   */
  void Connection::setUpState(bool state)
  {
    this->_isUp = state;
  }

  /**
   * @brief Updates the transmission speed and calculates missing data.
   * @details Default implementation is empty; intended to be overridden by subclasses.
   */
  void Connection::update()
  {
    // Empty default implementation
  }

  /**
   * @brief Aborts the currently ongoing message transfer.
   * @details Calculates the bytes transferred so far, notifies the sending node
   *          that the transfer was aborted, and clears the connection buffer.
   * @throws std::runtime_error If there is no message currently transferring.
   */
  void Connection::abortTransfer()
  {
    if (!this->msgOnFly)
    {
      throw std::runtime_error("No message to abort at connection.");
    }

    int bytesRemaining = getRemainingByteCount();

    this->bytesTransferred += (this->msgOnFly->getSize() - bytesRemaining);

    // Notify the sending node that the transfer was aborted
    getOtherNode(this->msgFromNode)->messageAborted(this->msgOnFly->getId(), this->msgFromNode, bytesRemaining);

    clearMsgOnFly();
  }

  /**
   * @brief Safely clears the message currently in transit from the buffer.
   */
  void Connection::clearMsgOnFly()
  {
    this->msgOnFly.reset();
    this->msgFromNode = nullptr;
  }

  /**
   * @brief Finalizes a successfully completed message transfer.
   * @details Updates the total transferred byte count, notifies the receiving node,
   *          and clears the connection buffer.
   * @throws std::runtime_error If there is no active message transfer.
   */
  void Connection::finalizeTransfer()
  {
    if (!this->msgOnFly)
    {
      throw std::runtime_error("Nothing to finalize in this connection.");
    }
    if (!this->msgFromNode)
    {
      throw std::runtime_error("msgFromNode is not set during finalization.");
    }

    this->bytesTransferred += this->msgOnFly->getSize();

    getOtherNode(this->msgFromNode)->messageTransferred(this->msgOnFly->getId(), this->msgFromNode);

    clearMsgOnFly();
  }

  /**
   * @brief Checks if the connection is active and ready to accept a new message.
   * @return True if the connection is up and not currently transferring a message.
   */
  bool Connection::isReadyForTransfer() const
  {
    return this->_isUp && (this->msgOnFly == nullptr);
  }

  /**
   * @brief Retrieves the message currently being transferred.
   * @return A shared pointer to the message in transit.
   */
  std::shared_ptr<Message> Connection::getMessage() const
  {
    return this->msgOnFly;
  }

  /**
   * @brief Calculates the total amount of bytes this connection has transferred over its lifetime.
   * @return Total bytes transferred, including partially transferred bytes from an ongoing transfer.
   */
  int Connection::getTotalBytesTransferred() const
  {
    if (!this->msgOnFly)
    {
      return this->bytesTransferred;
    }
    else
    {
      if (isMessageTransferred())
      {
        return this->bytesTransferred + this->msgOnFly->getSize();
      }
      else
      {
        return this->bytesTransferred +
               (this->msgOnFly->getSize() - getRemainingByteCount());
      }
    }
  }

  /**
   * @brief Retrieves the node on the opposite end of this connection.
   * @param node The node on one end.
   * @return Pointer to the host on the other end.
   */
  DTNHost *Connection::getOtherNode(DTNHost *node) const
  {
    return (node == this->fromNode) ? this->toNode : this->fromNode;
  }

  /**
   * @brief Retrieves the network interface on the opposite end of this connection.
   * @param i The interface on one end.
   * @return Pointer to the network interface on the other end.
   */
  NetworkInterface *Connection::getOtherInterface(NetworkInterface *i) const
  {
    return (i == this->fromInterface) ? this->toInterface : this->fromInterface;
  }

  /**
   * @brief Generates a string representation of the connection's status.
   * @return Formatted string including endpoints, speed, state, and active transfers.
   */
  std::string Connection::toString() const
  {
    std::ostringstream oss;
    oss << (this->fromNode ? this->fromNode->toString() : "Unknown")
        << "<->"
        << (this->toNode ? this->toNode->toString() : "Unknown")
        << " (" << getSpeed() << "Bps) is "
        << (isUp() ? "up" : "down");

    if (this->msgOnFly)
    {
      oss << " transferring " << this->msgOnFly->getId()
          << " from " << (this->msgFromNode ? this->msgFromNode->toString() : "Unknown");
    }

    return oss.str();
  }

} // namespace core