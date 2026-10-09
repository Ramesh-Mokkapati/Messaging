// RabbitMQConnection.h
#pragma once

#include "RabbitMQConfig.h"

#include <SimpleAmqpClient/SimpleAmqpClient.h>

namespace rmq
{
    // Wraps channel creation and exchange/queue/binding declaration so
    // the publisher and the consumer set up *exactly* the same topology
    // from the same ConnectionConfig. Declaring is idempotent in AMQP
    // (re-declaring with identical arguments is a no-op), so it's safe
    // for both sides to call DeclareTopology().
    class Connection
    {
    public:
        explicit Connection(ConnectionConfig config);

        void DeclareTopology();

        AmqpClient::Channel::ptr_t Channel() const { return channel_; }
        const ConnectionConfig& Config() const { return config_; }

    private:
        ConnectionConfig config_;
        AmqpClient::Channel::ptr_t channel_;
    };

}  // namespace rmq
