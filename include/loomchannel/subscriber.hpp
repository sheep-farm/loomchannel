#pragma once

/**
 * @file subscriber.hpp
 * @brief Subscriber interface and lambda adapter for LoomChannel.
 */

#include <any>
#include <functional>
#include <string>

#include "loomchannel/message_event.hpp"

namespace loomchannel {

/**
 * @brief Contract for a message receiver.
 *
 * Implementations receive notifications through `on_message`.
 * The payload is `std::any`, allowing any
 * data type to be transported through the channel without coupling sender
 * and receiver to a common type at compile time.
 */
class Subscriber {
public:
    virtual ~Subscriber() = default;

    /**
     * @param sender pointer to the sender (may be nullptr).
     * @param event event type (Stop or Broadcast).
     * @param payload message data. Valid only for the duration of this call.
     * Do not store the reference; copy explicitly if you need to retain it.
     */
    virtual void on_message(void* sender,
                            MessageEvent event,
                            const std::any& payload) = 0;
};

/**
 * @brief Adapter that wraps a std::function as a Subscriber.
 * Useful for lambdas without creating concrete classes.
 */
class FunctionSubscriber : public Subscriber {
public:
    using Callback = std::function<void(void*, MessageEvent, const std::any&)>;

    explicit FunctionSubscriber(Callback cb) : cb_(std::move(cb)) {}

    void on_message(void* sender,
                    MessageEvent event,
                    const std::any& payload) override {
        cb_(sender, event, payload);
    }

private:
    Callback cb_;
};

/**
 * @brief Creates a subscriber from a lambda.
 */
inline std::shared_ptr<Subscriber> make_subscriber(
    std::function<void(void*, MessageEvent, const std::any&)> cb) {
    return std::make_shared<FunctionSubscriber>(std::move(cb));
}

} // namespace loomchannel
