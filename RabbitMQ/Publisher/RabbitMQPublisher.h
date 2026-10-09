// RabbitMQPublisher.h
#pragma once

#include "Message.h"
#include "RabbitMQConfig.h"
#include "RabbitMQConnection.h"

namespace rmq
{
    class Publisher
    {
    public:
        explicit Publisher(ConnectionConfig config);

        // Declares the exchange/queue/binding. Call once before Publish().
        void Start();

        void Publish(const Message& message);

    private:
        Connection connection_;
    };

}  // namespace rmq
