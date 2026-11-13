#include "loomchannel/bus.hpp"
#include "loomchannel/connection.hpp"

#include <algorithm>
#include <functional>
#include <iostream>

namespace loomchannel {

// ===========================================================================
// Connection (RAII)
// ===========================================================================
Connection::Connection(std::shared_ptr<Bus> bus,
                       std::string channel_id,
                       std::shared_ptr<Subscriber> subscriber)
    : bus_(std::move(bus)),
      channel_id_(std::move(channel_id)),
      subscriber_(std::move(subscriber)) {}

Connection::Connection(Connection&& other) noexcept
    : bus_(std::move(other.bus_)),
      channel_id_(std::move(other.channel_id_)),
      subscriber_(std::move(other.subscriber_)) {}

Connection& Connection::operator=(Connection&& other) noexcept {
    if (this != &other) {
        disconnect();
        bus_ = std::move(other.bus_);
        channel_id_ = std::move(other.channel_id_);
        subscriber_ = std::move(other.subscriber_);
    }
    return *this;
}

Connection::~Connection() {
    disconnect();
}

void Connection::disconnect() {
    if (!subscriber_ || !bus_)
        return;

    auto b = bus_;
    auto cid = channel_id_;
    auto sub = subscriber_;

    // Reset to prevent reentrancy / double-disconnect.
    bus_.reset();
    channel_id_.clear();
    subscriber_.reset();

    b->unsubscribe(cid, sub);
}

// ===========================================================================
// Bus
// ===========================================================================
Bus& Bus::instance() {
    static Bus inst;
    return inst;
}

void Bus::open(std::string channel_id, std::shared_ptr<Channel> channel) {
    if (!channel)
        return;

    // Do not overwrite an already open channel (preserves connected subscribers).
    std::shared_ptr<Channel> target;
    std::vector<std::pair<std::shared_ptr<Subscriber>,
                          std::vector<std::shared_ptr<Constraint>>>>
        orphans;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = open_channels_.find(channel_id);
        if (it == open_channels_.end()) {
            it = open_channels_.emplace(channel_id, channel).first;
        }
        target = it->second;

        // Extract orphan subscribers so they can be migrated to the channel
        // outside the Bus lock (add_subscriber notifies its subscriber).
        auto oit = orphan_subscribers_.find(channel_id);
        if (oit != orphan_subscribers_.end()) {
            auto cit = orphan_constraints_.find(channel_id);
            auto& constraints =
                cit != orphan_constraints_.end() ? cit->second : empty_constraints_;
            for (size_t i = 0; i < oit->second.size(); ++i) {
                orphans.emplace_back(
                    oit->second[i],
                    i < constraints.size() ? constraints[i] : std::vector<std::shared_ptr<Constraint>>{});
            }
            oit->second.clear();
            if (cit != orphan_constraints_.end())
                cit->second.clear();
            orphan_subscribers_.erase(oit);
            if (cit != orphan_constraints_.end())
                orphan_constraints_.erase(cit);
        }
    }

    // Outside the Bus lock: these calls notify subscribers.
    target->set_open(true);
    for (auto& [sub, constraints] : orphans) {
        target->add_subscriber(sub, std::move(constraints));
    }
}

void Bus::close(std::string channel_id) {
    std::shared_ptr<Channel> channel;
    {
        std::lock_guard<std::mutex> lock(mutex_);

        auto it = open_channels_.find(channel_id);
        if (it == open_channels_.end())
            return;

        channel = it->second;

        // Keep subscribers as orphans for future reopening.
        auto subscribers = channel->get_subscribers();
        ensure_orphan_list(channel_id);
        for (auto& sub : subscribers) {
            orphan_subscribers_[channel_id].push_back(sub);
        }

        open_channels_.erase(it);
    }

    // Outside the Bus lock: set_open(false) notifies Stop to subscribers.
    channel->set_open(false);
}

