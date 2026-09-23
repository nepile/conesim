/**
 * @file ActiveRouter.cpp
 * @brief Implementation of the ActiveRouter base class.
 * @details Active router superclass for message transfer initiation, buffer management, and connection tracking.
 * @author Opeteer / Ported from The ONE (Java) to C++
 * @date September 2026
 */

#include "routing/ActiveRouter.hpp"
#include "core/DTNHost.hpp"
#include "core/Message.hpp"
#include "core/Connection.hpp"
#include "core/Configuration.hpp"
#include "core/SimulationClock.hpp"

#include <algorithm>
#include <random>

namespace routing {

ActiveRouter::ActiveRouter(Settings &s)
    : MessageRouter(s),
      deleteDelivered(false),
      lastTtlCheck(0.0) {
    if (s.contains(DELETE_DELIVERED_S)) {
        deleteDelivered = s.getBoolean(DELETE_DELIVERED_S);
    }
}

ActiveRouter::ActiveRouter(const ActiveRouter &r)
    : MessageRouter(r),
      deleteDelivered(r.deleteDelivered),
      lastTtlCheck(0.0) {
}

void ActiveRouter::init(DTNHost *host, const std::vector<std::shared_ptr<MessageListener>> &mListeners) {
    MessageRouter::init(host, mListeners);
    sendingConnections.clear();
    lastTtlCheck = 0.0;
}

void ActiveRouter::changedConnection(Connection *con) {
    // Subclasses may override
}

bool ActiveRouter::requestDeliverableMessages(Connection *con) {
    if (isTransferring()) {
        return false;
    }

    DTNHost *other = con->getOtherNode(getHost());
    auto temp = getMessageCollection();
    for (const auto &m : temp) {
        if (other == m->getTo()) {
            if (startTransfer(m, con) == RCV_OK) {
                return true;
            }
        }
    }
    return false;
}

bool ActiveRouter::createNewMessage(std::shared_ptr<Message> m) {
    makeRoomForNewMessage(m->getSize());
    return MessageRouter::createNewMessage(m);
}

int ActiveRouter::receiveMessage(Message *m, DTNHost *from) {
    int recvCheck = checkReceiving(*m);
    if (recvCheck != RCV_OK) {
        return recvCheck;
    }

    return MessageRouter::receiveMessage(m, from);
}

Message *ActiveRouter::messageTransferred(const std::string &id, DTNHost *from) {
    Message *m = MessageRouter::messageTransferred(id, from);

    if (m != nullptr && m->getTo() == getHost() && m->getResponseSize() > 0) {
        // generate a response message
        auto res = std::make_shared<Message>(
            getHost(),
            m->getFrom(),
            RESPONSE_PREFIX + m->getId(),
            m->getResponseSize()
        );
        createNewMessage(res);
        auto resPtr = getMessage(RESPONSE_PREFIX + m->getId());
        if (resPtr != nullptr) {
            resPtr->setRequest(m->shared_from_this());
        }
    }

    return m;
}

bool ActiveRouter::isTransferring() const {
    if (!sendingConnections.empty()) {
        return true;
    }

    if (getHost() == nullptr || getHost()->getConnections().empty()) {
        return false;
    }

    auto connections = getConnections();
    for (const auto *con : connections) {
        if (!con->isReadyForTransfer()) {
            return true;
        }
    }

    return false;
}

bool ActiveRouter::isSending(const std::string &msgId) const {
    for (const auto *con : sendingConnections) {
        auto msg = con->getMessage();
        if (msg == nullptr) {
            continue;
        }
        if (msg->getId() == msgId) {
            return true;
        }
    }
    return false;
}

std::vector<Connection *> ActiveRouter::getConnections() const {
    if (getHost() != nullptr) {
        return getHost()->getConnections();
    }
    return {};
}

int ActiveRouter::startTransfer(std::shared_ptr<Message> m, Connection *con) {
    if (!con->isReadyForTransfer()) {
        return TRY_LATER_BUSY;
    }

    int retVal = con->startTransfer(getHost(), m);
    if (retVal == RCV_OK) {
        addToSendingConnections(con);
    } else if (deleteDelivered && retVal == DENIED_OLD &&
               m->getTo() == con->getOtherNode(getHost())) {
        deleteMessage(m->getId(), false);
    }

    return retVal;
}

bool ActiveRouter::canStartTransfer() const {
    if (getNrofMessages() == 0) {
        return false;
    }
    if (getConnections().empty()) {
        return false;
    }

    return true;
}

int ActiveRouter::checkReceiving(const Message &m) {
    if (isTransferring()) {
        return TRY_LATER_BUSY;
    }

    if (hasMessage(m.getId()) || isDeliveredMessage(m)) {
        return DENIED_OLD;
    }

    if (m.getTtl() <= 0 && m.getTo() != getHost()) {
        return DENIED_TTL;
    }

    if (!makeRoomForMessage(m.getSize())) {
        return DENIED_NO_SPACE;
    }

    return RCV_OK;
}

bool ActiveRouter::makeRoomForMessage(int size) {
    if (size > getBufferSize()) {
        return false;
    }

    int freeBuffer = getFreeBufferSize();
    while (freeBuffer < size) {
        Message *m = getOldestMessage(true);

        if (m == nullptr) {
            return false;
        }

        deleteMessage(m->getId(), true);
        freeBuffer += m->getSize();
    }

    return true;
}

void ActiveRouter::dropExpiredMessages() {
    auto messages = getMessageCollection();
    for (const auto &m : messages) {
        int ttl = m->getTtl();
        if (ttl <= 0) {
            deleteMessage(m->getId(), true);
        }
    }
}

void ActiveRouter::makeRoomForNewMessage(int size) {
    makeRoomForMessage(size);
}

Message *ActiveRouter::getOldestMessage(bool excludeMsgBeingSent) {
    auto messages = getMessageCollection();
    Message *oldest = nullptr;
    for (const auto &m : messages) {
        if (excludeMsgBeingSent && isSending(m->getId())) {
            continue;
        }

        if (oldest == nullptr) {
            oldest = m.get();
        } else if (oldest->getReceiveTime() > m->getReceiveTime()) {
            oldest = m.get();
        }
    }

    return oldest;
}

std::vector<std::pair<Message *, Connection *>> ActiveRouter::getMessagesForConnected() {
    if (getNrofMessages() == 0 || getConnections().empty()) {
        return {};
    }

    std::vector<std::pair<Message *, Connection *>> forTuples;
    for (const auto &m : getMessageCollection()) {
        for (auto *con : getConnections()) {
            DTNHost *to = con->getOtherNode(getHost());
            if (m->getTo() == to) {
                forTuples.push_back({m.get(), con});
            }
        }
    }

    return forTuples;
}

std::pair<Message *, Connection *> ActiveRouter::tryMessagesForConnected(
    const std::vector<std::pair<Message *, Connection *>> &tuples) {
    if (tuples.empty()) {
        return {nullptr, nullptr};
    }

    for (const auto &t : tuples) {
        Message *m = t.first;
        Connection *con = t.second;
        if (m != nullptr && con != nullptr) {
            if (startTransfer(m->shared_from_this(), con) == RCV_OK) {
                return t;
            }
        }
    }

    return {nullptr, nullptr};
}

Message *ActiveRouter::tryAllMessages(Connection *con, const std::vector<std::shared_ptr<Message>> &messages) {
    for (const auto &m : messages) {
        int retVal = startTransfer(m, con);
        if (retVal == RCV_OK) {
            return m.get();
        } else if (retVal > 0) {
            return nullptr;
        }
    }

    return nullptr;
}

Connection *ActiveRouter::tryMessagesToConnections(
    const std::vector<std::shared_ptr<Message>> &messages,
    const std::vector<Connection *> &connections) {
    for (auto *con : connections) {
        Message *started = tryAllMessages(con, messages);
        if (started != nullptr) {
            return con;
        }
    }

    return nullptr;
}

Connection *ActiveRouter::tryAllMessagesToAllConnections() {
    auto connections = getConnections();
    if (connections.empty() || getNrofMessages() == 0) {
        return nullptr;
    }

    auto messages = getMessageCollection();
    sortByQueueMode(messages);

    return tryMessagesToConnections(messages, connections);
}

Connection *ActiveRouter::exchangeDeliverableMessages() {
    auto connections = getConnections();

    if (connections.empty()) {
        return nullptr;
    }

    auto tuples = getMessagesForConnected();
    sortByQueueMode(tuples);
    auto t = tryMessagesForConnected(tuples);

    if (t.first != nullptr && t.second != nullptr) {
        return t.second;
    }

    for (auto *con : connections) {
        if (con->getOtherNode(getHost())->requestDeliverableMessages(con)) {
            return con;
        }
    }

    return nullptr;
}

void ActiveRouter::shuffleMessages(std::vector<std::shared_ptr<Message>> &messages) {
    if (messages.size() <= 1) {
        return;
    }

    std::mt19937 rng(static_cast<unsigned int>(core::SimulationClock::getIntTime()));
    std::shuffle(messages.begin(), messages.end(), rng);
}

void ActiveRouter::addToSendingConnections(Connection *con) {
    sendingConnections.push_back(con);
}

void ActiveRouter::transferAborted(Connection *con) {
    // Hook for subclasses
}

void ActiveRouter::transferDone(Connection *con) {
    // Hook for subclasses
}

void ActiveRouter::update() {
    MessageRouter::update();

    for (size_t i = 0; i < sendingConnections.size();) {
        bool removeCurrent = false;
        Connection *con = sendingConnections[i];

        if (con->isMessageTransferred()) {
            if (con->getMessage() != nullptr) {
                transferDone(con);
                con->finalizeTransfer();
            }
            removeCurrent = true;
        } else if (!con->isUp()) {
            if (con->getMessage() != nullptr) {
                transferAborted(con);
                con->abortTransfer();
            }
            removeCurrent = true;
        }

        if (removeCurrent) {
            if (getFreeBufferSize() < 0) {
                makeRoomForMessage(0);
            }
            sendingConnections.erase(sendingConnections.begin() + i);
        } else {
            i++;
        }
    }

    if (core::SimulationClock::getTime() - lastTtlCheck >= TTL_CHECK_INTERVAL &&
        sendingConnections.empty()) {
        dropExpiredMessages();
        lastTtlCheck = core::SimulationClock::getTime();
    }
}

} // namespace routing
