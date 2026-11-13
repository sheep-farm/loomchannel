#include "loomchannel/ordered_channel.hpp"

#include <algorithm>
#include <iostream>

namespace loomchannel {

OrderedChannel::OrderedChannel(std::string channel_id, void* sender)
    : sender_(sender), content_(), open_(false), id_(std::move(channel_id)) {}

OrderedChannel::~OrderedChannel() = default;

const std::string& OrderedChannel::id() const { return id_; }

void* OrderedChannel::sender() const { return sender_; }

Channel& OrderedChannel::set_open(bool open) {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (open_ == open)
            return *this;

        open_ = open;

        if (open) {
            queue_.push({MessageEvent::Broadcast, content_});
        } else {
            queue_.push({MessageEvent::Stop, std::any{}});
        }
    }
    pump();
    return *this;
}

bool OrderedChannel::is_open() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return open_;
}

Channel& OrderedChannel::set_content(std::any content) {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        content_ = std::move(content);
        queue_.push({MessageEvent::Broadcast, content_});
    }
    pump();
    return *this;
}

std::any OrderedChannel::content() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return content_;
}

void OrderedChannel::broadcast() {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!open_)
            return;
        queue_.push({MessageEvent::Broadcast, content_});
    }
    pump();
}

void OrderedChannel::add_subscriber(std::shared_ptr<Subscriber> subscriber) {
    add_subscriber(std::move(subscriber), {});
}

void OrderedChannel::add_subscriber(
    std::shared_ptr<Subscriber> subscriber,
    std::vector<std::shared_ptr<Constraint>> constraints) {
    if (!subscriber)
        return;

    {
        std::lock_guard<std::mutex> lock(mutex_);

        if (std::find(subscribers_.begin(), subscribers_.end(), subscriber) !=
            subscribers_.end()) {
            return;
        }

        subscribers_.push_back(subscriber);
        constraints_by_subscriber_.emplace(subscriber, constraints);

        // Queues immediate delivery of current content to the new subscriber.
        if (open_) {
            queue_.push({MessageEvent::Broadcast, content_});
        } else {
            queue_.push({MessageEvent::Stop, content_});
        }
    }
    pump();
}

void OrderedChannel::remove_subscriber(std::shared_ptr<Subscriber> subscriber) {
    if (!subscriber)
        return;

    {
        std::lock_guard<std::mutex> lock(mutex_);

        if (std::find(subscribers_.begin(), subscribers_.end(), subscriber) ==
            subscribers_.end()) {
            return;
        }

        // Notifies Stop to all subscribers (including the one being removed)
        // while it is still registered.
        queue_.push({MessageEvent::Stop, content_});
    }
    pump();

    {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = std::find(subscribers_.begin(), subscribers_.end(), subscriber);
        if (it != subscribers_.end()) {
            subscribers_.erase(it);
        }
        constraints_by_subscriber_.erase(subscriber);
    }
}

void OrderedChannel::reset() {
    {
        std::lock_guard<std::mutex> lock(mutex_);

        // Notifies Stop to all subscribers before clearing.
        for (size_t i = 0; i < subscribers_.size(); ++i) {
            queue_.push({MessageEvent::Stop, content_});
        }
    }
    pump();

    {
        std::lock_guard<std::mutex> lock(mutex_);
        subscribers_.clear();
        constraints_by_subscriber_.clear();
    }
}

std::vector<std::shared_ptr<Subscriber>> OrderedChannel::get_subscribers() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return subscribers_;
}

void OrderedChannel::process_pending() {
    pump();
}

bool OrderedChannel::passes_constraints(
    const std::vector<std::shared_ptr<Constraint>>& constraints,
    MessageEvent event,
    const std::any& content) const {
    for (auto& c : constraints) {
        if (!c->validate(sender_, event, content))
            return false;
    }
    return true;
}

void OrderedChannel::notify_subscriber(
    std::shared_ptr<Subscriber> subscriber,
    MessageEvent event,
    const std::any& content,
    const std::vector<std::shared_ptr<Constraint>>& constraints) {
    if (!subscriber)
        return;

    if (!passes_constraints(constraints, event, content))
        return;

    try {
        // Passes const reference to content; valid only for this call.
        subscriber->on_message(sender_, event, content);
    } catch (const std::exception& e) {
        std::cerr << "[loomchannel] subscriber threw: " << e.what() << "\n";
    }
}

void OrderedChannel::pump() {
    // Only one thread drains the queue at a time; strict arrival order is
    // preserved because the owner delivers everything until the queue is
    // empty.
    bool expected = false;
    if (!delivering_.compare_exchange_strong(expected, true))
        return;

    for (;;) {
        QueuedMessage msg;
        std::vector<std::shared_ptr<Subscriber>> snapshot;
        std::vector<std::vector<std::shared_ptr<Constraint>>> constraints;
        {
            std::lock_guard<std::mutex> lock(mutex_);
            if (queue_.empty()) {
                // Resetting the flag under the same lock as the empty check
                // guarantees no message is left behind: any push that happened
                // before this point is seen here; any push after it makes the
                // pushing thread the next owner.
                delivering_ = false;
                return;
            }
            msg = std::move(queue_.front());
            queue_.pop();
            content_ = msg.payload; // keep content() consistent with delivery
            snapshot = subscribers_;
            for (auto& sub : snapshot) {
                auto it = constraints_by_subscriber_.find(sub);
                constraints.push_back(it != constraints_by_subscriber_.end()
                                          ? it->second
                                          : std::vector<std::shared_ptr<Constraint>>{});
            }
        }

        // Deliver outside the lock: callbacks may re-enter the channel or Bus.
        for (size_t i = 0; i < snapshot.size(); ++i) {
            notify_subscriber(snapshot[i], msg.event, msg.payload, constraints[i]);
        }
    }
}

} // namespace loomchannel
