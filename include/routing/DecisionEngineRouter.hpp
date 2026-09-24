/**
 * @file DecisionEngineRouter.hpp
 * @brief Header for the DecisionEngineRouter class.
 *
 * @details
 * This class extends ActiveRouter and delegates routing decision making
 * to a RoutingDecisionEngine object.
 *
 * The DecisionEngineRouter maintains the routing state and forwarding
 * information, while the RoutingDecisionEngine is responsible for making
 * routing decisions.
 *
 * @author rikoNbs
 * @date September, 2026
 */

#pragma once

#include "ActiveRouter.hpp"
#include "RoutingDecisionEngine.hpp"

#include <memory>
#include <vector>
#include <string>
#include <unordered_set>
#include <unordered_map>

namespace core {

    class DTNHost;
    class Connection;
    class Message;
    class Configuration;

}

namespace routing {

    /**
     * @class DecisionEngineRouter
     * @brief Router that delegates routing decisions to a RoutingDecisionEngine.
     *
     * @details
     * DecisionEngineRouter extends ActiveRouter and uses a
     * RoutingDecisionEngine to determine how messages should be handled,
     * forwarded, and deleted.
     *
     * The router is responsible for managing the routing state and
     * communication with connected peers, while the RoutingDecisionEngine
     * provides the decision-making logic.
     */
    class DecisionEngineRouter : public ActiveRouter {

    public:

        /**
         * @brief Creates a DecisionEngineRouter using the given configuration.
         *
         * @param config The configuration object containing settings for the router.
         */
        explicit DecisionEngineRouter(core::Configuration& config);

        /**
         * @brief Copy constructor for DecisionEngineRouter.
         *
         * Creates a new DecisionEngineRouter by copying the state of
         * another router and replicating its decision engine.
         *
         * @param r The DecisionEngineRouter to copy from.
         */
        DecisionEngineRouter(const DecisionEngineRouter& r);

        /**
         * @brief Creates a replica of this router.
         *
         * @return A pointer to the replicated MessageRouter.
         */
        MessageRouter* replicate() override;

        /**
         * @brief Creates and adds a new message to the router.
         *
         * The RoutingDecisionEngine is consulted to determine whether
         * the message should be accepted for routing.
         *
         * @param m The message to be created.
         * @return True if the message is accepted for routing,
         *         false otherwise.
         */
        bool createNewMessage(
            std::shared_ptr<core::Message> m
        ) override;

        /**
         * @brief Handles a change in connection state.
         *
         * This method notifies the RoutingDecisionEngine when a connection
         * goes up or down and handles routing decisions for the connected peer.
         *
         * @param con The connection whose state has changed.
         */
        void changedConnection(
            core::Connection* con
        ) override;

        /**
         * @brief Handles an incoming message from another host.
         *
         * @param m The incoming message.
         * @param from The host that sent the message.
         * @return The result code indicating whether the message was accepted.
         */
        int receiveMessage(
            core::Message* m,
            core::DTNHost* from
        ) override;

        /**
         * @brief Handles a message after it has been successfully transferred.
         *
         * @param id The ID of the transferred message.
         * @param from The host that sent the message.
         * @return The received message.
         */
        core::Message* messageTransferred(
            const std::string& id,
            core::DTNHost* from
        ) override;

        /**
         * @brief Deletes a message from the router.
         *
         * The outgoing message list is also updated when a message
         * is deleted.
         *
         * @param id The ID of the message to delete.
         * @param drop True if the message was dropped, false otherwise.
         */
        void deleteMessage(
            const std::string& id,
            bool drop
        ) override;

        /**
         * @brief Performs periodic router updates.
         *
         * Updates the RoutingDecisionEngine and processes pending
         * message transfers.
         */
        void update() override;


    protected:

        /**
         * @brief Performs an information exchange with a connected peer.
         *
         * This method invokes the RoutingDecisionEngine exchange operation
         * for the given connection.
         *
         * @param con The connection used for the exchange.
         * @param otherHost The peer host connected through the connection.
         */
        void doExchange(
            core::Connection* con,
            core::DTNHost* otherHost
        );

        /**
         * @brief Marks that an information exchange has already been performed.
         *
         * @param con The connection for which the exchange was performed.
         */
        void didExchange(
            core::Connection* con
        );

        /**
         * @brief Starts transferring a message through a connection.
         *
         * @param m The message to transfer.
         * @param con The connection through which the message is transferred.
         * @return The result code of the transfer attempt.
         */
        int startTransfer(
            std::shared_ptr<core::Message> m,
            core::Connection* con
        ) override;

        /**
         * @brief Handles a completed message transfer.
         *
         * The RoutingDecisionEngine is consulted to determine whether
         * the transferred message should be deleted.
         *
         * @param con The connection used for the completed transfer.
         */
        void transferDone(
            core::Connection* con
        ) override;

        /**
         * @brief Determines whether the router should notify its peer.
         *
         * @param con The connection to check.
         * @return True if the peer should be notified,
         *         false otherwise.
         */
        bool shouldNotifyPeer(
            core::Connection* con
        );

        /**
         * @brief Finds connections to which a new message should be forwarded.
         *
         * The RoutingDecisionEngine is consulted for each connected host
         * to determine whether the message should be sent.
         *
         * @param m The message to evaluate.
         * @param from The host from which the message originated.
         */
        void findConnectionsForNewMessage(
            std::shared_ptr<core::Message> m,
            core::DTNHost* from
        );
    };

} // namespace routing