#include "text_constraints.h"

#include "loomchannel/any_helpers.hpp"

namespace loomdemo {
namespace {

// Lowercase helper for the case-insensitive keyword match.
std::string lowered(std::string text) {
    for (char& c : text)
        c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    return text;
}

} // namespace

bool PrefixConstraint::accept(const std::string& text) const {
    // C++17 idiom for starts_with.
    return text.rfind(prefix_, 0) == 0;
}

void KeywordConstraint::set_keyword(std::string keyword) {
    keyword_ = lowered(std::move(keyword));
}

bool KeywordConstraint::accept(const std::string& text) const {
    // No keyword configured: the filter lets everything through.
    if (keyword_.empty())
        return true;
    return lowered(text).find(keyword_) != std::string::npos;
}

bool LengthConstraint::accept(const std::string& text) const {
    // Counted in bytes; enough for the demo's ASCII texts.
    return text.size() <= max_length_;
}

} // namespace loomdemo
