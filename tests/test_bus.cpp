/**
 * @file tests/test_bus.cpp
 * @brief Unit tests for LoomChannel Bus and Channel.
 */

#include <gtest/gtest.h>

#include <loomchannel/bus.hpp>
#include <loomchannel/channel.hpp>
#include <loomchannel/ordered_channel.hpp>
#include <loomchannel/subscriber.hpp>
#include <loomchannel/constraint.hpp>
#include <loomchannel/any_helpers.hpp>

#include <any>
#include <memory>
#include <string>
#include <thread>
#include <vector>

using namespace loomchannel;

namespace {

// ---------------------------------------------------------------------------
// Test subscriber that records received messages
// ---------------------------------------------------------------------------
class RecordingSubscriber : public Subscriber {
public:
    struct Message {
        void* sender = nullptr;
        MessageEvent event = MessageEvent::Stop;
        std::string payload;
    };

    void on_message(void* sender, MessageEvent event, const std::any& payload) override {
        std::lock_guard<std::mutex> lock(mutex_);
        messages_.push_back({sender, event, any_cast_or<std::string>(payload, std::string{})});
    }

    std::vector<Message> get_messages() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return messages_;
    }

    size_t count() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return messages_.size();
    }

    void clear() {
        std::lock_guard<std::mutex> lock(mutex_);
        messages_.clear();
    }

private:
    mutable std::mutex mutex_;
    std::vector<Message> messages_;
};

// ---------------------------------------------------------------------------
// Fixture: each test gets its own channel name to avoid singleton state leak
// ---------------------------------------------------------------------------
class BusTest : public ::testing::Test {
protected:
    void SetUp() override {
        const char* test_name = ::testing::UnitTest::GetInstance()->current_test_info()->name();
        channel_name_ = "test.channel." + std::string(test_name);
    }

    const std::string& channel_name() const { return channel_name_; }

private:
    std::string channel_name_;
};

// ---------------------------------------------------------------------------
// Bus tests
// ---------------------------------------------------------------------------
TEST_F(BusTest, OpenAndCloseChannel) {
    auto& bus = Bus::instance();
    auto channel = std::make_shared<DefaultChannel>(channel_name(), nullptr);

    EXPECT_FALSE(bus.has_channel(channel_name()));
    bus.open(channel_name(), channel);
    EXPECT_TRUE(bus.has_channel(channel_name()));
    EXPECT_EQ(bus.channel(channel_name()), channel);

    bus.close(channel_name());
    EXPECT_FALSE(bus.has_channel(channel_name()));
}

TEST_F(BusTest, OpenDoesNotOverwriteExistingChannel) {
    auto& bus = Bus::instance();
    auto ch1 = std::make_shared<DefaultChannel>(channel_name(), reinterpret_cast<void*>(0x1));
    auto ch2 = std::make_shared<DefaultChannel>(channel_name(), reinterpret_cast<void*>(0x2));

    bus.open(channel_name(), ch1);
    bus.open(channel_name(), ch2); // should be ignored

    EXPECT_EQ(bus.channel(channel_name()), ch1);
}

TEST_F(BusTest, SubscribersReceivePublishedContent) {
    auto& bus = Bus::instance();
    auto channel = std::make_shared<DefaultChannel>(channel_name(), nullptr);
    bus.open(channel_name(), channel);
    auto ch = bus.channel(channel_name());

    ASSERT_NE(ch, nullptr);

    auto sub1 = std::make_shared<RecordingSubscriber>();
    auto sub2 = std::make_shared<RecordingSubscriber>();
    ch->add_subscriber(sub1);
    ch->add_subscriber(sub2);

    sub1->clear();
    sub2->clear();

    ch->set_content(std::string("data"));

    EXPECT_EQ(sub1->count(), 1u);
    EXPECT_EQ(sub2->count(), 1u);
    EXPECT_EQ(sub1->get_messages()[0].payload, "data");
    EXPECT_EQ(sub2->get_messages()[0].payload, "data");
}

TEST_F(BusTest, BusSubscribeDoesNotCrash) {
    auto& bus = Bus::instance();
    auto channel = std::make_shared<DefaultChannel>(channel_name(), nullptr);
    bus.open(channel_name(), channel);

    auto sub = std::make_shared<RecordingSubscriber>();
    auto conn = bus.subscribe(channel_name(), sub);

    // Connection should be valid
    EXPECT_TRUE(conn.connected());

    // Subscriber should get an immediate delivery (empty content) on subscribe
    EXPECT_GE(sub->count(), 1u);

    conn.disconnect();
    EXPECT_FALSE(conn.connected());
}

TEST_F(BusTest, OrphanSubscriberDoesNotCrash) {
    auto& bus = Bus::instance();
    auto channel = std::make_shared<DefaultChannel>(channel_name(), nullptr);

    // Simulate orphan: subscribe to channel before it exists via Bus
    auto sub = std::make_shared<RecordingSubscriber>();
    bus.subscribe(channel_name(), sub);

    // Open channel - should not crash
    bus.open(channel_name(), channel);
    auto ch = bus.channel(channel_name());
    ASSERT_NE(ch, nullptr);

    // Publish real content through the channel
    ch->set_content(std::string("hello"));

    // Directly add the subscriber to the channel and verify delivery works
    ch->add_subscriber(sub);
    sub->clear();
    ch->set_content(std::string("hello again"));

    auto msgs = sub->get_messages();
    bool received = false;
    for (const auto& m : msgs) {
        if (m.payload == "hello again")
            received = true;
    }
    EXPECT_TRUE(received);
}

