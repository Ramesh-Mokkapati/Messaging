// MqttSubscriber.h
#pragma once

#include "Message.h"
#include "MqttConfig.h"
#include "MqttConnection.h"

#include <functional>

namespace mqttapp
{
    using MessageHandler = std::function<void(const Message&)>;

    class Subscriber
    {
    public:
        explicit Subscriber(const ConnectionConfig& config);

        // Connects, starts the client's internal consumer queue, and
        // subscribes to config's topic.
        void Start();

        // Blocks, invoking handler for each message received, until
        // Stop() is called from another thread (e.g. a signal handler).
        void Run(const MessageHandler& handler);

        // Unblocks a Run() in progress and disconnects.
        void Stop();

    private:
        Connection connection_;
    };

}  // namespace mqttapp
