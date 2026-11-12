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
    std::lock_guard<std::mutex> lock(mutex_);
    if (open_ == open)
        return *this;

    open_ = open;

    if (open_) {
        queue_.push({MessageEvent::Broadcast, content_});
    } else {
        queue_.push({MessageEvent::Stop, std::any{}});
    }

    if (!delivering_) {
        delivering_ = true;
        deliver_queued();
        delivering_ = false;
    }
    return *this;
}

bool OrderedChannel::is_open() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return open_;
}

Channel& OrderedChannel::set_content(std::any content) {
    std::lock_guard<std::mutex> lock(mutex_);
    content_ = std::move(content);
    queue_.push({MessageEvent::Broadcast, content_});

    if (!delivering_) {
        delivering_ = true;
        deliver_queued();
        delivering_ = false;
    }
    return *this;
}

std::any OrderedChannel::content() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return content_;
}

void OrderedChannel::broadcast() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!open_)
        return;

    queue_.push({MessageEvent::Broadcast, content_});

    if (!delivering_) {
        delivering_ = true;
        deliver_queued();
        delivering_ = false;
    }
}

void OrderedChannel::add_subscriber(std::shared_ptr<Subscriber> subscriber) {
    add_subscriber(std::move(subscriber), {});
}

void OrderedChannel::add_subscriber(
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

    // Queues immediate delivery of current content to the new subscriber.
    if (open_) {
        queue_.push({MessageEvent::Broadcast, content_});
    } else {
        queue_.push({MessageEvent::Stop, content_});
    }

    if (!delivering_) {
        delivering_ = true;
        deliver_queued();
        delivering_ = false;
    }
}

void OrderedChannel::remove_subscriber(std::shared_ptr<Subscriber> subscriber) {
    if (!subscriber)
        return;

    std::lock_guard<std::mutex> lock(mutex_);

    auto it = std::find(subscribers_.begin(), subscribers_.end(), subscriber);
    if (it == subscribers_.end())
        return;

    // Notifies Stop to all subscribers before clearing.
    queue_.push({MessageEvent::Stop, content_});

    if (!delivering_) {
        delivering_ = true;
        deliver_queued();
        delivering_ = false;
    }

    subscribers_.erase(it);
    constraints_by_subscriber_.erase(subscriber);
}

void OrderedChannel::reset() {
    std::lock_guard<std::mutex> lock(mutex_);

    // Notifies Stop to all subscribers before clearing.
    for (auto& sub : subscribers_) {
        (void)sub;
        queue_.push({MessageEvent::Stop, content_});
    }

    if (!delivering_) {
        delivering_ = true;
        deliver_queued();
        delivering_ = false;
    }

    subscribers_.clear();
    constraints_by_subscriber_.clear();
}

std::vector<std::shared_ptr<Subscriber>> OrderedChannel::get_subscribers() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return subscribers_;
}

void OrderedChannel::process_pending() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (delivering_)
        return;
    delivering_ = true;
    deliver_queued();
    delivering_ = false;
}

bool OrderedChannel::passes_constraints(
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

void OrderedChannel::notify_subscriber(std::shared_ptr<Subscriber> subscriber,
                                       MessageEvent event) {
    if (!subscriber)
        return;

    if (!passes_constraints(subscriber, event))
        return;

    try {
        // Passes const reference to content_; valid only for this call.
        subscriber->on_message(sender_, event, content_);
    } catch (const std::exception& e) {
        std::cerr << "[loomchannel] subscriber threw: " << e.what() << "\n";
    }
}

void OrderedChannel::deliver_queued() {
    // Assumimos que mutex_ já está segurado pelo chamador.
    while (!queue_.empty()) {
        QueuedMessage msg = std::move(queue_.front());
        queue_.pop();

        // Atualiza content_ para o payload desta mensagem antes de notificar,
        // mantendo passes_constraints consistente com a mensagem em entrega.
        content_ = std::move(msg.payload);

        std::vector<std::shared_ptr<Subscriber>> snapshot = subscribers_;

        for (auto& sub : snapshot) {
            notify_subscriber(sub, msg.event);
        }
    }
}

} // namespace loomchannel
