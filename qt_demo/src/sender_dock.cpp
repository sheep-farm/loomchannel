#include "sender_dock.h"

#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>

#include "loomchannel/bus.hpp"
#include "loomchannel/channel.hpp"

namespace {

// Same identifier the ReceiverDock subscribes to.
constexpr char kChannel[] = "chat";

} // namespace

SenderDock::SenderDock(QWidget* parent)
    : QDockWidget(tr("Transmissor"), parent) {
    auto* content = new QWidget(this);
    auto* layout = new QVBoxLayout(content);

    message_ = new QLineEdit(content);
    message_->setPlaceholderText(tr("Texto da mensagem"));
    message_->setClearButtonEnabled(true);

    send_button_ = new QPushButton(tr("Enviar"), content);
    send_button_->setDefault(true);

    // Checkable: checked = channel open on the bus, unchecked = closed.
    open_button_ = new QPushButton(tr("Abrir canal"), content);
    open_button_->setCheckable(true);

    status_ = new QLabel(content);
    status_->setWordWrap(true);

    layout->addWidget(message_);
    layout->addWidget(send_button_);
    layout->addWidget(open_button_);
    layout->addWidget(status_);
    layout->addStretch();

    setWidget(content);

    connect(message_, &QLineEdit::returnPressed, this, &SenderDock::send);
    connect(send_button_, &QPushButton::clicked, this, &SenderDock::send);
    connect(open_button_, &QPushButton::toggled, this, &SenderDock::toggle_channel);

    refresh_status();
}

SenderDock::~SenderDock() {
    // Close the channel on the way out so subscribers see a proper Stop
    // instead of losing the sender silently.
    if (channel_ && channel_->is_open()) {
        loomchannel::Bus::instance().close(kChannel);
    }
}

void SenderDock::toggle_channel(bool open) {
    if (open) {
        if (!channel_) {
            // This dock is the sender: the channel points back at it.
            channel_ = std::make_shared<loomchannel::DefaultChannel>(
                kChannel, static_cast<void*>(this));
        }
        // Registers the channel (or reattaches the same object after a
        // close) and delivers the current content to subscribers.
        loomchannel::Bus::instance().open(kChannel, channel_);
    } else {
        // Subscribers stay on the bus as orphans and are reattached on
        // the next open, so nothing needs to be done with them here.
        loomchannel::Bus::instance().close(kChannel);
    }
    refresh_status();
}

void SenderDock::send() {
    const QString text = message_->text().trimmed();
    if (text.isEmpty())
        return;

    const bool was_open = channel_ && channel_->is_open();

    if (!channel_) {
        // Sending with the toggle untouched still has to work: the channel
        // is created closed, so the message is stored but not delivered.
        channel_ = std::make_shared<loomchannel::DefaultChannel>(
            kChannel, static_cast<void*>(this));
    }

    // Publishes; with the channel closed it only updates the stored
    // content, which is broadcast as soon as the channel reopens.
    channel_->set_content(text.toStdString());
    message_->clear();
    message_->setFocus();

    if (!was_open) {
        status_->setText(tr("mensagem armazenada (canal fechado)"));
    } else {
        status_->setText(tr("mensagem publicada"));
    }
}

void SenderDock::refresh_status() {
    const bool open = channel_ && channel_->is_open();
    open_button_->setText(open ? tr("Fechar canal") : tr("Abrir canal"));
    if (open) {
        status_->setText(tr("canal aberto"));
        status_->setStyleSheet("color: #3fb950;");
    } else {
        status_->setText(tr("canal fechado"));
        status_->setStyleSheet("color: #f85149;");
    }
}
