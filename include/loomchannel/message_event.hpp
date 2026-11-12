#pragma once

/**
 * @file message_event.hpp
 * @brief Message event types for the LoomChannel messaging bus.
 */

namespace loomchannel {

enum class MessageEvent {
    /** Channel closed / inactive. Payload may be null. */
    Stop,
    /** Channel active with payload available for delivery. */
    Broadcast
};

inline const char* to_string(MessageEvent e) noexcept {
    switch (e) {
        case MessageEvent::Stop:      return "Stop";
        case MessageEvent::Broadcast: return "Broadcast";
    }
    return "Unknown";
}

} // namespace loomchannel
