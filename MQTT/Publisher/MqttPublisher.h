// MqttPublisher.h
#pragma once

#include "Message.h"
#include "MqttConfig.h"
#include "MqttConnection.h"

namespace mqttapp
{
    class Publisher
    {
    public:
        explicit Publisher(const ConnectionConfig& config);

        void Start();
        void Publish(const Message& message);
        void Stop();

    private:
        Connection connection_;
    };

}  // namespace mqttapp
