/**
 * @file example/advanced.cpp
 * @brief Advanced example: multiple channels, constraints, and ordered delivery.
 *
 * Demonstrates:
 *  - Multiple independent channels on the same Bus
 *  - Constraints that filter messages per subscriber
 *  - OrderedChannel guaranteeing in-order delivery from concurrent senders
 *  - Multiple subscribers reacting differently to the same channel
 */

#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <vector>

#include <loomchannel/bus.hpp>
#include <loomchannel/channel.hpp>
#include <loomchannel/ordered_channel.hpp>
#include <loomchannel/subscriber.hpp>
#include <loomchannel/constraint.hpp>
#include <loomchannel/any_helpers.hpp>

using namespace loomchannel;

constexpr char CHANNEL_ALERTS[] = "alerts";
constexpr char CHANNEL_METRICS[] = "metrics";
constexpr char CHANNEL_ORDERED[] = "ordered";

class AlertSubscriber : public Subscriber {
public:
    explicit AlertSubscriber(std::string name) : name_(std::move(name)) {}
    void on_message(void* /*sender*/, MessageEvent event, const std::any& payload) override {
        if (event == MessageEvent::Broadcast) {
            std::string msg = any_cast_or<std::string>(payload, std::string{});
            std::cout << "[" << name_ << "] Alert: " << msg << std::endl;
        }
    }
private:
    std::string name_;
};

class MetricsSubscriber : public Subscriber {
public:
    void on_message(void* /*sender*/, MessageEvent event, const std::any& payload) override {
        if (event == MessageEvent::Broadcast) {
            double value = any_cast_or<double>(payload, 0.0);
            std::cout << "[Metrics] Value: " << value << std::endl;
        }
    }
};

int main() {
    auto& bus = Bus::instance();

    // =======================================================================
    // 1. Multiple independent channels
    // =======================================================================
    std::cout << "=== Multiple Channels ===" << std::endl;

    auto alerts_channel = std::make_shared<DefaultChannel>(CHANNEL_ALERTS, nullptr);
    auto metrics_channel = std::make_shared<DefaultChannel>(CHANNEL_METRICS, nullptr);

    bus.open(CHANNEL_ALERTS, alerts_channel);
    bus.open(CHANNEL_METRICS, metrics_channel);

    auto alert_sub = std::make_shared<AlertSubscriber>("Console");
    auto metrics_sub = std::make_shared<MetricsSubscriber>();

    alerts_channel->add_subscriber(alert_sub);
    metrics_channel->add_subscriber(metrics_sub);

    alerts_channel->set_content(std::string("Disk space low!"));
    metrics_channel->set_content(42.5);

    // =======================================================================
    // 2. Constraints: only critical alerts
    // =======================================================================
    std::cout << "\n=== Constraints ===" << std::endl;

    auto critical_only = make_constraint([](void*, MessageEvent, const std::any& p) {
        std::string s = any_cast_or<std::string>(p, std::string{});
        return s.rfind("[CRITICAL]", 0) == 0;
    });

    auto critical_sub = std::make_shared<AlertSubscriber>("CriticalOnly");
    alerts_channel->add_subscriber(critical_sub, {critical_only});

    alerts_channel->set_content(std::string("Disk space low!"));      // filtered
    alerts_channel->set_content(std::string("[CRITICAL] Server down!")); // delivered

    // =======================================================================
    // 3. OrderedChannel: concurrent senders, strict order
    // =======================================================================
    std::cout << "\n=== OrderedChannel ===" << std::endl;

    auto ordered = std::make_shared<OrderedChannel>(CHANNEL_ORDERED, nullptr);
    bus.open(CHANNEL_ORDERED, ordered);

    auto ordered_sub = std::make_shared<AlertSubscriber>("Ordered");
    ordered->add_subscriber(ordered_sub);

    std::vector<std::thread> threads;
    for (int t = 0; t < 3; ++t) {
        threads.emplace_back([ordered, t]() {
            for (int i = 0; i < 5; ++i) {
                int msg = t * 100 + i;
                ordered->set_content("msg_" + std::to_string(msg));
            }
        });
    }
    for (auto& th : threads) {
        th.join();
    }

    std::cout << "\nAll done." << std::endl;
    return 0;
}
