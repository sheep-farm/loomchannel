/**
 * @file example/main.cpp
 * @brief Pure C++17 console example (zero GUI dependency).
 *
 * Demonstrates:
 *  - Sender and receivers fully decoupled (they do not know each other).
 *  - Channel identified by string.
 *  - Subscription before the channel exists (orphan subscriber).
 *  - Constraints (content filtering).
 *  - Connection RAII (auto-unsubscribe on scope exit).
 *  - std::any with safe conversion helpers.
 *  - OrderedChannel: strict in-order delivery even with multiple sender threads.
 */

#include <algorithm>
#include <iostream>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include "loomchannel/bus.hpp"
#include "loomchannel/ordered_channel.hpp"
#include "loomchannel/subscriber.hpp"
#include "loomchannel/constraint.hpp"
#include "loomchannel/any_helpers.hpp"

using namespace loomchannel;

constexpr char CHANNEL_DB[] = "database.connection";
constexpr char CHANNEL_ORDERED[] = "ordered.channel";

// ---------------------------------------------------------------------------
// Helpers for std::any
// ---------------------------------------------------------------------------
static std::string any_string(const std::any& a) {
    return any_cast_or<std::string>(a, std::string{});
}

// ---------------------------------------------------------------------------
// Sender: does not know who is listening
// ---------------------------------------------------------------------------
class Database {
public:
    explicit Database(void* sender_id) : sender_id_(sender_id) {
        channel_ = std::make_shared<DefaultChannel>(CHANNEL_DB, sender_id_);
        Bus::instance().open(CHANNEL_DB, channel_);
        std::cout << "[Database] Channel opened. Sender ready.\n";
    }

    ~Database() {
        channel_->set_content(std::any{});
        channel_->set_open(false);
        Bus::instance().close(CHANNEL_DB);
        std::cout << "[Database] Channel closed.\n";
    }

    void connect(const std::string& url) {
        std::cout << "[Database] Connecting to " << url << "...\n";
        channel_->set_content(url); // automatic broadcast
    }

    void disconnect() {
        channel_->set_content(std::any{});
        channel_->set_open(false);
        std::cout << "[Database] Disconnected.\n";
    }

private:
    void* sender_id_;
    std::shared_ptr<Channel> channel_;
};

// ---------------------------------------------------------------------------
// Receiver A: listens to the specific channel
// ---------------------------------------------------------------------------
class StatusBar : public Subscriber {
public:
    void on_message(void* /*sender*/, MessageEvent event, const std::any& payload) override {
        std::string url = any_string(payload);
        if (event == MessageEvent::Broadcast && !url.empty()) {
            std::cout << "[StatusBar] Connected: " << url << "\n";
        } else {
            std::cout << "[StatusBar] No active connection.\n";
        }
    }
};

// ---------------------------------------------------------------------------
// Receiver B: generic log (listens to any channel)
// ---------------------------------------------------------------------------
class Logger : public Subscriber {
public:
    explicit Logger(std::string name) : name_(std::move(name)) {}

    void on_message(void* sender, MessageEvent event, const std::any& payload) override {
        const char* ev = (event == MessageEvent::Broadcast) ? "BROADCAST" : "STOP";
        std::cout << "[" << name_ << "] [" << ev << "] sender=" << sender
                  << " content=" << any_string(payload) << "\n";
    }

private:
    std::string name_;
};

