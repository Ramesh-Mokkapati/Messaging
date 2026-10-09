# MQTT Pub/Sub — Visual Studio 2022

A minimal MQTT publisher/subscriber pair in C++, using
[Eclipse Paho MQTT C++](https://github.com/eclipse/paho.mqtt.cpp)
(`paho-mqttpp3`) for the protocol work. Same three-project pattern as
the OpenDDS and RabbitMQ samples:

```
MQTT_PubSub.sln
├── MQTTCommon     (static library)  Common\MQTTCommon.vcxproj
├── MQTTPublisher  (console app)     Publisher\MQTTPublisher.vcxproj
└── MQTTSubscriber (console app)     Subscriber\MQTTSubscriber.vcxproj
```

`MQTTCommon` holds the classes shared by both sides —
`mqttapp::ConnectionConfig` (broker address, topic, QoS), `mqttapp::Message`
(topic/payload/QoS/retained), and `mqttapp::Connection` (wraps
`mqtt::client` construction, connect, disconnect). `MQTTPublisher`
and `MQTTSubscriber` each add their own class (`mqttapp::Publisher`,
`mqttapp::Subscriber`) on top and reference `MQTTCommon` via
`<ProjectReference>`.

## Layout

```
MQTT_PubSub_Cxx11/
├── MQTT_PubSub.sln
├── MQTT.props            <- shared compiler settings / optional vcpkg paths
├── Common/
│   ├── MqttConfig.h        <- mqttapp::ConnectionConfig
│   ├── Message.h           <- mqttapp::Message
│   ├── MqttConnection.h / .cpp  <- mqttapp::Connection
│   └── MQTTCommon.vcxproj
├── Publisher/
│   ├── MqttPublisher.h / .cpp   <- mqttapp::Publisher
│   ├── main.cpp
│   └── MQTTPublisher.vcxproj
└── Subscriber/
    ├── MqttSubscriber.h / .cpp  <- mqttapp::Subscriber
    ├── main.cpp
    └── MQTTSubscriber.vcxproj
```

## 1. Prerequisites

**An MQTT broker.** Easiest is Docker with Eclipse Mosquitto:
```bat
docker run -it --rm -p 1883:1883 eclipse-mosquitto:2 mosquitto -c /mosquitto-no-auth.conf
```
(the stock image needs a config that allows anonymous connections —
the flag above does that for a quick local test; for anything beyond
that, use a real `mosquitto.conf`). Otherwise install Mosquitto or
EMQX directly on Windows.

**paho-mqttpp3, via vcpkg** (handled automatically):

The repo root contains a `vcpkg.json` manifest and a `vcpkg/` clone
(gitignored, must be bootstrapped once):

```bat
# From the repo root — one-time setup
.\vcpkg\bootstrap-vcpkg.bat
.\vcpkg\vcpkg integrate install   # registers machine-wide MSBuild integration
```

After `integrate install`, packages listed in `vcpkg.json` (including
`paho-mqttpp3` and its `paho-mqtt3a` C-library dependency) install
automatically on the first build. `MQTT.props` also includes a direct
`VCPKG_ROOT`-based fallback path so the project links correctly even
without `integrate install`.

## 2. Open and build

1. Open `MQTT_PubSub.sln` in Visual Studio 2022 (Community edition is
   fine).
2. Select **x64** and **Debug** (or **Release**).
3. **Build → Build Solution** (Ctrl+Shift+B). `MQTTCommon` builds
   first; `MQTTPublisher` and `MQTTSubscriber` link against it
   automatically via their project references.

Output binaries land in `bin\x64\Debug\` (or `Release\`) at the
solution root.

## 3. Run it

```bat
MQTTSubscriber.exe
MQTTPublisher.exe
```
(from `bin\x64\Debug\`, or via **Debug → Start Without Debugging** in
Visual Studio, one project at a time, or **Multiple startup
projects** in Solution Properties as in the other two samples).

Start the subscriber first (or within a few seconds of the publisher
if using QoS 0/1 without a persistent session — this sample uses a
clean session each run, so messages published before the subscriber
connects are not redelivered). The publisher sends 10 messages and
exits; the subscriber prints each one as it arrives and keeps running
until Ctrl+C.

## Notes / things you'll likely want to change

- **Broker address, topic, QoS** all live in `mqttapp::ConnectionConfig`
  (`Common/MqttConfig.h`), shared by both `main.cpp` files, so they
  can't drift out of sync. Defaults (`tcp://localhost:1883`, topic
  `messenger/topic`, QoS 1) match a local broker.
- **Client IDs must be unique per connection** — that's why
  `ConnectionConfig` has separate `clientIdPublisher` /
  `clientIdSubscriber` fields rather than one shared ID; connecting
  two clients with the same ID causes the broker to disconnect
  whichever connected first.
- **Clean session**: `cleanSession = true` (the default here) means
  the broker forgets the subscriber's subscriptions and any queued
  QoS 1/2 messages as soon as it disconnects. Set it to `false` (and
  give the subscriber a stable, unchanging client ID) if you want
  messages published while the subscriber is offline delivered once
  it reconnects.
- **No broker-side redelivery on handler failure**: unlike the
  RabbitMQ sample's `BasicReject(..., requeue=true)`, MQTT has no
  equivalent — by the time your handler runs, the client library has
  already acknowledged the message at QoS 1/2. `Subscriber::Run()`
  just logs if your handler throws.
- **Shutdown is push-based, not polled**: `Subscriber::Stop()` calls
  `stop_consuming()`, which immediately unblocks a pending
  `consume_message()` call in `Run()` — no timeout/poll loop needed
  (contrast with the RabbitMQ sample's 1-second polling loop).
- MSVC has no dedicated "C++11" `/std` switch — its earliest
  `/std:c++NN` option is `c++14` (a superset of C++11, and what Paho
  C++ itself requires at minimum), which is what `MQTT.props` sets.
- For TLS, switch `serverAddress` to `ssl://host:8883` and link
  `paho-mqtt3as` instead of `paho-mqtt3a` in `MQTT.props`; you'll also
  need to set `ssl_options` on `mqtt::connect_options` in
  `Connection::Connect()`.
