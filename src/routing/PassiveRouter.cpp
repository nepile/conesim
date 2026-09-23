/**
 * @file PassiveRouter.cpp
 * @brief Implementation of the PassiveRouter class.
 * @details A router that passively holds messages without actively initiating transfers.
 * @author Frathol / Ported to C++
 * @date September 2026
 */

#include "routing/PassiveRouter.hpp"
#include "core/Connection.hpp"
#include "core/Configuration.hpp"

namespace routing {

PassiveRouter::PassiveRouter(Settings &s)
    : MessageRouter(s) {
}

PassiveRouter::PassiveRouter(const PassiveRouter &r)
    : MessageRouter(r) {
}

void PassiveRouter::update() {
    MessageRouter::update();
}

void PassiveRouter::changedConnection(Connection *con) {
    // Passive router does not actively react to connection state changes
}

MessageRouter *PassiveRouter::replicate() {
    return new PassiveRouter(*this);
}

} // namespace routing
