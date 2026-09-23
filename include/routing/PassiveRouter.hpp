/**
 * @file PassiveRouter.hpp
 * @brief Header definition of the PassiveRouter class.
 * @details A passive router that does not initiate any message transfers unless
 *          explicitly commanded. This is highly useful for external event-controlled
 *          routing, static access points, or dummy nodes in the simulation.
 *          For implementation specifics, refer to the MessageRouter base class.
 * @author Frathol
 * @date September, 2026
 */

#pragma once

#include "routing/MessageRouter.hpp"

namespace core {
    class Connection;
    class Configuration;
}

using Connection = core::Connection;
using Settings = core::Configuration;

namespace routing
{

  /**
   * @class PassiveRouter
   * @brief A router that passively holds messages without actively sending them.
   */
  class PassiveRouter : public MessageRouter
  {
  public:
    /**
     * @brief Constructor to initialize the passive router.
     * @param s Reference to the Settings object containing configurations.
     */
    explicit PassiveRouter(Settings &s);

    /**
     * @brief Default destructor.
     */
    virtual ~PassiveRouter() = default;

    /**
     * @brief Updates the router state.
     * @details For a passive router, this simply calls the base class update
     *          to process attached applications, doing no active routing.
     */
    void update() override;

    /**
     * @brief Called when a connection's state changes.
     * @details Does nothing for passive routers, as they do not actively manage connections.
     * @param con The connection that changed.
     */
    void changedConnection(Connection *con) override;

    /**
     * @brief Creates a replica of this passive router.
     * @return Pointer to a new PassiveRouter instance with the same settings.
     */
    MessageRouter *replicate() override;

  protected:
    /**
     * @brief Copy constructor.
     * @details Kept protected to force the use of the replicate() method
     *          for polymorphic duplication.
     * @param r The prototype router to copy settings from.
     */
    PassiveRouter(const PassiveRouter &r);
  };

} // namespace routing