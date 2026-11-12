#pragma once

/**
 * @file connection.hpp
 * @brief RAII handle for a channel subscription.
 *
 * When destroyed (or when `disconnect()` is called), automatically cancels
 * the subscriber's registration on the channel. Avoids leaks and forgotten
 * `unsubscribe` calls.
 *
 * Example:
 *   auto conn = bus.subscribe("channel", subscriber);
 *   // conn goes out of scope -> unsubscribes automatically
 */

#include <any>
#include <memory>
#include <string>
#include <utility>

#include "loomchannel/bus.hpp"
#include "loomchannel/channel.hpp"
#include "loomchannel/constraint.hpp"
#include "loomchannel/message_event.hpp"
#include "loomchannel/subscriber.hpp"

namespace loomchannel {

class Bus;

class Connection {
public:
    Connection() = default;

    Connection(std::shared_ptr<Bus> bus,
               std::string channel_id,
               std::shared_ptr<Subscriber> subscriber);

    Connection(const Connection&) = delete;
    Connection& operator=(const Connection&) = delete;

    Connection(Connection&& other) noexcept;
    Connection& operator=(Connection&& other) noexcept;

    ~Connection();

    /** Manually cancels the subscription (idempotent). */
    void disconnect();

    /** Returns true while the subscription is active. */
    bool connected() const { return subscriber_ != nullptr; }

    const std::string& channel_id() const { return channel_id_; }

private:
    std::shared_ptr<Bus> bus_;
    std::string channel_id_;
    std::shared_ptr<Subscriber> subscriber_;
};

} // namespace loomchannel
