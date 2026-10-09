// MqttConnection.cpp
#include "MqttConnection.h"

namespace mqttapp
{

    Connection::Connection(const ConnectionConfig& config, const std::string& clientId)
        : config_(config)
        , client_(config_.serverAddress, clientId)
    {
    }

    Connection::~Connection()
    {
        try
        {
            if (client_.is_connected())
            {
                client_.disconnect();
            }
        }
        catch (...)
        {
            // Best-effort cleanup; never throw out of a destructor.
        }
    }

    void Connection::Connect()
    {
        mqtt::connect_options connOpts;
        connOpts.set_clean_session(config_.cleanSession);
        connOpts.set_keep_alive_interval(config_.keepAliveIntervalSec);
        client_.connect(connOpts);
    }

    void Connection::Disconnect()
    {
        if (client_.is_connected())
        {
            client_.disconnect();
        }
    }

}  // namespace mqttapp
