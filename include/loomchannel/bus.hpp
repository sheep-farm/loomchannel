#pragma once

/**
 * @file bus.hpp
 * @brief Central message bus (Mediator / Pub-Sub).
 */

#include <any>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include "loomchannel/channel.hpp"
#include "loomchannel/connection.hpp"
#include "loomchannel/constraint.hpp"
#include "loomchannel/message_event.hpp"
#include "loomchannel/subscriber.hpp"

namespace loomchannel {

/**
 * @brief Central message bus (Mediator / Pub-Sub).
 *
 * Responsibilities:
 *  - Manage named channels identified by string (open / close / lookup).
 *  - Accept subscriber subscriptions to channels; if the channel does not
 *    yet exist, the subscriber becomes an "orphan" and is delivered to the
 *    channel as soon as it is opened.
 *  - Relay publications (set_content) to all subscribers of the channel.
 *
 * Thread-safe. All public operations are protected by a mutex;
 * notifications use a snapshot of the subscriber list to avoid deadlocks
 * and concurrency issues.
 *
 * Typical usage:
 *   auto& bus = Bus::instance();
 *   auto channel = std::make_shared<DefaultChannel>("db", this);
 *   bus.open("db", channel);
 *
 *   auto conn = bus.subscribe("db", my_subscriber);
 *   // or with lambda:
 *   auto conn2 = bus.subscribe("db",
 *       make_subscriber([](void* s, MessageEvent e, const std::any& p){ ... }));
 *
 *   channel->set_content(std::string("jdbc:...")); // broadcast
 */
class Bus {
public:
    Bus(const Bus&) = delete;
    Bus& operator=(const Bus&) = delete;

    /** Returns the singleton instance (Meyer's singleton). */
    static Bus& instance();

    /**
     * @brief Opens (or reopens) a channel.
     * Orphan subscribers accumulated for this channel are migrated to it.
     */
    void open(std::string channel_id, std::shared_ptr<Channel> channel);

    /**
     * @brief Closes a channel. Current subscribers are kept as orphans
     * for possible future reopening.
     */
    void close(std::string channel_id);

    /**
     * @brief Subscribes a subscriber to a channel.
     * Returns a `Connection` RAII handle that cancels the subscription when destroyed.
     */
    Connection subscribe(std::string channel_id,
                         std::shared_ptr<Subscriber> subscriber);

    Connection subscribe(std::string channel_id,
                         std::shared_ptr<Subscriber> subscriber,
                         std::vector<std::shared_ptr<Constraint>> constraints);

    /**
     * @brief Subscribes via lambda (convenience).
     */
    Connection subscribe(std::string channel_id,
                         std::function<void(void*, MessageEvent, const std::any&)> callback);

    Connection subscribe(std::string channel_id,
                         std::function<void(void*, MessageEvent, const std::any&)> callback,
                         std::vector<std::shared_ptr<Constraint>> constraints);

    /**
     * @brief Manually cancels a subscriber's subscription on a channel.
     */
    void unsubscribe(std::string channel_id,
                     std::shared_ptr<Subscriber> subscriber);

    /**
     * @brief Returns the open channel with the given ID, or nullptr if it does not exist.
     */
    std::shared_ptr<Channel> channel(std::string channel_id) const;

    /**
     * @brief Checks whether an open channel with the given ID exists.
     */
    bool has_channel(std::string channel_id) const;

private:
    Bus() = default;

    mutable std::mutex mutex_;
    std::unordered_map<std::string, std::shared_ptr<Channel>> open_channels_;
    // Subscribers that subscribed before the channel existed.
    std::unordered_map<std::string,
                       std::vector<std::shared_ptr<Subscriber>>> orphan_subscribers_;
    // Constraints associated with orphan subscribers (same order as the vector above).
    std::unordered_map<std::string,
                       std::vector<std::vector<std::shared_ptr<Constraint>>>> orphan_constraints_;

    void ensure_orphan_list(std::string channel_id);

    // Shared empty constraint list used when an orphan entry has no
    // associated constraints.
    const std::vector<std::vector<std::shared_ptr<Constraint>>> empty_constraints_;
};

} // namespace loomchannel
