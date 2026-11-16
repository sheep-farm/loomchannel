#include "receiver_dock.h"

#include <QLabel>
#include <QTextEdit>
#include <QTime>
#include <QVBoxLayout>
#include <QWidget>

#include "loomchannel/any_helpers.hpp"
#include "loomchannel/bus.hpp"

namespace {

constexpr char kChannel[] = "chat";

} // namespace

ReceiverDock::ReceiverDock(QWidget* parent)
    : QDockWidget(tr("Receptor"), parent) {
    auto* content = new QWidget(this);
    auto* layout = new QVBoxLayout(content);

    state_ = new QLabel(content);
    log_ = new QTextEdit(content);
    log_->setReadOnly(true);

    layout->addWidget(state_);
    layout->addWidget(log_);

    setWidget(content);

    // Subscribe before the transmitter opens the channel: until then the
    // subscription waits on the bus as an orphan and is delivered to the
    // channel on open.
    connection_ = loomchannel::Bus::instance().subscribe(
        kChannel,
        [this](void*, loomchannel::MessageEvent event, const std::any& payload) {
            if (event == loomchannel::MessageEvent::Stop) {
                set_state(false);
                append_log(tr("canal fechado"));
                return;
            }
            set_state(true);
            const auto text =
                loomchannel::any_cast_or<std::string>(payload, std::string{});
            if (text.empty()) {
                // Opened without content yet.
                append_log(tr("canal aberto"));
            } else {
                append_log(QString::fromStdString(text));
            }
        });

    // The bus does not notify an orphan subscription at subscribe time,
    // so the initial open/closed state comes from a direct query.
    set_state(loomchannel::Bus::instance().has_channel(kChannel));
}

ReceiverDock::~ReceiverDock() {
    // Disconnect here, while the widgets are still alive: the RAII
    // unsubscribe triggers a Stop notification back into this dock.
    connection_.disconnect();
}

void ReceiverDock::set_state(bool open) {
    if (open) {
        state_->setText(tr("● canal ABERTO"));
        state_->setStyleSheet("color: #3fb950; font-weight: bold;");
    } else {
        state_->setText(tr("● canal FECHADO"));
        state_->setStyleSheet("color: #f85149; font-weight: bold;");
    }
}

void ReceiverDock::append_log(const QString& line) {
    log_->append(QTime::currentTime().toString("HH:mm:ss") + "  " + line);
}
