#include "receiver_pane.h"

#include <QLabel>
#include <QPlainTextEdit>
#include <QTime>
#include <QVBoxLayout>

#include "channels.h"
#include "loomchannel/any_helpers.hpp"
#include "loomchannel/bus.hpp"

ReceiverPane::ReceiverPane(const QString& note, QWidget* parent)
    : QWidget(parent) {
    layout_ = new QVBoxLayout(this);

    state_ = new QLabel(this);
    note_label_ = new QLabel(this);
    log_ = new QPlainTextEdit(this);
    log_->setReadOnly(true);

    layout_->addWidget(state_);
    layout_->addWidget(note_label_);
    layout_->addWidget(log_);

    set_note(note);

    // The bus does not notify an orphan subscription at subscribe time,
    // so the initial open/closed state comes from a direct query.
    set_state(loomchannel::Bus::instance().has_channel(loomdemo::kChat));
}

ReceiverPane::~ReceiverPane() {
    // Disconnect here, while the widgets are still alive: the RAII
    // unsubscribe triggers a Stop notification back into this pane.
    connection_.disconnect();
}

void ReceiverPane::subscribe(
    std::vector<std::shared_ptr<loomchannel::Constraint>> constraints) {
    auto on_message =
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
        };
    connection_ = loomchannel::Bus::instance().subscribe(
        loomdemo::kChat, on_message, std::move(constraints));
    subscribed_ = true;
}

void ReceiverPane::set_state(bool open) {
    if (open) {
        state_->setText(tr("● canal ABERTO"));
        state_->setStyleSheet("color: #3fb950; font-weight: bold;");
    } else {
        state_->setText(tr("● canal FECHADO"));
        state_->setStyleSheet("color: #f85149; font-weight: bold;");
    }
}

void ReceiverPane::set_note(const QString& note) {
    note_label_->setText(note);
    note_label_->setWordWrap(true);
    note_label_->setVisible(!note.isEmpty());
}

void ReceiverPane::insert_header_widget(QWidget* widget) {
    layout_->insertWidget(layout_->indexOf(log_), widget);
}

void ReceiverPane::append_log(const QString& line) {
    log_->appendPlainText(QTime::currentTime().toString("HH:mm:ss") + "  " + line);
}
