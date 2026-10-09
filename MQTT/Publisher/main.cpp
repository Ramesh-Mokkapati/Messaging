// main.cpp (Publisher)
#include "MqttPublisher.h"

#include <chrono>
#include <iostream>
#include <string>
#include <thread>

int main()
{
    mqttapp::ConnectionConfig config;
    // Defaults (tcp://localhost:1883, QoS 1) match a local broker
    // (e.g. Mosquitto or EMQX) started with default settings.

    try
    {
        mqttapp::Publisher publisher(config);
        publisher.Start();

        std::cout << "Publisher: connected to " << config.serverAddress
            << ", sending 10 messages on \"" << config.topic << "\"..."
            << std::endl;

        for (int i = 0; i < 10; ++i)
        {
            mqttapp::Message message(config.topic, "Hello from C++ publisher, message #" + std::to_string(i));
            message.SetQos(config.qos);
            publisher.Publish(message);
            std::cout << "Sent: " << message.Payload() << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }

        publisher.Stop();
        std::cout << "Publisher: done." << std::endl;
    }
    catch (const mqtt::exception& e)
    {
        std::cerr << "Publisher MQTT error: " << e.what() << std::endl;
        return 1;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Publisher error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
