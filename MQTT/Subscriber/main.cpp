// main.cpp (Subscriber)
#include "MqttSubscriber.h"

#include <atomic>
#include <csignal>
#include <iostream>

namespace
{
    std::atomic<mqttapp::Subscriber*> g_subscriber{ nullptr };

    void OnSigInt(int /*signal*/)
    {
        if (auto* subscriber = g_subscriber.load())
        {
            subscriber->Stop();
        }
    }
}  // namespace

int main()
{
    mqttapp::ConnectionConfig config;

    try
    {
        mqttapp::Subscriber subscriber(config);
        g_subscriber = &subscriber;
        std::signal(SIGINT, OnSigInt);

        subscriber.Start();
        std::cout << "Subscriber: connected to " << config.serverAddress
            << ", waiting for messages on \"" << config.topic
            << "\" (Ctrl+C to stop)..." << std::endl;

        subscriber.Run([](const mqttapp::Message& message) {
            std::cout << "Received on " << message.Topic() << ": "
                << message.Payload() << std::endl;
        });

        std::cout << "Subscriber: stopped." << std::endl;
    }
    catch (const mqtt::exception& e)
    {
        std::cerr << "Subscriber MQTT error: " << e.what() << std::endl;
        return 1;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Subscriber error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