int main() {
    auto& bus = Bus::instance();

    // =======================================================================
    // 1. Subscription BEFORE the channel exists (orphan subscriber)
    // =======================================================================
    auto logger = std::make_shared<Logger>("GlobalLog");
    auto conn_orphan = bus.subscribe(CHANNEL_DB, logger);
    std::cout << "\n--- Logger subscribed before channel exists (orphan) ---\n";

    // =======================================================================
    // 2. Sender enters the scene
    // =======================================================================
    void* db_sender = reinterpret_cast<void*>(0x1001);
    Database db(db_sender); // opens channel -> orphan receives current content

    // =======================================================================
    // 3. Another receiver subscribes normally (with Connection RAII)
    // =======================================================================
    auto status = std::make_shared<StatusBar>();
    auto conn_status = bus.subscribe(CHANNEL_DB, status);

    // =======================================================================
    // 4. Constraint: deliver only if URL starts with "jdbc:"
    // =======================================================================
    auto only_jdbc = make_constraint([](void*, MessageEvent, const std::any& p) {
        std::string s = any_cast_or<std::string>(p, std::string{});
        return s.rfind("jdbc:", 0) == 0;
    });
    auto conn_filtered = bus.subscribe(CHANNEL_DB, logger, {only_jdbc});

    // =======================================================================
    // 5. Sender publishes data
    // =======================================================================
    std::cout << "\n--- Publishing jdbc:sqlite://localhost/mydb ---\n";
    db.connect("jdbc:sqlite://localhost/mydb");

    // =======================================================================
    // 6. Unsubscription via Connection (scope exit)
    // =======================================================================
    {
        auto tmp = std::make_shared<Logger>("Temporary");
        auto tmp_conn = bus.subscribe(CHANNEL_DB, tmp);
        std::cout << "\n--- TemporaryLogger subscribed in block (RAII) ---\n";
        db.connect("jdbc:postgresql://db/prod");
        // tmp_conn destroyed here -> unsubscribes automatically
    }

    // TemporaryLogger must not receive anything after the block.
    std::cout << "\n--- Publishing after block exit (TemporaryLogger must not appear) ---\n";
    db.connect("jdbc:mysql://db/test");

    // =======================================================================
    // 7. Disconnect
    // =======================================================================
    std::cout << "\n--- Disconnecting ---\n";
    db.disconnect();

    // =======================================================================
    // 8. OrderedChannel: strict in-order delivery with multiple sender threads
    // =======================================================================
    std::cout << "\n--- OrderedChannel: multiple senders, strict order ---\n";

    auto ordered_channel =
        std::make_shared<OrderedChannel>(CHANNEL_ORDERED,
                                         reinterpret_cast<void*>(0x2002));
    Bus::instance().open(CHANNEL_ORDERED, ordered_channel);

    std::mutex print_mutex;
    std::vector<int> received_order;

    auto conn_ordered = bus.subscribe(
        CHANNEL_ORDERED,
        make_subscriber([&](void* /*sender*/, MessageEvent event,
                            const std::any& payload) {
            if (event == MessageEvent::Broadcast) {
                // Ignores the initial empty content (std::any without value).
                if (!payload.has_value())
                    return;

                int value = any_cast_or<int>(payload, -1);
                std::lock_guard<std::mutex> lock(print_mutex);
                received_order.push_back(value);
                std::cout << "[OrderedReceiver] received " << value << "\n";
            }
        }));

    // Múltiplas threads enviam mensagens numeradas concorrentemente.
    // Coletivamente produzem exatamente os valores 0..14, intercalados.
    std::vector<std::thread> threads;
    for (int sender_id = 0; sender_id < 3; ++sender_id) {
        threads.emplace_back([sender_id, &ordered_channel]() {
            for (int i = 0; i < 5; ++i) {
                int msg = sender_id + i * 3; // 0,3,6,9,12 / 1,4,7,10,13 / 2,5,8,11,14
                ordered_channel->set_content(msg);
            }
        });
    }

    for (auto& t : threads) {
        t.join();
    }

    // Valida que recebemos exatamente os 15 valores (0..14), sem perda
    // nem duplicata. A ordem exata depende de qual thread segura o mutex
    // primeiro, mas o OrderedChannel preserva a ordem de chegada ao canal.
    std::vector<int> expected(15);
    for (int i = 0; i < 15; ++i)
        expected[i] = i;

    std::vector<int> sorted = received_order;
    std::sort(sorted.begin(), sorted.end());
    bool no_loss_or_dup = (sorted == expected);

    std::cout << "[OrderedChannel] Messages received: "
              << received_order.size() << " (expected 15)\n";
    std::cout << "[OrderedChannel] Values are exactly 0..14 (no loss/dup): "
              << (no_loss_or_dup ? "yes" : "no") << "\n";

    ordered_channel->set_open(false);
    Bus::instance().close(CHANNEL_ORDERED);

    std::cout << "\n--- End ---\n";
    return 0;
}
