#pragma once

/**
 * @file constraint.hpp
 * @brief Optional per-subscriber filter for LoomChannel.
 */

#include <any>
#include <functional>
#include <memory>
#include <vector>

#include "loomchannel/message_event.hpp"

namespace loomchannel {

/**
 * @brief Optional filter applied to a specific subscriber.
 *
 * If `validate` returns false, the message is not delivered to that subscriber.
 * Allows, for example, delivering only messages whose content satisfies a
 * given criterion without modifying the sender.
 */
class Constraint {
public:
    virtual ~Constraint() = default;

    /**
     * @param sender pointer to the sender (may be nullptr).
     * @param event event type.
     * @param payload message content.
     * @return true if the message should be delivered.
     */
    virtual bool validate(void* sender,
                          MessageEvent event,
                          const std::any& payload) const = 0;
};

/**
 * @brief Adapter that wraps a std::function as a Constraint.
 */
class FunctionConstraint : public Constraint {
public:
    using Predicate = std::function<bool(void*, MessageEvent, const std::any&)>;

    explicit FunctionConstraint(Predicate p) : p_(std::move(p)) {}

    bool validate(void* sender,
                  MessageEvent event,
                  const std::any& payload) const override {
        return p_(sender, event, payload);
    }

private:
    Predicate p_;
};

/**
 * @brief Creates a constraint from a lambda/predicate.
 */
inline std::shared_ptr<Constraint> make_constraint(
    std::function<bool(void*, MessageEvent, const std::any&)> p) {
    return std::make_shared<FunctionConstraint>(std::move(p));
}

} // namespace loomchannel
