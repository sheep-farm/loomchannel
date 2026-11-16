#include <any>
#include <chrono>
#include <condition_variable>
#include <iostream>
#include <memory>
#include <mutex>
#include <queue>
#include <random>
#include <string>
#include <thread>

#include "loomchannel/any_helpers.hpp"
#include "loomchannel/bus.hpp"
#include "loomchannel/message_event.hpp"
#include "loomchannel/ordered_channel.hpp"

// ---------------------------------------------------------------------------
// A receiver thread that sleeps while nothing arrives.
//
// loomchannel is push-based: on_message() runs on the publisher's thread.
// A consumer with slow, heavy work should not run on the publisher: it
// enqueues the message (a copy — the payload crosses threads) and wakes a
// condition variable. The consumer thread blocks inside cv.wait(), costing
// zero CPU while idle, and wakes only when a message arrives.
// ---------------------------------------------------------------------------

class PrinterReceiver : public loomchannel::Subscriber {
public:
    explicit PrinterReceiver(std::string name) : name_(std::move(name)) {}

    ~PrinterReceiver() override {
        // Graceful shutdown: drain what remains, then let the thread exit.
        stop();
        join();
    }

    // Called on the publisher's thread: fast, only enqueue and wake.
    void on_message(void*, loomchannel::MessageEvent event,
                    const std::any& payload) override {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            if (event == loomchannel::MessageEvent::Broadcast)
                queue_.push(loomchannel::any_cast_or<std::string>(
                    payload, std::string{}));
            else
                // Stop: deliver what remains in the queue, then exit.
                stopping_ = true;
        }
        cv_.notify_one();
    }

    void start() {
        thread_ = std::thread([this] { run(); });
    }

    // Signals the consumer thread to drain and exit, then joins it.
    void stop() {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            stopping_ = true;
        }
        cv_.notify_all();
    }

    void join() {
        if (thread_.joinable())
            thread_.join();
    }

private:
    void run() {
        std::size_t messages = 0;
        std::size_t wakeups = 0;
        bool awake = false;

        while (true) {
            // Sleeps here at zero cost until a message or Stop arrives.
            std::unique_lock<std::mutex> lock(mutex_);
            cv_.wait(lock, [this] { return !queue_.empty() || stopping_; });

            // Drain everything already queued before sleeping again, so
            // several messages may cost a single wakeup.
            while (!queue_.empty()) {
                if (!awake) {
                    ++wakeups;
                    awake = true;
                }
                auto text = loomchannel::any_cast_or<std::string>(
                    std::move(queue_.front()), std::string{});
                queue_.pop();
                lock.unlock();

                // Heavy work happens outside the lock and off the
                // publisher's thread. A small pause stands in for it, so
                // the run below shows several messages per wakeup.
                std::this_thread::sleep_for(std::chrono::milliseconds(2));
                if (text.empty())
                    std::cout << "[" << name_ << "] channel opened\n";
                else
                    std::cout << "[" << name_ << "] " << text << "\n";
                ++messages;

                lock.lock();
            }
            awake = false;

            const bool done = stopping_;
            lock.unlock();
            if (done) {
                std::cout << "[" << name_ << "] done: " << messages
                          << " messages in " << wakeups
                          << " wakeups (slept the rest of the time)\n";
                return;
            }
        }
    }

    std::string name_;
    std::thread thread_;
    std::mutex mutex_;
    std::condition_variable cv_;
    std::queue<std::any> queue_;
    bool stopping_ = false;
};

// ---------------------------------------------------------------------------
// Two workers publishing concurrently on one OrderedChannel: the mutex of
// the channel defines arrival order and every subscriber sees the same
// sequence, no matter which thread published.
// ---------------------------------------------------------------------------

namespace {

void worker(const std::string& name, int steps,
            const std::shared_ptr<loomchannel::Channel>& channel) {
    std::mt19937 rng(std::hash<std::string>{}(name));
    std::uniform_int_distribution<int> pause(1, 4);
    for (int i = 1; i <= steps; ++i) {
        std::this_thread::sleep_for(std::chrono::milliseconds(pause(rng)));
        channel->set_content(name + ": step " + std::to_string(i));
    }
}

} // namespace

int main() {
    auto& bus = loomchannel::Bus::instance();
    constexpr char kChannel[] = "jobs";

    // The consumer subscribes before the channel exists (orphan) and is
    // delivered to it on open. The subscription is RAII: it disconnects
    // when the handle goes out of scope.
    auto consumer = std::make_shared<PrinterReceiver>("consumer");
    consumer->start();
    auto conn = bus.subscribe(kChannel, consumer);

    // One ordered channel shared by both workers.
    auto channel =
        std::make_shared<loomchannel::OrderedChannel>(kChannel, nullptr);
    bus.open(kChannel, channel);

    std::thread a(worker, "worker-a", 5, channel);
    std::thread b(worker, "worker-b", 5, channel);
    a.join();
    b.join();

    // Close: subscribers receive Stop, drain their queues and exit.
    bus.close(kChannel);
    conn.disconnect();
    consumer->join();

    std::cout << "[main] workers joined, channel closed, consumer exited.\n";
    return 0;
}
