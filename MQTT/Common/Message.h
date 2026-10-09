// Message.h
#pragma once

#include <string>
#include <utility>

namespace mqttapp
{

    // A minimal MQTT message: topic, payload, and the couple of
    // properties this sample cares about.
    class Message
    {
    public:
        Message() = default;
        Message(std::string topic, std::string payload)
            : topic_(std::move(topic)), payload_(std::move(payload))
        {
        }

        const std::string& Topic() const { return topic_; }
        void SetTopic(std::string topic) { topic_ = std::move(topic); }

        const std::string& Payload() const { return payload_; }
        void SetPayload(std::string payload) { payload_ = std::move(payload); }

        int Qos() const { return qos_; }
        void SetQos(int qos) { qos_ = qos; }

        // Whether the broker should keep this as the topic's "last known
        // good" value for future subscribers.
        bool Retained() const { return retained_; }
        void SetRetained(bool retained) { retained_ = retained; }

    private:
        std::string topic_;
        std::string payload_;
        int qos_ = 1;
        bool retained_ = false;
    };

}  // namespace mqttapp
