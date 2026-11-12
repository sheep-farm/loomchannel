#pragma once

/**
 * @file ordered_channel.hpp
 * @brief Channel that guarantees in-order delivery even with multiple senders.
 *
 * Unlike DefaultChannel, OrderedChannel serializes every publication into an
 * internal queue and delivers messages strictly in arrival order. Multiple
 * threads may call set_content() concurrently; the mutex determines the
 * arrival order and all receivers observe the same sequence.
 */

#include <any>
#include <memory>
#include <mutex>
#include <queue>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "loomchannel/channel.hpp"
#include "loomchannel/constraint.hpp"
#include "loomchannel/message_event.hpp"
#include "loomchannel/subscriber.hpp"

namespace loomchannel {

class OrderedChannel : public Channel {
public:
    /**
     * @param channel_id channel identifier.
     * @param sender opaque pointer to the sender (who is speaking).
     */
    OrderedChannel(std::string channel_id, void* sender);
    ~OrderedChannel() override;

    // ---- Channel ----
    const std::string& id() const override;
    void* sender() const override;

    Channel& set_open(bool open) override;
    bool is_open() const override;

    Channel& set_content(std::any content) override;
    std::any content() const override;

    void broadcast() override;

    void add_subscriber(std::shared_ptr<Subscriber> subscriber) override;
    void add_subscriber(std::shared_ptr<Subscriber> subscriber,
                        std::vector<std::shared_ptr<Constraint>> constraints) override;
    void remove_subscriber(std::shared_ptr<Subscriber> subscriber) override;
    void reset() override;
    std::vector<std::shared_ptr<Subscriber>> get_subscribers() const override;

    /**
     * @brief Processes all queued messages in arrival order.
     * Called automatically by set_content/set_open/broadcast when no
     * delivery is in progress.
     */
    void process_pending();

private:
    struct QueuedMessage {
        MessageEvent event;
        std::any payload;
    };

    void* sender_;
    std::any content_;
    bool open_;
    std::string id_;

    mutable std::mutex mutex_;
    std::vector<std::shared_ptr<Subscriber>> subscribers_;
    std::unordered_map<std::shared_ptr<Subscriber>,
                       std::vector<std::shared_ptr<Constraint>>> constraints_by_subscriber_;

    std::queue<QueuedMessage> queue_;
    bool delivering_ = false;

    bool passes_constraints(std::shared_ptr<Subscriber> subscriber,
                            MessageEvent event) const;

    void notify_subscriber(std::shared_ptr<Subscriber> subscriber,
                           MessageEvent event);

    /**
     * @brief Drains the internal queue delivering each message to every
     * subscriber. Must be called while holding mutex_.
     */
    void deliver_queued();
};

} // namespace loomchannel
