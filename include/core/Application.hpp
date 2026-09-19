/**
 * @file Application.hpp
 * @brief Header definition for the base Application class in the ONE Sim.
 * @author mjpitka, teemuk (Original Java) / Frathol
 * @date September 2026
 * 
 * Copyright 2010 Aalto University, ComNet
 * Released under GPLv3. See LICENSE.txt for details. 
 */

#pragma once

#include <string>
#include <vector>
#include <memory>
#include <any>

namespace core {

    // Forward declarations to avoid circular dependencies
    class DTNHost;
    class Message;
    class ApplicationListener;

    /**
     * @class Application
     * @brief Base class for applications. 
     * 
     * Nodes that have an application running will forward all incoming messages 
     * to the application's handle() method before they are processed further. 
     * The application can change the properties of the message before returning it, 
     * or return nullptr to signal to the router that it wants the message to be dropped.
     * 
     * In addition, the application's update() method is called every simulation cycle.
     */
    class Application {
    protected:
        std::vector<std::shared_ptr<ApplicationListener>> aListeners; ///< List of event listeners
        std::string appID;                                            ///< Unique application identifier

    public:
        /**
         * @brief Default constructor.
         */
        Application();

        /**
         * @brief Copy constructor.
         * @param app The application prototype to copy listeners and ID from.
         */
        Application(const Application& app);

        /**
         * @brief Virtual destructor to ensure proper cleanup of derived classes.
         */
        virtual ~Application() = default;

        /**
         * @brief Handles application functionality related to processing of the bundle.
         * 
         * Application handles a message which arrives at the node hosting this application. 
         * After performing application-specific handling, this method returns the message. 
         * 
         * @param msg The incoming message.
         * @param host The host this application instance is attached to.
         * @return The (possibly modified) message to forward, or nullptr if the application 
         *         wants the router to stop forwarding and drop the message.
         */
        virtual std::shared_ptr<Message> handle(std::shared_ptr<Message> msg, DTNHost* host) = 0;

        /**
         * @brief Called every simulation cycle.
         * @param host The host this application instance is attached to.
         */
        virtual void update(DTNHost* host) = 0;

        /**
         * @brief Returns the unique application ID.
         * 
         * The application will only receive messages with this application ID. 
         * If the AppID is empty, the application will receive all messages.
         * 
         * @return The Application ID string.
         */
        std::string getAppID() const;

        /**
         * @brief Sets the application ID.
         * 
         * Should only be set once when the application is created. Changing the value 
         * during simulation runtime is not recommended unless explicitly required.
         * 
         * @param appID The new application ID to set.
         */
        void setAppID(const std::string& appID);

        /**
         * @brief Replicates this application instance.
         * @return A shared pointer to the newly created replica.
         */
        virtual std::shared_ptr<Application> replicate() const = 0;

        /**
         * @brief Binds a list of listeners to this application.
         * @param listeners Vector of shared pointers to ApplicationListeners.
         */
        void setAppListeners(const std::vector<std::shared_ptr<ApplicationListener>>& listeners);

        /**
         * @brief Retrieves the current list of listeners bound to this application.
         * @return Vector of shared pointers to ApplicationListeners.
         */
        std::vector<std::shared_ptr<ApplicationListener>> getAppListeners() const;

        /**
         * @brief Sends an event notification to all attached listeners.
         * 
         * @param event The string identifying the event type.
         * @param params Any additional parameters to send (uses std::any for generic types).
         * @param host The host where the app is currently running.
         */
        void sendEventToListeners(const std::string& event, const std::any& params, DTNHost* host);
    };

} // namespace core