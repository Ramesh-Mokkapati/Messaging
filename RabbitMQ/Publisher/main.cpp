// main.cpp (Publisher)
#include "RabbitMQPublisher.h"

#include <chrono>
#include <iostream>
#include <string>
#include <thread>

int main()
{
    rmq::ConnectionConfig config;
    // Defaults (localhost:5672, guest/guest, vhost "/") match a local
    // RabbitMQ broker started with default settings. Override any
    // field here (or read from argv/env) to point elsewhere.

    try
    {
        rmq::Publisher publisher(config);
        publisher.Start();

        std::cout << "Publisher: connected to " << config.host << ":"
            << config.port << ", sending 10 messages..." << std::endl;

        for (int i = 0; i < 10; ++i)
        {
            rmq::Message message("Hello from C++ publisher, message #" +
                std::to_string(i));
            publisher.Publish(message);
            std::cout << "Sent: " << message.Body() << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }

        std::cout << "Publisher: done." << std::endl;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Publisher error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
