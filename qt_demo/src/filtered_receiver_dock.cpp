#include "filtered_receiver_dock.h"

#include "text_constraints.h"

namespace {

constexpr char kPrefix[] = "jdbc:";

} // namespace

FilteredReceiverDock::FilteredReceiverDock(QWidget* parent)
    : ReceiverBase(tr("Receptor filtrado"),
                   tr("só entrega mensagens que começam com \"%1\"").arg(kPrefix),
                   parent) {
    subscribe({std::make_shared<loomdemo::PrefixConstraint>(kPrefix)});
}
