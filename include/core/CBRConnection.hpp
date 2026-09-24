// CBRConnection.hpp

#pragma once

#include "Connection.hpp"

namespace core {

class DTNHost;
class NetworkInterface;
class Message;

/**
 * @brief A constant bit-rate connection between two DTN nodes.
 */
class CBRConnection : public core::Connection {
private:
    int speed;
    double transferDoneTime;

protected:
    void clearMsgOnFly() override;

public:
    /**
     * @brief Creates a new connection between nodes and sets the connection
     * state to "up".
     *
     * @param fromNode The node that initiated the connection.
     * @param fromInterface The interface that initiated the connection.
     * @param toNode The node on the other side of the connection.
     * @param toInterface The interface on the other side of the connection.
     * @param connectionSpeed Transfer speed of the connection in Bps when
     * the connection is initiated.
     */
    CBRConnection(
        DTNHost* fromNode,
        NetworkInterface* fromInterface,
        DTNHost* toNode,
        NetworkInterface* toInterface,
        int connectionSpeed
    );

    /**
     * @brief Sets a message that this connection is currently transferring.
     *
     * Only one message at a time can be transferred using one connection.
     *
     * @param from The host sending the message.
     * @param m The message to transfer.
     * @return The result returned by the receiving host's message router.
     */
    int startTransfer(DTNHost* from, std::shared_ptr<Message> m) override;

    /**
     * @brief Aborts the transfer of the currently transferred message.
     */
    void abortTransfer() override;

    /**
     * @brief Gets the time at which the current transfer will finish.
     *
     * @return The transfer completion time.
     */
    double getTransferDoneTime() const;

    /**
     * @brief Returns whether the current message transfer is complete.
     *
     * @return true if the transfer is complete, false otherwise.
     */
    bool isMessageTransferred() const override;

    /**
     * @brief Gets the current connection speed.
     *
     * @return Connection speed in Bps.
     */
    double getSpeed() const override;

    /**
     * @brief Gets the amount of bytes remaining to be transferred.
     *
     * @return Remaining bytes, or 0 if there is no active transfer or the
     * transfer has already completed.
     */
    int getRemainingByteCount() const override;

    /**
     * @brief Returns a string representation of the connection.
     *
     * @return String representation of the connection.
     */
    std::string toString() const override;
};

} // namespace core