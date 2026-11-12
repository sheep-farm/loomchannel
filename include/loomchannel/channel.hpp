#pragma once

/**
 * @file channel.hpp
 * @brief Named communication channel interface and default implementation.
 */

#include <any>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "loomchannel/constraint.hpp"
#include "loomchannel/message_event.hpp"
#include "loomchannel/subscriber.hpp"

namespace loomchannel {

/**
 * @brief Interface of a named communication channel.
 *
 * A channel carries:
 *  - a unique identifier (string),
 *  - a sender (opaque pointer),
 *  - current content (std::any),
 *  - open/closed state,
 *  - list of subscribed subscribers,
 *  - per-subscriber constraints.
 */
class Channel {
public:
    virtual ~Channel() = default;

    /** Unique channel identifier on the bus. */
    virtual const std::string& id() const = 0;

    /** Pointer to the sender associated with this channel. */
    virtual void* sender() const = 0;

    /** Opens or closes the channel. Opening broadcasts the current content. */
    virtual Channel& set_open(bool open) = 0;
    virtual bool is_open() const = 0;

    /** Sets the channel content and, if open, broadcasts it. */
    virtual Channel& set_content(std::any content) = 0;
    virtual std::any content() const = 0;

    /** Delivers the current content to all subscribed subscribers. */
    virtual void broadcast() = 0;

    /** Subscribes a subscriber (no constraints). */
    virtual void add_subscriber(std::shared_ptr<Subscriber> subscriber) = 0;

    /** Subscribes a subscriber with optional constraints. */
    virtual void add_subscriber(std::shared_ptr<Subscriber> subscriber,
                                std::vector<std::shared_ptr<Constraint>> constraints) = 0;

    /** Removes a subscriber. */
    virtual void remove_subscriber(std::shared_ptr<Subscriber> subscriber) = 0;

    /** Removes all subscribers and notifies Stop. */
    virtual void reset() = 0;

    /** Returns a copy of the current subscriber list. */
    virtual std::vector<std::shared_ptr<Subscriber>> get_subscribers() const = 0;
};

/**
 * @brief Default thread-safe Channel implementation.
 *
 * Uses `std::mutex` to protect subscriber lists and takes a snapshot before
 * notifying, avoiding deadlocks if a subscriber unsubscribes from within its
 * own callback.
 */
class DefaultChannel : public Channel {
public:
    /**
     * @param channel_id channel identifier.
     * @param sender opaque pointer to the sender (who is speaking).
     */
    DefaultChannel(std::string channel_id, void* sender);

    ~DefaultChannel() override;

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

protected:
    void* sender_;
    std::any content_;
    bool open_;
    std::string id_;

    mutable std::mutex mutex_;
    std::vector<std::shared_ptr<Subscriber>> subscribers_;
    std::unordered_map<std::shared_ptr<Subscriber>,
                       std::vector<std::shared_ptr<Constraint>>> constraints_by_subscriber_;

private:
    bool passes_constraints(std::shared_ptr<Subscriber> subscriber,
                            MessageEvent event) const;

    void notify_subscriber(std::shared_ptr<Subscriber> subscriber,
                           MessageEvent event);
};

} // namespace loomchannel
