#pragma once

/**
 * @file any_helpers.hpp
 * @brief Safe helpers for working with std::any.
 *
 * Example:
 *   std::any a = std::string("hello");
 *   auto s = any_cast_or<std::string>(a, std::string{}); // "hello"
 *   auto i = any_cast_or<int>(a, 0);                     // 0 (wrong type)
 */

#include <any>
#include <optional>
#include <string>
#include <type_traits>

namespace loomchannel {

/**
 * @brief Safely casts std::any to T, returning default_value on failure.
 */
template<typename T>
T any_cast_or(const std::any& a, T default_value) {
    if (!a.has_value()) return default_value;
    try {
        return std::any_cast<T>(a);
    } catch (const std::bad_any_cast&) {
        return default_value;
    }
}

/**
 * @brief Returns true if std::any contains a value of type T.
 */
template<typename T>
bool any_is(const std::any& a) noexcept {
    return a.has_value() && a.type() == typeid(T);
}

/**
 * @brief Returns an optional<T> from std::any.
 * Useful to avoid bad_any_cast exceptions.
 */
template<typename T>
std::optional<T> any_optional(const std::any& a) {
    if (!a.has_value()) return std::nullopt;
    try {
        return std::any_cast<T>(a);
    } catch (const std::bad_any_cast&) {
        return std::nullopt;
    }
}

} // namespace loomchannel
