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
    if (open == is_open())
        return *this;

    {
        std::lock_guard<std::mutex> lock(mutex_);
        open_ = open;
    }

    if (open) {
        // Channel opened: deliver current content to all subscribers.
        broadcast();
    } else {
        // Channel closed: notify all that it stopped (Stop).
        std::vector<std::shared_ptr<Subscriber>> snapshot;
        std::vector<std::vector<std::shared_ptr<Constraint>>> constraints;
        std::any content;
        {
            std::lock_guard<std::mutex> lock(mutex_);
            snapshot = subscribers_;
            for (auto& sub : snapshot) {
                auto it = constraints_by_subscriber_.find(sub);
                constraints.push_back(it != constraints_by_subscriber_.end()
                                          ? it->second
                                          : std::vector<std::shared_ptr<Constraint>>{});
            }
            content = content_;
        }
        // Notify outside the lock: callbacks may re-enter the channel or Bus.
        for (size_t i = 0; i < snapshot.size(); ++i) {
            notify_subscriber(snapshot[i], MessageEvent::Stop, content, constraints[i]);
        }
    }
    return *this;
}

bool DefaultChannel::is_open() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return open_;
}

Channel& DefaultChannel::set_content(std::any content) {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        content_ = std::move(content);
    }
    if (is_open()) {
        broadcast();
    }
    return *this;
}

std::any DefaultChannel::content() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return content_;
}

void DefaultChannel::broadcast() {
    std::vector<std::shared_ptr<Subscriber>> snapshot;
    std::vector<std::vector<std::shared_ptr<Constraint>>> constraints;
    std::any content;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!open_)
            return;
        snapshot = subscribers_;
        for (auto& sub : snapshot) {
            auto it = constraints_by_subscriber_.find(sub);
            constraints.push_back(it != constraints_by_subscriber_.end()
                                      ? it->second
                                      : std::vector<std::shared_ptr<Constraint>>{});
        }
        content = content_;
    }

    // Notify outside the lock: callbacks may re-enter the channel or Bus.
    for (size_t i = 0; i < snapshot.size(); ++i) {
        notify_subscriber(snapshot[i], MessageEvent::Broadcast, content, constraints[i]);
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

    MessageEvent immediate_event;
    std::any immediate_content;
    {
        std::lock_guard<std::mutex> lock(mutex_);

        if (std::find(subscribers_.begin(), subscribers_.end(), subscriber) !=
            subscribers_.end()) {
            return;
        }

        subscribers_.push_back(subscriber);
        constraints_by_subscriber_.emplace(subscriber, constraints);

        // Immediate delivery: Broadcast if open, Stop if closed.
        immediate_event = open_ ? MessageEvent::Broadcast : MessageEvent::Stop;
        immediate_content = content_;
    }

    // Notify outside the lock: callbacks may re-enter the channel or Bus.
    notify_subscriber(subscriber, immediate_event, immediate_content, constraints);
}

void DefaultChannel::remove_subscriber(std::shared_ptr<Subscriber> subscriber) {
    if (!subscriber)
        return;

    std::vector<std::shared_ptr<Constraint>> constraints;
    std::any content;
    {
        std::lock_guard<std::mutex> lock(mutex_);

        auto it = std::find(subscribers_.begin(), subscribers_.end(), subscriber);
        if (it == subscribers_.end())
            return;

        auto cit = constraints_by_subscriber_.find(subscriber);
        if (cit != constraints_by_subscriber_.end()) {
            constraints = std::move(cit->second);
            constraints_by_subscriber_.erase(cit);
        }

        subscribers_.erase(it);
        content = content_;
    }

    // Notify Stop after removal, outside the lock: callbacks may re-enter
    // the channel or Bus (e.g. the subscriber re-plugging elsewhere).
    notify_subscriber(subscriber, MessageEvent::Stop, content, constraints);
}

void DefaultChannel::reset() {
    std::vector<std::shared_ptr<Subscriber>> snapshot;
    std::vector<std::vector<std::shared_ptr<Constraint>>> constraints;
    std::any content;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        snapshot = subscribers_;
        subscribers_.clear();
        constraints_by_subscriber_.clear();
        content = content_;
    }
    // Notify outside the lock to avoid deadlock in the subscriber's callback.
    for (size_t i = 0; i < snapshot.size(); ++i) {
        notify_subscriber(snapshot[i], MessageEvent::Stop, content, constraints[i]);
    }
}

std::vector<std::shared_ptr<Subscriber>> DefaultChannel::get_subscribers() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return subscribers_;
}

bool DefaultChannel::passes_constraints(
    const std::vector<std::shared_ptr<Constraint>>& constraints,
    MessageEvent event,
    const std::any& content) const {
    for (auto& c : constraints) {
        if (!c->validate(sender_, event, content))
            return false;
    }
    return true;
}

void DefaultChannel::notify_subscriber(
    std::shared_ptr<Subscriber> subscriber,
    MessageEvent event,
    const std::any& content,
    const std::vector<std::shared_ptr<Constraint>>& constraints) {
    if (!subscriber)
        return;

    if (!passes_constraints(constraints, event, content))
        return;

    try {
        subscriber->on_message(sender_, event, content);
    } catch (const std::exception& e) {
        std::cerr << "[loomchannel] subscriber threw: " << e.what() << "\n";
    }
}

} // namespace loomchannel
