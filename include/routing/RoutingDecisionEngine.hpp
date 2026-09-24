/**
 * @file RoutingDecisionEngine.hpp
 * @brief Defines the RoutingDecisionEngine interface for routing decisions in a DTN simulation.
 *
 * @details This interface provides a contract for implementing routing decision
 * logic in Delay Tolerant Network (DTN) simulations. It defines the methods
 * used by DecisionEngineRouter to communicate with its decision-making object.
 *
 * @author rikoNbs
 * @date September, 2026
 */

#pragma once

#include <memory>

namespace routing {

class DTNHost;
class Connection;
class Message;

/**
 * @class RoutingDecisionEngine
 * @brief Defines the interface between DecisionEngineRouter and its decision-making object.
 *
 * The RoutingDecisionEngine interface defines the methods used by
 * DecisionEngineRouter to communicate with its decision-making object.
 */
class RoutingDecisionEngine
{
public:

    /**
     * @brief Virtual destructor.
     */
    virtual ~RoutingDecisionEngine() = default;

    /**
     * @brief Called when a connection goes up between this host and a peer.
     *
     * Note that doExchangeForNewConnection() may be called first.
     *
     * @param thisHost The host associated with this decision engine.
     * @param peer The peer host connected to this host.
     */
    virtual void connectionUp(
        DTNHost& thisHost,
        DTNHost& peer
    ) = 0;

    /**
     * @brief Called when a connection goes down between this host and a peer.
     *
     * @param thisHost The host associated with this decision engine.
     * @param peer The peer host connected to this host.
     */
    virtual void connectionDown(
        DTNHost& thisHost,
        DTNHost& peer
    ) = 0;

    /**
     * @brief Called once for each connection that comes up to give two
     * decision engine objects on either end of the connection to exchange
     * and update information in a simultaneous fashion.
     *
     * This call is provided so that one end of the connection does not
     * perform an update based on newly updated information from the
     * opposite end of the connection. Real life would reflect an update
     * based on the old peer information.
     *
     * @param con The connection that has come up.
     * @param peer The peer host connected to this host.
     */
    virtual void doExchangeForNewConnection(
        Connection& con,
        DTNHost& peer
    ) = 0;

    /**
     * @brief Allows the decision engine to gather information from the given
     * message and determine if it should be forwarded on or discarded.
     *
     * This method is only called when a message originates at the current
     * host, not when received from a peer. In this way, applications can
     * use a Message to communicate information to this routing layer.
     *
     * @param m The new Message to consider routing.
     * @return True if the message should be forwarded on.
     * @return False if the message should be discarded.
     */
    virtual bool newMessage(
        Message& m
    ) = 0;

    /**
     * @brief Determines if the given host is an intended recipient of the
     * given Message.
     *
     * This method is expected to be called when a new Message is received
     * at a given router.
     *
     * @param m Message just received.
     * @param aHost Host to check.
     * @return True if the given host is a recipient of this given message.
     * False otherwise.
     */
    virtual bool isFinalDestination(
        Message& m,
        DTNHost& aHost
    ) = 0;

    /**
     * @brief Called to determine if a new message received from a peer should
     * be saved to the host's message store and further forwarded on.
     *
     * @param m Message just received.
     * @param thisHost The requesting host.
     * @return True if the message should be saved and further routed.
     * False otherwise.
     */
    virtual bool shouldSaveReceivedMessage(
        Message& m,
        DTNHost& thisHost
    ) = 0;

    /**
     * @brief Called to determine if the given Message should be sent to
     * the given host.
     *
     * This method will often be called multiple times in succession as the
     * DecisionEngineRouter loops through its respective Message or
     * Connection Collections.
     *
     * @param m Message to possibly send.
     * @param otherHost Peer to potentially send the message to.
     * @param thisHost The host associated with this decision engine.
     * @return True if the message should be sent.
     * @return False if the message should not be sent.
     */
    virtual bool shouldSendMessageToHost(
        Message& m,
        DTNHost& otherHost,
        DTNHost& thisHost
    ) = 0;

    /**
     * @brief Called after a message is sent to some other peer to ask if it
     * should now be deleted from the message store.
     *
     * @param m Sent message.
     * @param otherHost Host who received the message.
     * @return True if the message should be deleted.
     * @return False if the message should not be deleted.
     */
    virtual bool shouldDeleteSentMessage(
        Message& m,
        DTNHost& otherHost
    ) = 0;

    /**
     * @brief Called if an attempt was unsuccessfully made to transfer a
     * message to a peer and the return code indicates the message is old
     * or already delivered, in which case it might be appropriate to
     * delete the message.
     *
     * @param m Old Message.
     * @param hostReportingOld Peer claiming the message is old.
     * @return True if the message should be deleted.
     * @return False if the message should not be deleted.
     */
    virtual bool shouldDeleteOldMessage(
        Message& m,
        DTNHost& hostReportingOld
    ) = 0;

    /**
     * @brief Updates the decision engine for the given host.
     *
     * @param thisHost The host associated with this decision engine.
     */
    virtual void update(
        DTNHost& thisHost
    ) = 0;

    /**
     * @brief Duplicates this decision engine.
     *
     * @return A duplicated RoutingDecisionEngine.
     */
    virtual std::unique_ptr<RoutingDecisionEngine> replicate() = 0;
};

} // namespace routing