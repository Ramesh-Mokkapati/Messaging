// MqttPublisher.cpp
#include "MqttPublisher.h"

namespace mqttapp
{

    Publisher::Publisher(const ConnectionConfig& config)
        : connection_(config, config.clientIdPublisher)
    {
    }

    void Publisher::Start()
    {
        connection_.Connect();
    }

    void Publisher::Publish(const Message& message)
    {
        auto pubmsg = mqtt::make_message(message.Topic(), message.Payload(),
            message.Qos(), message.Retained());
        connection_.Client().publish(pubmsg);
    }

    void Publisher::Stop()
    {
        connection_.Disconnect();
    }

}  // namespace mqttapp
