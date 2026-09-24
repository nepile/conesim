/**
 * @file EpidemicRouter.hpp
 * @brief Header definition of the EpidemicRouter class.
 * @details Epidemic message router with drop-oldest buffer and only single transferring
 *          connections at a time. Ported from The ONE simulator.
 * @author Neville
 * @date September, 2026
 */

#pragma once

#include "routing/ActiveRouter.hpp"

namespace core {
    class Configuration;
}

namespace routing {

using core::Configuration;

/**
 * @class EpidemicRouter
 * @brief Epidemic message router with drop-oldest buffer and single transferring
 *        connection at a time.
 */
class EpidemicRouter : public ActiveRouter {
public:
    /**
     * @brief Constructor. Creates a new message router based on the configuration in
     *        the given Configuration object.
     * @param config The Configuration object.
     */
    explicit EpidemicRouter(core::Configuration &config);

    virtual ~EpidemicRouter() = default;

    /**
     * @brief Performs periodic routing checks: tries deliverable messages first,
     *        then tries all messages to all connections.
     */
    void update() override;

    /**
     * @brief Creates a replicate of this epidemic router.
     * @return Pointer to a new EpidemicRouter instance with the same settings.
     */
    MessageRouter *replicate() override;

protected:
    /**
     * @brief Copy constructor.
     * @param r The router prototype where setting values are copied from.
     */
    EpidemicRouter(const EpidemicRouter &r);
};

} // namespace routing
