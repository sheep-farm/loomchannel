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
- **Thread-safe:** `std::mutex` plus snapshot of subscribers before notifying (no deadlock in callbacks). Subscriber callbacks may safely re-enter the `Bus` (e.g. call `Bus::instance().channel(...)`) because notifications are delivered outside the Bus lock.
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

The payload is delivered as `const std::any&` pointing to the **channel's own content storage**. The channel keeps the content alive for as long as the channel exists (typically the whole session, while the channel is open on the `Bus`).

Two practical consequences:

1. **Retaining the reference is safe.** A receiver may hold the `const std::any&` beyond the callback (e.g. capture it in a deferred UI callback such as wxWidgets `CallAfter` or a Qt queued invocation). As long as the channel object is alive, the reference stays valid.
2. **The reference always reflects the latest published state.** `set_content()` replaces the value *in place*, so a retained reference read later returns the newest state, not a snapshot of the moment it was delivered. This is intentional: transmitters publish current state at all times, and receivers treat both fresh and stale states as valid (e.g. a `SqlConnection` channel always carries the current connection state).

If you need to **freeze the value at the moment of delivery** (or hand it to another thread), copy it explicitly:

```cpp
void on_message(void* sender, MessageEvent event, const std::any& payload) override {
    // Copy when the value must be frozen at delivery time or cross threads.
    CallAfter([payload = std::any(payload)]() { ... });
}
```

**Exception — `OrderedChannel`:** queued messages are delivered as private per-message snapshots (strict ordering requires independent payloads). A retained reference to an `OrderedChannel` payload is only valid for the duration of the callback; copy it if you need it later.

This model is intentional: the **channel owns the state**, transmitters publish it continuously, and receivers are observers of the current state.

## License

MIT License. See `LICENSE` file for details.
