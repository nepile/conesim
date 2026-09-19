/**
 * @file ApplicationListener.hpp
 * @brief Header definition for the ApplicationListener interface.
 * @details Interface for classes that want to listen and react to application events.
 * @author mjpitka (Original Java) / Frathol
 * @date September 2026
 * 
 * Copyright 2010 Aalto University, ComNet
 * Released under GPLv3. See LICENSE.txt for details. 
 */

#pragma once

#include <string>
#include <any>

namespace core {

    class Application;
    class DTNHost;

    /**
     * @class ApplicationListener
     * @brief Interface for classes that want to receive application events.
     * @details Applications can broadcast events to all attached listeners 
     *          to notify the simulation engine or GUI about specific occurrences.
     */
    class ApplicationListener {
    public:
        /**
         * @brief Virtual destructor to ensure safe inheritance and memory cleanup.
         */
        virtual ~ApplicationListener() = default;

        /**
         * @brief Invoked when an application broadcasts an event.
         * 
         * @param event  The string identifier of the event.
         * @param params Additional parameters sent with the event. (Uses std::any to accept any data type).
         * @param app    Pointer to the application that triggered the event.
         * @param host   Pointer to the host where the application is running.
         */
        virtual void gotEvent(const std::string& event, const std::any& params, Application* app, DTNHost* host) = 0;
    };

} // namespace core