// RabbitMQConsumer.cpp
#include "RabbitMQConsumer.h"

#include <iostream>
#include <utility>

namespace rmq
{
    Consumer::Consumer(ConnectionConfig config) : connection_(std::move(config)) {}

    void Consumer::Start()
    {
        connection_.DeclareTopology();

        consumerTag_ = connection_.Channel()->BasicConsume(
            connection_.Config().queue,
            /*consumer_tag=*/"",  // let the broker generate one
            /*no_local=*/true,
            /*no_ack=*/false,  // we ack manually in Run()
            /*exclusive=*/false,
            /*message_prefetch_count=*/1);
    }

    void Consumer::Run(const MessageHandler& handler)
    {
        running_ = true;
        while (running_)
        {
            AmqpClient::Envelope::ptr_t envelope;
            // Poll with a timeout rather than blocking forever so Stop()
            // (set from another thread) takes effect promptly.
            if (connection_.Channel()->BasicConsumeMessage(consumerTag_, envelope,
                /*timeout_ms=*/1000))
            {
                Message message(envelope->Message()->Body());
                message.SetContentType(envelope->Message()->ContentType());

                try
                {
                    handler(message);
                    connection_.Channel()->BasicAck(envelope);
                }
                catch (const std::exception& e)
                {
                    std::cerr << "Handler threw, rejecting (requeue): " << e.what()
                        << std::endl;
                    connection_.Channel()->BasicReject(envelope, /*requeue=*/true);
                }
            }
        }
    }

    void Consumer::Stop()
    {
        running_ = false;
    }

}  // namespace rmq
