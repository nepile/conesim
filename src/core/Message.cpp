/**
 * @file Message.cpp
 * @brief Implementation of the Message class in ONE Simulator C++ port.
 * @author Neville Jeremy Onorato Laia
 * 
 * Copyright 2010 Aalto University, ComNet
 * Released under GPLv3. See LICENSE.txt for details. 
 */

#include "Message.hpp"
#include "core/DTNHost.hpp"
#include "core/SimulationError.hpp"
#include "core/SimulationClock.hpp"
#include <limits>

namespace core {

    int Message::nextUniqueId = 0;

    /**
     * @copydoc Message::Message(DTNHost*, DTNHost*, const std::string&, int)
     */
    Message::Message(DTNHost* from, DTNHost* to, const std::string& id, int size)
        : from(from), 
          to(to), 
          id(id), 
          size(size), 
          uniqueId(nextUniqueId++), 
          timeCreated(SimulationClock::getTime()),
          timeReceived(timeCreated), 
          initTtl(INFINITE_TTL), 
          responseSize(0), 
          requestMsg(nullptr), 
          appID("") {
        
        addNodeOnPath(from);
    }

    /**
     * @copydoc Message::getFrom()
     */
    DTNHost* Message::getFrom() const { return this->from; }

    /**
     * @copydoc Message::getTo()
     */
    DTNHost* Message::getTo() const { return this->to; }

    /**
     * @copydoc Message::getId()
     */
    std::string Message::getId() const { return this->id; }
    
    /**
     * @copydoc Message::getUniqueId()
     */
    int Message::getUniqueId() const { return this->uniqueId; }
    
    /**
     * @copydoc Message::getSize()
     */
    int Message::getSize() const { return this->size; }

    /**
     * @copydoc Message::addNodeOnPath(DTNHost*)
     */
    void Message::addNodeOnPath(DTNHost* node) {
        this->path.push_back(node);
    }
    
    /**
     * @copydoc Message::getHops()
     */
    std::vector<DTNHost*> Message::getHops() const {
        return this->path;
    }
    
    /**
     * @copydoc Message::getHopCount()
     */
    int Message::getHopCount() const {
        return static_cast<int>(this->path.size()) - 1;
    }
    
    /** 
     * @copydoc Message::getTtl()
     */
    int Message::getTtl() const {
        if (this->initTtl == INFINITE_TTL) {
            return std::numeric_limits<int>::max();
        } else {
            return static_cast<int>(
                ((this->initTtl * 60) - (SimulationClock::getTime() - this->timeCreated)) / 60.0
            );
        }
    }
    
    /**
     * @copydoc Message::setTtl(int)
     */
    void Message::setTtl(int ttl) { this->initTtl = ttl; }
    
    /**
     * @copydoc Message::setReceiveTime(double)
     */
    void Message::setReceiveTime(double time) { this->timeReceived = time; }
    
    /**
     * @copydoc Message::getReceiveTime()
     */
    double Message::getReceiveTime() const { return this->timeReceived; }
    
    /**
     * @copydoc Message::getCreationTime()
     */
    double Message::getCreationTime() const { return this->timeCreated; }
    
    /**
     * @copydoc Message::setRequest(std::shared_ptr<Message>)
     */
    void Message::setRequest(std::shared_ptr<Message> request) { this->requestMsg = request; }
    
    /**
     * @copydoc Message::getRequest()
     */
    std::shared_ptr<Message> Message::getRequest() const { return this->requestMsg; }
    
    /**
     * @copydoc Message::isResponse()
     */
    bool Message::isResponse() const { return this->requestMsg != nullptr; }
    
    /**
     * @copydoc Message::setResponseSize(int)
     */
    void Message::setResponseSize(int size) { this->responseSize = size; }
    
    /**
     * @copydoc Message::getResponseSize()
     */
    int Message::getResponseSize() const { return this->responseSize; }
    
    /**
     * @copydoc Message::toString()
     */
    std::string Message::toString() const { return this->id; }

    /**
     * @copydoc Message::copyFrom(const Message&)
     */
    void Message::copyFrom(const Message& m) {
        this->path = m.path;
        this->timeCreated = m.timeCreated;
        this->responseSize = m.responseSize;
        this->requestMsg = m.requestMsg;
        this->initTtl = m.initTtl;
        this->appID = m.appID;
        
        for (const auto& pair : m.properties) {
            updateProperty(pair.first, pair.second);
        }
    }
    
    /**
     * @copydoc Message::addProperty(const std::string&, const std::any&)
     */
    void Message::addProperty(const std::string& key, const std::any& value) {
        if (this->properties.find(key) != this->properties.end()) {
            throw SimulationError("Message " + this->toString() + " already contains value for a key " + key);
        }
        this->updateProperty(key, value);
    }
    
    /**
     * @copydoc Message::getProperty(const std::string&)
     */
    std::any Message::getProperty(const std::string& key) const {
        auto it = this->properties.find(key);
        if (it != this->properties.end()) {
            return it->second;
        }
        return std::any();
    }
    
    /**
     * @copydoc Message::updateProperty(const std::string&, const std::any&)
     */
    void Message::updateProperty(const std::string& key, const std::any& value) {
        this->properties[key] = value;
    }
    
    /**
     * @copydoc Message::replicate()
     */
    std::shared_ptr<Message> Message::replicate() const {
        auto m = std::make_shared<Message>(this->from, this->to, this->id, this->size);
        m->copyFrom(*this);
        return m;
    }
    
    /**
     * @copydoc Message::operator<(const Message&)
     */
    bool Message::operator<(const Message& m) const {
        return this->toString() < m.toString();
    }
    
    /**
     * @copydoc Message::reset()
     */
    void Message::reset() {
        nextUniqueId = 0;
    }

    /**
     * @copydoc Message::getAppID()
     */
    std::string Message::getAppID() const { return this->appID; }

    /**
     * @copydoc Message::setAppID(const std::string&)
     */
    void Message::setAppID(const std::string& appID) { this->appID = appID; }
    
} // namespace core