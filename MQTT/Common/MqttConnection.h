// MqttConnection.h
#pragma once

#include "MqttConfig.h"

#include <mqtt/client.h>

namespace mqttapp
{

    // Thin wrapper around mqtt::client: constructs it with a given
    // client id (publisher and subscriber must each use a distinct one),
    // connects with the shared keep-alive/clean-session options from
    // ConnectionConfig, and disconnects on destruction if still
    // connected.
    class Connection
    {
    public:
        Connection(const ConnectionConfig& config, const std::string& clientId);
        ~Connection();

        void Connect();
        void Disconnect();

        mqtt::client& Client() { return client_; }
        const ConnectionConfig& Config() const { return config_; }

    private:
        ConnectionConfig config_;
        mqtt::client client_;
    };

}  // namespace mqttapp
