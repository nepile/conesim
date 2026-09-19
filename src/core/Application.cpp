/**
 * @file Application.cpp
 * @brief Implementation of the base Application class.
 * @author mjpitka, teemuk (Original Java) / Frathol & Team (C++ Port)
 * @date September 2026
 */

#include "core/Application.hpp"
#include "core/DTNHost.hpp"
#include "core/Message.hpp"
#include "core/ApplicationListener.hpp"

namespace core {

    Application::Application() : appID("") {
        // Default constructor initializes an empty app ID and an empty listener list
    }

    Application::Application(const Application& app) {
        this->aListeners = app.getAppListeners();
        this->appID = app.appID;
    }

    std::string Application::getAppID() const {
        return this->appID;
    }

    void Application::setAppID(const std::string& appID) {
        this->appID = appID;
    }

    void Application::setAppListeners(const std::vector<std::shared_ptr<ApplicationListener>>& listeners) {
        this->aListeners = listeners;
    }

    std::vector<std::shared_ptr<ApplicationListener>> Application::getAppListeners() const {
        return this->aListeners;
    }

    void Application::sendEventToListeners(const std::string& event, const std::any& params, DTNHost* host) {
        for (auto& listener : this->aListeners) {
            if (listener) {
                listener->gotEvent(event, params, this, host);
            }
        }
    }

} // namespace core