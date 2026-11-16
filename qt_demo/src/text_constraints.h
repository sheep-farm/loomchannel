#pragma once

/**
 * @file text_constraints.h
 * @brief Named constraints for text messages, replacing anonymous filter
 *        lambdas with readable classes.
 *
 * All of them share the same state-event policy, kept in one place
 * (TextConstraint): Stop always passes, and a bare open notification
 * (empty text) also passes — only real text is filtered.
 */

#include <any>
#include <string>

#include "loomchannel/any_helpers.hpp"
#include "loomchannel/constraint.hpp"
#include "loomchannel/message_event.hpp"

namespace loomdemo {

/**
 * @brief Base for text-content constraints.
 *
 * Subclasses only decide whether a real text is accepted; the handling
 * of Stop and of the empty payload lives here, written once.
 */
class TextConstraint : public loomchannel::Constraint {
public:
    bool validate(void*, loomchannel::MessageEvent event,
                  const std::any& payload) const final {
        // Stop must always pass, or the subscriber would stop seeing
        // that the channel closed.
        if (event != loomchannel::MessageEvent::Broadcast)
            return true;
        const auto text =
            loomchannel::any_cast_or<std::string>(payload, std::string{});
        // Empty text = bare open notification: also passes.
        if (text.empty())
            return true;
        return accept(text);
    }

protected:
    ~TextConstraint() override = default;

private:
    virtual bool accept(const std::string& text) const = 0;
};

/** Accepts only texts that start with a fixed prefix. */
class PrefixConstraint final : public TextConstraint {
public:
    explicit PrefixConstraint(std::string prefix) : prefix_(std::move(prefix)) {}

    const std::string& prefix() const { return prefix_; }

protected:
    bool accept(const std::string& text) const override;

private:
    std::string prefix_;
};

/** Accepts only texts containing a keyword (case-insensitive, mutable). */
class KeywordConstraint final : public TextConstraint {
public:
    explicit KeywordConstraint(std::string keyword) { set_keyword(std::move(keyword)); }

    // Mutable so a UI can retarget the filter without resubscribing.
    // Read and written on the GUI thread in the demo; not synchronized.
    void set_keyword(std::string keyword);
    const std::string& keyword() const { return keyword_; }

protected:
    bool accept(const std::string& text) const override;

private:
    std::string keyword_;  // stored lowercase
};

/** Accepts only texts up to a maximum length (bytes). */
class LengthConstraint final : public TextConstraint {
public:
    explicit LengthConstraint(std::size_t max_length) : max_length_(max_length) {}

    std::size_t max_length() const { return max_length_; }

protected:
    bool accept(const std::string& text) const override;

private:
    std::size_t max_length_;
};

} // namespace loomdemo
