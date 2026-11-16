#include "filtered_receiver_dock.h"

#include "loomchannel/any_helpers.hpp"
#include "loomchannel/message_event.hpp"

namespace {

constexpr char kPrefix[] = "jdbc:";

} // namespace

FilteredReceiverDock::FilteredReceiverDock(QWidget* parent)
    : ReceiverBase(tr("Receptor filtrado"),
                   {loomchannel::make_constraint(
                       [](void*, loomchannel::MessageEvent event,
                          const std::any& payload) {
                           // Stop must always pass, or this dock would stop
                           // seeing that the channel closed.
                           if (event != loomchannel::MessageEvent::Broadcast)
                               return true;
                           const auto text = loomchannel::any_cast_or<std::string>(
                               payload, std::string{});
                           // Empty payload = bare open notification: also passes.
                           return text.empty() || text.rfind(kPrefix, 0) == 0;
                       })},
                   tr("só entrega mensagens que começam com \"%1\"").arg(kPrefix),
                   parent) {}
