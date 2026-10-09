// RabbitMQConnection.cpp
#include "RabbitMQConnection.h"

#include <stdexcept>
#include <utility>

namespace rmq
{
    Connection::Connection(ConnectionConfig config) : config_(std::move(config))
    {
        channel_ = AmqpClient::Channel::Create(config_.host, config_.port,
            config_.username, config_.password,
            config_.vhost);
        if (!channel_)
        {
            throw std::runtime_error("Failed to create AMQP channel to " +
                config_.host);
        }
    }

    void Connection::DeclareTopology()
    {
        channel_->DeclareExchange(config_.exchange, config_.exchangeType,
            /*passive=*/false, config_.durableExchange,
            /*auto_delete=*/false);

        channel_->DeclareQueue(config_.queue, /*passive=*/false,
            config_.durableQueue, /*exclusive=*/false,
            /*auto_delete=*/false);

        channel_->BindQueue(config_.queue, config_.exchange, config_.routingKey);
    }

}  // namespace rmq
