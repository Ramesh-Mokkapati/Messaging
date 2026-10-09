// Message.h
#pragma once

#include <string>
#include <utility>

namespace rmq
{
    // A minimal AMQP message: a body plus the few properties this sample
    // cares about. Extend with headers/correlation-id/etc. as needed.
    class Message
    {
    public:
        Message() = default;
        explicit Message(std::string body) : body_(std::move(body)) {}

        const std::string& Body() const { return body_; }
        void SetBody(std::string body) { body_ = std::move(body); }

        const std::string& ContentType() const { return contentType_; }
        void SetContentType(std::string contentType)
        {
            contentType_ = std::move(contentType);
        }

        // Whether the broker should persist this message to disk (survive
        // a broker restart) — only meaningful when the queue is durable.
        bool Persistent() const { return persistent_; }
        void SetPersistent(bool persistent) { persistent_ = persistent; }

    private:
        std::string body_;
        std::string contentType_ = "text/plain";
        bool persistent_ = true;
    };

}  // namespace rmq
