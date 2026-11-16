#include "receiver_base.h"

#include <QLabel>
#include <QTextEdit>
#include <QTime>
#include <QVBoxLayout>
#include <QWidget>

#include "channels.h"
#include "loomchannel/any_helpers.hpp"
#include "loomchannel/bus.hpp"

ReceiverBase::ReceiverBase(const QString& title, const QString& note,
                           QWidget* parent)
    : QDockWidget(title, parent) {
    auto* content = new QWidget(this);
    layout_ = new QVBoxLayout(content);

    state_ = new QLabel(content);
    note_label_ = new QLabel(content);
    log_ = new QTextEdit(content);
    log_->setReadOnly(true);

    layout_->addWidget(state_);
    layout_->addWidget(note_label_);
    layout_->addWidget(log_);

    setWidget(content);
    set_note(note);

    // The bus does not notify an orphan subscription at subscribe time,
    // so the initial open/closed state comes from a direct query.
    set_state(loomchannel::Bus::instance().has_channel(loomdemo::kChat));
}

ReceiverBase::~ReceiverBase() {
    // Disconnect here, while the widgets are still alive: the RAII
    // unsubscribe triggers a Stop notification back into this dock.
    connection_.disconnect();
}

void ReceiverBase::subscribe(
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

void ReceiverBase::set_state(bool open) {
    if (open) {
        state_->setText(tr("● canal ABERTO"));
        state_->setStyleSheet("color: #3fb950; font-weight: bold;");
    } else {
        state_->setText(tr("● canal FECHADO"));
        state_->setStyleSheet("color: #f85149; font-weight: bold;");
    }
}

void ReceiverBase::set_note(const QString& note) {
    note_label_->setText(note);
    note_label_->setWordWrap(true);
    note_label_->setVisible(!note.isEmpty());
}

void ReceiverBase::insert_header_widget(QWidget* widget) {
    layout_->insertWidget(layout_->indexOf(log_), widget);
}

void ReceiverBase::append_log(const QString& line) {
    log_->append(QTime::currentTime().toString("HH:mm:ss") + "  " + line);
}
