# RabbitMQ Pub/Sub — Visual Studio 2022

A minimal RabbitMQ publisher/consumer pair in C++, using
[SimpleAmqpClient](https://github.com/alanxz/SimpleAmqpClient) (a
thin C++ wrapper around the `rabbitmq-c` library) for the actual AMQP
protocol work. Three Visual Studio 2022 projects, one solution:

```
RabbitMQ_PubSub.sln
├── RabbitMQCommon   (static library)  Common\RabbitMQCommon.vcxproj
├── RabbitMQPublisher (console app)    Publisher\RabbitMQPublisher.vcxproj
└── RabbitMQConsumer  (console app)    Consumer\RabbitMQConsumer.vcxproj
```

`RabbitMQCommon` holds the classes shared by both sides — connection
setup and exchange/queue/binding declaration (`rmq::Connection`), the
config struct (`rmq::ConnectionConfig`), and the message value type
(`rmq::Message`) — so publisher and consumer can never declare
different topology for the same queue. `RabbitMQPublisher` and
`RabbitMQConsumer` each add their own class (`rmq::Publisher`,
`rmq::Consumer`) on top and reference `RabbitMQCommon` via
`<ProjectReference>`.

## Layout

```
RabbitMQ_PubSub_Cxx11/
├── RabbitMQ_PubSub.sln
├── RabbitMQ.props          <- shared compiler settings / optional vcpkg paths
├── Common/
│   ├── RabbitMQConfig.h     <- rmq::ConnectionConfig
│   ├── Message.h            <- rmq::Message
│   ├── RabbitMQConnection.h / .cpp   <- rmq::Connection
│   └── RabbitMQCommon.vcxproj
├── Publisher/
│   ├── RabbitMQPublisher.h / .cpp    <- rmq::Publisher
│   ├── main.cpp
│   └── RabbitMQPublisher.vcxproj
└── Consumer/
    ├── RabbitMQConsumer.h / .cpp     <- rmq::Consumer
    ├── main.cpp
    └── RabbitMQConsumer.vcxproj
```

## 1. Prerequisites

**A RabbitMQ broker.** Easiest is Docker:
```bat
docker run -it --rm -p 5672:5672 -p 15672:15672 rabbitmq:3-management
```
(management UI at http://localhost:15672, guest/guest). Otherwise
install RabbitMQ directly on Windows from rabbitmq.com.

**SimpleAmqpClient, via vcpkg** (handled automatically):

The repo root contains a `vcpkg.json` manifest and a `vcpkg/` clone
(gitignored, must be bootstrapped once):

```bat
# From the repo root — one-time setup
.\vcpkg\bootstrap-vcpkg.bat
.\vcpkg\vcpkg integrate install   # registers machine-wide MSBuild integration
```

After `integrate install`, packages listed in `vcpkg.json` (including
`simpleamqpclient` and its `rabbitmq-c` dependency) install
automatically on the first build. `RabbitMQ.props` also includes a
direct `VCPKG_ROOT`-based fallback so the project links correctly even
without `integrate install`.

## 2. Open and build

1. Open `RabbitMQ_PubSub.sln` in Visual Studio 2022 (Community
   edition is fine).
2. Select **x64** and **Debug** (or **Release**).
3. **Build → Build Solution** (Ctrl+Shift+B). `RabbitMQCommon`
   builds first; `RabbitMQPublisher` and `RabbitMQConsumer` link
   against it automatically via their project references.

Output binaries land in `bin\x64\Debug\` (or `Release\`) at the
solution root.

## 3. Run it

```bat
RabbitMQConsumer.exe
RabbitMQPublisher.exe
```
(from `bin\x64\Debug\`, or via **Debug → Start Without Debugging** in
Visual Studio, one project at a time — or set up **Multiple startup
projects** in Solution Properties as in the OpenDDS example, if you
built that one too).

Start the consumer first (or within a few seconds of the publisher —
unlike the OpenDDS sample, this one doesn't wait for a match, since
RabbitMQ queues messages for you). The publisher sends 10 messages
and exits; the consumer prints each one as it arrives and keeps
running until Ctrl+C.

## Notes / things you'll likely want to change

- **Connection defaults** (`localhost:5672`, `guest`/`guest`, vhost
  `/`) are set in `rmq::ConnectionConfig`'s member initializers
  (`Common/RabbitMQConfig.h`) and used as-is by both `main.cpp`
  files. Override fields there, or read them from `argv`/environment
  variables, to point at a non-local broker.
- **Exchange/queue/routing key** (`messenger.exchange` /
  `messenger.queue` / `messenger.key`) also live in
  `RabbitMQConfig.h` — both sides share this file, so they can't
  drift out of sync.
- **Acknowledgement**: the consumer acks each message only after
  your handler returns successfully, and rejects-with-requeue if the
  handler throws — so a crash mid-handler doesn't silently drop
  messages.
- **Delivery mode**: `Message::SetPersistent(false)` switches a
  message to non-persistent (faster, not saved to disk / lost on
  broker restart); the queue itself is still durable by default
  (`ConnectionConfig::durableQueue`).
- This is single-threaded and uses polling (`BasicConsumeMessage`
  with a 1s timeout) rather than a push-style callback — simple and
  easy to reason about, at the cost of up to ~1s added latency on
  `Stop()`. For lower latency or multiple consumers, look at
  `Channel::BasicConsume` combined with running each queue's poll
  loop on its own thread.
- If the linker reports unresolved AMQP symbols despite a successful
  vcpkg install, `RabbitMQ.props` already lists both `SimpleAmqpClient.7.lib`
  and `rabbitmq.4.lib` explicitly as a belt-and-braces fallback on top
  of `vcpkg integrate install`'s automatic linking.