Connection Bus::subscribe(std::string channel_id,
                          std::shared_ptr<Subscriber> subscriber) {
    return subscribe(std::move(channel_id), std::move(subscriber), {});
}

Connection Bus::subscribe(std::string channel_id,
                          std::shared_ptr<Subscriber> subscriber,
                          std::vector<std::shared_ptr<Constraint>> constraints) {
    if (!subscriber)
        return {};

    std::shared_ptr<Channel> target;
    {
        std::lock_guard<std::mutex> lock(mutex_);

        auto it = open_channels_.find(channel_id);
        if (it != open_channels_.end()) {
            target = it->second;
        } else {
            // Channel does not exist yet: subscriber becomes an orphan.
            ensure_orphan_list(channel_id);
            orphan_subscribers_[channel_id].push_back(subscriber);
            orphan_constraints_[channel_id].push_back(std::move(constraints));
        }
    }

    // Outside the Bus lock: add_subscriber notifies the subscriber immediately.
    if (target) {
        target->add_subscriber(subscriber, std::move(constraints));
    }

    // Returns RAII handle. The Bus does not retain a shared_ptr to itself,
    // so the user must ensure the Bus outlives the Connection (the singleton
    // instance() satisfies this naturally).
    auto self = std::shared_ptr<Bus>(&Bus::instance(), [](Bus*) {});
    return Connection(self, channel_id, subscriber);
}

Connection Bus::subscribe(std::string channel_id,
                          std::function<void(void*, MessageEvent, const std::any&)> callback) {
    return subscribe(std::move(channel_id),
                     make_subscriber(std::move(callback)));
}

Connection Bus::subscribe(std::string channel_id,
                          std::function<void(void*, MessageEvent, const std::any&)> callback,
                          std::vector<std::shared_ptr<Constraint>> constraints) {
    return subscribe(std::move(channel_id),
                     make_subscriber(std::move(callback)),
                     std::move(constraints));
}

void Bus::unsubscribe(std::string channel_id,
                      std::shared_ptr<Subscriber> subscriber) {
    if (!subscriber)
        return;

    std::shared_ptr<Channel> target;
    {
        std::lock_guard<std::mutex> lock(mutex_);

        auto it = open_channels_.find(channel_id);
        if (it != open_channels_.end()) {
            target = it->second;
        }
    }

    // Outside the Bus lock: remove_subscriber notifies Stop to the subscriber.
    if (target) {
        target->remove_subscriber(subscriber);
        return;
    }

    std::lock_guard<std::mutex> lock(mutex_);
    auto oit = orphan_subscribers_.find(channel_id);
    if (oit != orphan_subscribers_.end()) {
        auto& vec = oit->second;
        vec.erase(std::remove(vec.begin(), vec.end(), subscriber), vec.end());

        // Clear constraints for this orphan channel (they are reapplied
        // only when the channel opens again).
        auto cit = orphan_constraints_.find(channel_id);
        if (cit != orphan_constraints_.end()) {
            orphan_constraints_.erase(cit);
        }

        if (vec.empty()) {
            orphan_subscribers_.erase(oit);
        }
    }
}

std::shared_ptr<Channel> Bus::channel(std::string channel_id) const {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = open_channels_.find(channel_id);
    if (it != open_channels_.end())
        return it->second;
    return nullptr;
}

bool Bus::has_channel(std::string channel_id) const {
    std::lock_guard<std::mutex> lock(mutex_);
    return open_channels_.find(channel_id) != open_channels_.end();
}

void Bus::ensure_orphan_list(std::string channel_id) {
    if (orphan_subscribers_.find(channel_id) == orphan_subscribers_.end()) {
        orphan_subscribers_.emplace(channel_id, std::vector<std::shared_ptr<Subscriber>>{});
        orphan_constraints_.emplace(channel_id,
                                    std::vector<std::vector<std::shared_ptr<Constraint>>>{});
    }
}

} // namespace loomchannel
