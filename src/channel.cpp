#include "loomchannel/channel.hpp"

#include <algorithm>
#include <iostream>

namespace loomchannel {

DefaultChannel::DefaultChannel(std::string channel_id, void* sender)
    : sender_(sender), content_(), open_(false), id_(std::move(channel_id)) {}

DefaultChannel::~DefaultChannel() = default;

const std::string& DefaultChannel::id() const { return id_; }

void* DefaultChannel::sender() const { return sender_; }

Channel& DefaultChannel::set_open(bool open) {
    if (open_ == open)
        return *this;

    open_ = open;

    if (open_) {
        // Channel opened: deliver current content to all subscribers.
        broadcast();
    } else {
        // Channel closed: notify all that it stopped (Stop).
        std::vector<std::shared_ptr<Subscriber>> snapshot;
        {
            std::lock_guard<std::mutex> lock(mutex_);
            snapshot = subscribers_;
        }
        for (auto& sub : snapshot) {
            notify_subscriber(sub, MessageEvent::Stop);
        }
    }
    return *this;
}

bool DefaultChannel::is_open() const { return open_; }

Channel& DefaultChannel::set_content(std::any content) {
    content_ = std::move(content);
    if (is_open()) {
        broadcast();
    }
    return *this;
}

std::any DefaultChannel::content() const { return content_; }

void DefaultChannel::broadcast() {
    if (!is_open())
        return;

    std::vector<std::shared_ptr<Subscriber>> snapshot;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        snapshot = subscribers_;
    }

    for (auto& sub : snapshot) {
        notify_subscriber(sub, MessageEvent::Broadcast);
    }
}

void DefaultChannel::add_subscriber(std::shared_ptr<Subscriber> subscriber) {
    add_subscriber(std::move(subscriber), {});
}

void DefaultChannel::add_subscriber(
    std::shared_ptr<Subscriber> subscriber,
    std::vector<std::shared_ptr<Constraint>> constraints) {
    if (!subscriber)
        return;

    std::lock_guard<std::mutex> lock(mutex_);

    if (std::find(subscribers_.begin(), subscribers_.end(), subscriber) !=
        subscribers_.end()) {
        return;
    }

    subscribers_.push_back(subscriber);
    constraints_by_subscriber_.emplace(subscriber, std::move(constraints));

    // Immediate delivery: Broadcast if open, Stop if closed.
    notify_subscriber(subscriber,
                      is_open() ? MessageEvent::Broadcast : MessageEvent::Stop);
}

void DefaultChannel::remove_subscriber(std::shared_ptr<Subscriber> subscriber) {
    if (!subscriber)
        return;

    std::lock_guard<std::mutex> lock(mutex_);

    auto it = std::find(subscribers_.begin(), subscribers_.end(), subscriber);
    if (it == subscribers_.end())
        return;

    // Notify Stop before removing (while channel is still in current state).
    notify_subscriber(subscriber, MessageEvent::Stop);

    subscribers_.erase(it);
    constraints_by_subscriber_.erase(subscriber);
}

void DefaultChannel::reset() {
    std::vector<std::shared_ptr<Subscriber>> snapshot;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        snapshot = subscribers_;
        subscribers_.clear();
        constraints_by_subscriber_.clear();
    }
    // Notify outside the lock to avoid deadlock in the subscriber's callback.
    for (auto& sub : snapshot) {
        notify_subscriber(sub, MessageEvent::Stop);
    }
}

std::vector<std::shared_ptr<Subscriber>> DefaultChannel::get_subscribers() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return subscribers_;
}

bool DefaultChannel::passes_constraints(
    std::shared_ptr<Subscriber> subscriber,
    MessageEvent event) const {
    auto it = constraints_by_subscriber_.find(subscriber);
    if (it == constraints_by_subscriber_.end())
        return true;

    for (auto& c : it->second) {
        if (!c->validate(sender_, event, content_))
            return false;
    }
    return true;
}

void DefaultChannel::notify_subscriber(std::shared_ptr<Subscriber> subscriber,
                                       MessageEvent event) {
    if (!subscriber)
        return;

    if (!passes_constraints(subscriber, event))
        return;

    try {
        subscriber->on_message(sender_, event, content_);
    } catch (const std::exception& e) {
        std::cerr << "[loomchannel] subscriber threw: " << e.what() << "\n";
    }
}

} // namespace loomchannel
