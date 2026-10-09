# C++ Messaging Samples — Visual Studio 2022

Three self-contained pub/sub samples covering different messaging
protocols, all in C++17, all targeting x64 Windows. A single combined
solution (`Messaging.sln`) builds all nine projects; each sub-directory
also has its own solution for working on one protocol at a time.

| Sub-project | Protocol | Library | Broker needed |
|---|---|---|---|
| `MQTT/` | MQTT | Eclipse Paho C++ (`paho-mqttpp3`) | Mosquitto / EMQX |
| `RabbitMQ/` | AMQP 0-9-1 | SimpleAmqpClient | RabbitMQ |
| `OpenDDS/` | DDS / RTPS | OpenDDS 3.x (ACE/TAO) | none — peer-to-peer |

## Repository layout

```
Messaging.sln          ← combined solution (all 9 projects)
vcpkg.json             ← vcpkg manifest (paho-mqttpp3 + simple-amqp-client)
vcpkg/                 ← vcpkg bootstrap clone (gitignored, auto-created)
MQTT/
├── MQTT_PubSub.sln
├── MQTT.props
├── Common/            ← MQTTCommon.lib  (ConnectionConfig, Message, Connection)
├── Publisher/         ← MQTTPublisher.exe
└── Subscriber/        ← MQTTSubscriber.exe
RabbitMQ/
├── RabbitMQ_PubSub.sln
├── RabbitMQ.props
├── Common/            ← RabbitMQCommon.lib  (ConnectionConfig, Message, Connection)
├── Publisher/         ← RabbitMQPublisher.exe
└── Consumer/          ← RabbitMQConsumer.exe
OpenDDS/
├── OpenDDS_PubSub_Cxx11.sln
├── OpenDDS.props
├── setup-opendds.bat  ← first-time clone + cmake build of OpenDDS (auto-run)
├── rtps.ini           ← DDS RTPS discovery config (copied to output dir)
├── Common/            ← MessengerTypesLib.lib  (IDL-generated type support)
├── Publisher/         ← OpenDDSPublisher.exe
└── Subscriber/        ← OpenDDSSubscriber.exe
bin/                   ← compiled executables + runtime DLLs  (gitignored)
lib/                   ← static libs                          (gitignored)
obj/                   ← MSBuild intermediates                (gitignored)
```

## Prerequisites

**Required for all sub-projects:**
- Visual Studio 2022 (Community edition is fine) with the
  **Desktop development with C++** workload
- Git for Windows (on `PATH`)

**MQTT and RabbitMQ** — dependencies are installed automatically from
`vcpkg.json` on first build. No extra steps.

**OpenDDS** — the first build clones OpenDDS, downloads ACE/TAO, and
compiles everything (~30-90 minutes, one-time). Requires:
- CMake (included with VS 2022 via *C++ CMake tools for Windows*
  individual component, or on `PATH`)
- Perl (auto-downloaded as portable Strawberry Perl if not found)

**Brokers** (MQTT and RabbitMQ only — OpenDDS uses peer-to-peer RTPS):
```bat
# MQTT
docker run -it --rm -p 1883:1883 eclipse-mosquitto:2 mosquitto -c /mosquitto-no-auth.conf

# RabbitMQ  (management UI at http://localhost:15672, guest/guest)
docker run -it --rm -p 5672:5672 -p 15672:15672 rabbitmq:3-management
```

## Build

Open **`Messaging.sln`** (or a sub-directory's own `.sln`) in Visual
Studio 2022, select **x64 | Debug** (or Release), then
**Build → Build Solution** (`Ctrl+Shift+B`).

> **OpenDDS first build:** `setup-opendds.bat` runs automatically as a
> pre-build step and compiles OpenDDS from source. The build window will
> appear to hang for 30-90 minutes — this is normal. Subsequent builds
> skip this step instantly.

Output binaries land in `bin\x64\Debug\` (or `Release\`). The
post-build step copies all required runtime DLLs and any config files
(e.g. `rtps.ini`) there automatically, so you can run the executables
directly from that directory.

## Run

### MQTT

```bat
cd bin\x64\Debug
MQTTSubscriber.exe
MQTTPublisher.exe
```

Publisher sends 10 messages and exits. Subscriber prints each one and
keeps running until Ctrl+C. Start the subscriber first.

### RabbitMQ

```bat
cd bin\x64\Debug
RabbitMQConsumer.exe
RabbitMQPublisher.exe
```

Same pattern — consumer first, then publisher. RabbitMQ queues messages
server-side, so a few seconds' gap between them is fine.

### OpenDDS

```bat
cd bin\x64\Debug
OpenDDSSubscriber.exe -DCPSConfigFile rtps.ini
OpenDDSPublisher.exe  -DCPSConfigFile rtps.ini
```

Or use **Debug → Multiple Startup Projects** in VS to launch both at
once. No broker process needed — discovery and transport are peer-to-peer
via RTPS (`rtps.ini`).

## Per-sub-project details

See each sub-directory's own README for protocol-specific notes,
configuration options, and what to change for real use:

- [`MQTT/README.md`](MQTT/README.md)
- [`RabbitMQ/README.md`](RabbitMQ/README.md)
- [`OpenDDS/README.md`](OpenDDS/README.md)
