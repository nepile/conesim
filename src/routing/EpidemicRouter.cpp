/**
 * @file EpidemicRouter.cpp
 * @brief Implementation of the EpidemicRouter class.
 * @details Ported from The ONE simulator's EpidemicRouter.java.
 * @author Neville
 * @date September, 2026
 */

#include "routing/EpidemicRouter.hpp"

namespace routing {

EpidemicRouter::EpidemicRouter(core::Configuration &config)
    : ActiveRouter(config) {
}

EpidemicRouter::EpidemicRouter(const EpidemicRouter &r)
    : ActiveRouter(r) {
}

void EpidemicRouter::update() {
    ActiveRouter::update();

    if (isTransferring() || !canStartTransfer()) {
        return; // transferring, don't try other connections yet
    }

    // Try first the messages that can be delivered to final recipient
    if (exchangeDeliverableMessages() != nullptr) {
        return; // started a transfer, don't try others (yet)
    }

    // then try any/all message to any/all connection
    tryAllMessagesToAllConnections();
}

MessageRouter *EpidemicRouter::replicate() {
    return new EpidemicRouter(*this);
}

} // namespace routing
