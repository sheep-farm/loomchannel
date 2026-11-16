#pragma once

/**
 * @file channels.h
 * @brief Channel identifiers shared by every participant of the demo.
 */

namespace loomdemo {

// Single text channel: transmitter, receiver and filtered receiver all
// refer to the same identifier.
constexpr char kChat[] = "chat";

} // namespace loomdemo
