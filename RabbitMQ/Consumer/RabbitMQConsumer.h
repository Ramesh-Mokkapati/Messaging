// RabbitMQConsumer.h
#pragma once

#include "Message.h"
#include "RabbitMQConfig.h"
#include "RabbitMQConnection.h"

#include <atomic>
#include <functional>
#include <string>

namespace rmq
{
    using MessageHandler = std::function<void(const Message&)>;

    class Consumer
    {
    public:
        explicit Consumer(ConnectionConfig config);

        // Declares topology and registers as a consumer of config's queue.
        void Start();

        // Blocks, invoking handler for each message received (acking on
        // success, rejecting-with-requeue if handler throws), until Stop()
        // is called from another thread (e.g. a signal handler).
        void Run(const MessageHandler& handler);

        void Stop();

    private:
        Connection connection_;
        std::string consumerTag_;
        std::atomic<bool> running_{ false };
    };

}  // namespace rmq
