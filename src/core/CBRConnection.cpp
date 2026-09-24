// CBRConnection.cpp

#include "core/CBRConnection.hpp"

#include "core/DTNHost.hpp"
#include "core/Message.hpp"
#include "core/NetworkInterface.hpp"
#include "core/SimulationClock.hpp"
#include "routing/MessageRouter.hpp"

#include <cassert>
#include <string>

namespace core {

CBRConnection::CBRConnection(
    DTNHost* fromNode,
    NetworkInterface* fromInterface,
    DTNHost* toNode,
    NetworkInterface* toInterface,
    int connectionSpeed
)
    : Connection(fromNode, fromInterface, toNode, toInterface),
      speed(connectionSpeed),
      transferDoneTime(0.0)
{
}

void CBRConnection::clearMsgOnFly()
{
    Connection::clearMsgOnFly();
    transferDoneTime = 0.0;
}

int CBRConnection::startTransfer(DTNHost* from, std::shared_ptr<Message> m)
{
    assert(
        msgOnFly == nullptr &&
        "Already transferring a message. Cannot start another transfer."
    );

    msgFromNode = from;

    std::shared_ptr<Message> newMessage = m->replicate();

    int retVal = getOtherNode(from)->receiveMessage(newMessage, from);

    if (retVal == routing::MessageRouter::RCV_OK) {
        msgOnFly = newMessage;

        transferDoneTime =
            SimulationClock::getTime() +
            (static_cast<double>(m->getSize()) / speed);
    }

    return retVal;
}

void CBRConnection::abortTransfer()
{
    Connection::abortTransfer();
    transferDoneTime = 0.0;
}

double CBRConnection::getTransferDoneTime() const
{
    return transferDoneTime;
}

bool CBRConnection::isMessageTransferred() const
{
    return getRemainingByteCount() == 0;
}

double CBRConnection::getSpeed() const
{
    return static_cast<double>(speed);
}

int CBRConnection::getRemainingByteCount() const
{
    if (msgOnFly == nullptr) {
        return 0;
    }

    int remaining = static_cast<int>(
        (transferDoneTime - SimulationClock::getTime()) * speed
    );

    return remaining > 0 ? remaining : 0;
}

std::string CBRConnection::toString() const
{
    return Connection::toString() +
           (msgOnFly ? " until " + std::to_string(transferDoneTime) : "");
}

} // namespace core