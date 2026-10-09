// main.cpp (Consumer)
#include "RabbitMQConsumer.h"

#include <atomic>
#include <csignal>
#include <iostream>

namespace
{
    std::atomic<rmq::Consumer*> g_consumer{ nullptr };

    void OnSigInt(int /*signal*/)
    {
        if (auto* consumer = g_consumer.load())
        {
            consumer->Stop();
        }
    }
}  // namespace

int main()
{
    rmq::ConnectionConfig config;
    // Defaults (localhost:5672, guest/guest, vhost "/") match a local
    // RabbitMQ broker started with default settings.

    try
    {
        rmq::Consumer consumer(config);
        g_consumer = &consumer;
        std::signal(SIGINT, OnSigInt);

        consumer.Start();
        std::cout << "Consumer: waiting for messages on \"" << config.queue
            << "\" (Ctrl+C to stop)..." << std::endl;

        consumer.Run([](const rmq::Message& message) {
            std::cout << "Received: " << message.Body() << std::endl;
        });

        std::cout << "Consumer: stopped." << std::endl;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Consumer error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
