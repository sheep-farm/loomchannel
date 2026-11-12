# LoomChannel

Decoupled messaging library (Pub-Sub / Mediator) in **pure C++17**, with no
coupling to wxWidgets, Qt, GTK, or any GUI framework.

## Philosophy

The **sender does not know** who the receivers are.  
The **receiver does not need to know** whether the sender is active.

Four roles, fully separated:

| Role | Class | Responsibility |
|------|-------|----------------|
| Broker | `Bus` | Manages channels identified by string (`std::string`). |
| Channel | `Channel` / `DefaultChannel` | Carries content (`std::any`) and open/closed state. |
| Receiver | `Subscriber` | Receives `on_message(sender, event, const std::any& payload)`. |
| Filter | `Constraint` | Optional per-subscriber filter. |

## Key features

- **Zero GUI dependency:** compiles in any C++17 environment (console, server, embedded, etc.).
- **Thread-safe:** `std::mutex` plus snapshot of subscribers before notifying (no deadlock in callbacks).
- **Orphans:** a subscriber may subscribe to a channel that does not exist yet; it is delivered as soon as the channel is opened.
- **RAII:** `subscribe()` returns a `Connection` that automatically unsubscribes when it goes out of scope.
- **Generic payload, zero-copy delivery:** `std::any` allows transporting any type without coupling sender and receiver to a common type. The payload is delivered as `const std::any&`, avoiding copies during notification (similar to borrowing in Rust). The reference is valid only for the duration of `on_message`; copy explicitly if you need to retain it.
- **Constraints:** per-subscriber filters (e.g., deliver only if URL starts with `"jdbc:"`).
- **Lambdas:** `make_subscriber` and `make_constraint` avoid creating concrete classes for simple callbacks.
- **Ordered delivery:** `OrderedChannel` serializes concurrent publications into a queue, guaranteeing all receivers observe the same arrival order.

## Structure

```
loomchannel/
├── CMakeLists.txt
├── LICENSE
├── README.md
├── include/loomchannel/
│   ├── any_helpers.hpp    # safe std::any conversion
│   ├── bus.hpp            # central bus (optional singleton)
│   ├── channel.hpp        # Channel interface and DefaultChannel
│   ├── connection.hpp     # RAII subscription handle
│   ├── constraint.hpp     # optional per-subscriber filter
│   ├── message_event.hpp  # Stop / Broadcast enum
│   ├── ordered_channel.hpp# ordered delivery channel
│   └── subscriber.hpp     # Subscriber interface + lambda adapter
├── src/
│   ├── bus.cpp
│   ├── channel.cpp
│   └── ordered_channel.cpp
├── example/
│   └── main.cpp           # console example (no GUI)
└── tests/
```

## Build

```bash
cd /home/flavio/Work/loomchannel
mkdir build && cd build
cmake ..
make
./loomchannel_example
```

## Quick usage

```cpp
#include "loomchannel/bus.hpp"
#include "loomchannel/subscriber.hpp"

using namespace loomchannel;

constexpr char CHANNEL[] = "my.channel";

// 1. Sender creates and opens the channel
auto channel = std::make_shared<DefaultChannel>(CHANNEL, this);
Bus::instance().open(CHANNEL, channel);

// 2. Receiver subscribes (lambda). Payload is const std::any&: zero-copy.
//    Valid only during the call; copy if you need to retain it.
auto conn = Bus::instance().subscribe(
    CHANNEL,
    make_subscriber([](void* sender, MessageEvent ev, const std::any& payload) {
        if (ev == MessageEvent::Broadcast) {
            std::string text = any_cast_or<std::string>(payload, std::string{});
            std::cout << "Received: " << text << "\n";
        }
    })
);

// 3. Sender publishes (automatic broadcast)
channel->set_content(std::string("hello, world!"));

// 4. conn goes out of scope -> unsubscribes automatically
```

## Payload usage contract

The `std::any` delivered to receivers via `const std::any&` is valid **only for the duration of the `on_message` call**.

Receivers must consume the data immediately (e.g., `any_cast<T>`) and **must not retain the reference** after the callback returns. If you need to keep the value for later use, copy it explicitly:

```cpp
void on_message(void* sender, MessageEvent event, const std::any& payload) override {
    // Consume immediately; do not store &payload or payload itself beyond this call.
    std::string client_key = any_cast_or<std::string>(payload, std::string{});
    // use client_key here...
}
```

This model is intentional: the **sender owns the data**, the channel lends the reference during delivery, and receivers are temporary observers.

## License

MIT License. See `LICENSE` file for details.
