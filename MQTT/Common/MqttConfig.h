// MqttConfig.h
#pragma once

#include <string>

namespace mqttapp
{

    // Everything needed to connect to a broker and to publish/subscribe
    // on the same topic from both sides. MQTT client IDs must be unique
    // per connection, so publisher and subscriber each get their own.
    struct ConnectionConfig
    {
        std::string serverAddress = "tcp://localhost:1883";  // "ssl://host:8883" for TLS
        std::string clientIdPublisher = "cpp-publisher";
        std::string clientIdSubscriber = "cpp-subscriber";

        std::string topic = "messenger/topic";
        int qos = 1;  // 0 = at most once, 1 = at least once, 2 = exactly once

        bool cleanSession = true;
        int keepAliveIntervalSec = 20;
    };

}  // namespace mqttapp