TEST_F(BusTest, UnsubscribeStopsDelivery) {
    auto& bus = Bus::instance();
    auto channel = std::make_shared<DefaultChannel>(channel_name(), nullptr);
    bus.open(channel_name(), channel);
    auto ch = bus.channel(channel_name());

    auto sub = std::make_shared<RecordingSubscriber>();
    ch->add_subscriber(sub);
    sub->clear(); // ignore immediate delivery

    ch->set_content(std::string("first"));

    bool received_first = false;
    for (const auto& m : sub->get_messages()) {
        if (m.payload == "first")
            received_first = true;
    }
    EXPECT_TRUE(received_first);

    ch->remove_subscriber(sub);
    ch->set_content(std::string("second"));

    bool received_second = false;
    for (const auto& m : sub->get_messages()) {
        if (m.payload == "second")
            received_second = true;
    }
    EXPECT_FALSE(received_second);
}

TEST_F(BusTest, ConnectionRAIIUnsubscribesOnScopeExit) {
    auto& bus = Bus::instance();
    auto channel = std::make_shared<DefaultChannel>(channel_name(), nullptr);
    bus.open(channel_name(), channel);
    auto ch = bus.channel(channel_name());

    {
        auto sub = std::make_shared<RecordingSubscriber>();
        auto conn = bus.subscribe(channel_name(), sub);
        EXPECT_TRUE(conn.connected());
        // sub should have received immediate delivery on subscribe
        EXPECT_GE(sub->count(), 1u);
    } // conn destroyed here -> unsubscribed

    ch->set_content(std::string("outside"));
    // sub was destroyed; no crash expected
}

TEST_F(BusTest, ConstraintFiltersMessages) {
    auto& bus = Bus::instance();
    auto channel = std::make_shared<DefaultChannel>(channel_name(), nullptr);
    bus.open(channel_name(), channel);
    auto ch = bus.channel(channel_name());

    auto sub = std::make_shared<RecordingSubscriber>();
    auto only_jdbc = make_constraint([](void*, MessageEvent, const std::any& p) {
        std::string s = any_cast_or<std::string>(p, std::string{});
        return s.rfind("jdbc:", 0) == 0;
    });
    ch->add_subscriber(sub, {only_jdbc});
    sub->clear(); // ignore immediate delivery (empty content, filtered out)

    ch->set_content(std::string("jdbc:sqlite://db"));
    EXPECT_EQ(sub->count(), 1u);

    ch->set_content(std::string("http://example.com"));
    EXPECT_EQ(sub->count(), 1u); // filtered out
}

// ---------------------------------------------------------------------------
// DefaultChannel tests (no Bus needed, no state leak)
// ---------------------------------------------------------------------------
TEST(DefaultChannelTest, SubscriberReceivesStopOnClose) {
    auto channel = std::make_shared<DefaultChannel>("ch", nullptr);
    channel->set_open(true);

    auto sub = std::make_shared<RecordingSubscriber>();
    channel->add_subscriber(sub);
    sub->clear(); // ignore immediate Broadcast on subscribe

    channel->set_open(false); // should notify Stop

    auto msgs = sub->get_messages();
    ASSERT_EQ(msgs.size(), 1u);
    EXPECT_EQ(msgs[0].event, MessageEvent::Stop);
}

TEST(DefaultChannelTest, NewSubscriberGetsCurrentContent) {
    auto channel = std::make_shared<DefaultChannel>("ch", nullptr);
    channel->set_open(true);
    channel->set_content(std::string("current"));

    auto sub = std::make_shared<RecordingSubscriber>();
    channel->add_subscriber(sub);

    auto msgs = sub->get_messages();
    ASSERT_EQ(msgs.size(), 1u);
    EXPECT_EQ(msgs[0].event, MessageEvent::Broadcast);
    EXPECT_EQ(msgs[0].payload, "current");
}

// ---------------------------------------------------------------------------
// OrderedChannel tests
// ---------------------------------------------------------------------------
TEST(OrderedChannelTest, MultipleThreadsPreserveAllMessages) {
    auto channel = std::make_shared<OrderedChannel>("ordered", nullptr);
    channel->set_open(true);

    auto sub = std::make_shared<RecordingSubscriber>();
    channel->add_subscriber(sub);
    sub->clear(); // ignore immediate delivery on subscribe

    std::vector<std::thread> threads;
    for (int t = 0; t < 3; ++t) {
        threads.emplace_back([&channel, t]() {
            for (int i = 0; i < 5; ++i) {
                int msg = t * 100 + i;
                channel->set_content(std::to_string(msg));
            }
        });
    }
    for (auto& th : threads) {
        th.join();
    }

    auto msgs = sub->get_messages();
    // Exactly 15 broadcasts delivered (no loss, no dup)
    size_t broadcast_count = 0;
    for (const auto& m : msgs) {
        if (m.event == MessageEvent::Broadcast)
            ++broadcast_count;
    }
    EXPECT_EQ(broadcast_count, 15u);
}

} // namespace

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
