// RabbitMQConfig.h
#pragma once

#include <string>

namespace rmq
{

    // Everything needed to connect to a broker and to declare the exact
    // same exchange/queue/binding on both the publishing and consuming
    // side. Keeping this in one shared struct means the two sides can't
    // drift apart (e.g. one side using a different exchange type).
    struct ConnectionConfig
    {
        std::string host = "localhost";
        int port = 5672;
        std::string username = "guest";
        std::string password = "guest";
        std::string vhost = "/";

        std::string exchange = "messenger.exchange";
        std::string exchangeType = "direct";  // "direct", "fanout", "topic", ...
        std::string queue = "messenger.queue";
        std::string routingKey = "messenger.key";

        bool durableExchange = true;
        bool durableQueue = true;
    };

}  // namespace rmq
