// RabbitMQPublisher.cpp
#include "RabbitMQPublisher.h"

#include <utility>

namespace rmq
{
    Publisher::Publisher(ConnectionConfig config)
        : connection_(std::move(config))
    {
    }

    void Publisher::Start()
    {
        connection_.DeclareTopology();
    }

    void Publisher::Publish(const Message& message)
    {
        auto amqpMessage = AmqpClient::BasicMessage::Create(message.Body());
        amqpMessage->ContentType(message.ContentType());
        amqpMessage->DeliveryMode(message.Persistent()
            ? AmqpClient::BasicMessage::dm_persistent
            : AmqpClient::BasicMessage::dm_nonpersistent);

        connection_.Channel()->BasicPublish(connection_.Config().exchange,
            connection_.Config().routingKey,
            amqpMessage,
            /*mandatory=*/false);
    }

}  // namespace rmq
