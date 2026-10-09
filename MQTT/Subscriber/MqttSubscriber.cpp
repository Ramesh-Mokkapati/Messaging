// MqttSubscriber.cpp
#include "MqttSubscriber.h"

#include <iostream>

namespace mqttapp
{

    Subscriber::Subscriber(const ConnectionConfig& config)
        : connection_(config, config.clientIdSubscriber)
    {
    }

    void Subscriber::Start()
    {
        connection_.Connect();
        // Buffers incoming messages internally so consume_message() below
        // can pull them off one at a time.
        connection_.Client().start_consuming();
        connection_.Client().subscribe(connection_.Config().topic,
            connection_.Config().qos);
    }

    void Subscriber::Run(const MessageHandler& handler)
    {
        while (true)
        {
            mqtt::const_message_ptr msg = connection_.Client().consume_message();
            if (!msg)
            {
                // stop_consuming() (called from Stop()) unblocks
                // consume_message() by handing back a null pointer -- our
                // signal to leave the loop.
                break;
            }

            Message message(msg->get_topic(), msg->to_string());
            message.SetQos(msg->get_qos());
            message.SetRetained(msg->is_retained());

            try
            {
                handler(message);
            }
            catch (const std::exception& e)
            {
                // Unlike AMQP, MQTT has no broker-side redelivery/nack once
                // the client library has acked receipt (which it already has
                // by the time we get here at QoS 1/2) -- so there's nothing
                // further to do besides logging.
                std::cerr << "Handler threw while processing message: " << e.what()
                    << std::endl;
            }
        }
    }

    void Subscriber::Stop()
    {
        connection_.Client().stop_consuming();
        connection_.Disconnect();
    }

}  // namespace mqttapp
