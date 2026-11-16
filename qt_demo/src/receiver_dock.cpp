#include "receiver_dock.h"

ReceiverDock::ReceiverDock(QWidget* parent)
    : ReceiverBase(tr("Receptor"), {}, parent) {
    // No constraints: this dock receives everything on the channel.
    subscribe();
}
