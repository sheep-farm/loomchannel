#include "transmitter_pane.h"

#include <QAction>
#include <QKeySequence>
#include <QLabel>
#include <QStringList>
#include <QToolButton>
#include <QVBoxLayout>

#include "channels.h"
#include "loomchannel/bus.hpp"

TransmitterPane::TransmitterPane(QWidget* parent) : QWidget(parent) {
    state_ = new QLabel(this);
    hint_ = new QLabel(this);
    run_button_ = new QToolButton(this);
    open_button_ = new QToolButton(this);
    open_button_->setCheckable(true);

    run_action_ = new QAction(tr("Executar"), this);
    run_action_->setShortcut(QKeySequence("Ctrl+R"));
    open_action_ = new QAction(tr("Abrir canal"), this);
    open_action_->setCheckable(true);

    run_button_->setDefaultAction(run_action_);
    open_button_->setDefaultAction(open_action_);

    // Signal-to-signal: the action is the single entry point for "run",
    // shared by the button, the menu, the toolbar and the shortcut.
    connect(run_action_, &QAction::triggered, this, &TransmitterPane::run_requested);
    connect(open_action_, &QAction::toggled, this, &TransmitterPane::toggle_channel);

    auto* layout = new QVBoxLayout(this);
    layout->addWidget(state_);
    layout->addWidget(run_button_);
    layout->addWidget(open_button_);
    layout->addWidget(hint_);
    layout->addStretch();

    refresh();
}

TransmitterPane::~TransmitterPane() {
    // Close the channel on the way out so subscribers see a proper Stop
    // instead of losing the sender silently.
    if (channel_ && channel_->is_open()) {
        loomchannel::Bus::instance().close(loomdemo::kChat);
    }
}

void TransmitterPane::toggle_channel(bool open) {
    if (open) {
        if (!channel_) {
            // This pane is the sender: the channel points back at it.
            channel_ = std::make_shared<loomchannel::DefaultChannel>(
                loomdemo::kChat, static_cast<void*>(this));
        }
        // Registers the channel (or reattaches the same object after a
        // close) and delivers the current content to subscribers.
        loomchannel::Bus::instance().open(loomdemo::kChat, channel_);
    } else {
        // Subscribers stay on the bus as orphans and are reattached on
        // the next open, so nothing needs to be done with them here.
        loomchannel::Bus::instance().close(loomdemo::kChat);
    }
    refresh();
}

void TransmitterPane::publish_lines(const QStringList& lines) {
    const bool was_open = channel_ && channel_->is_open();

    if (!channel_) {
        // Running with the toggle untouched still has to work: the
        // channel is created closed, so lines are stored but not
        // delivered.
        channel_ = std::make_shared<loomchannel::DefaultChannel>(
            loomdemo::kChat, static_cast<void*>(this));
    }

    // Publishes; with the channel closed it only updates the stored
    // content, which is broadcast as soon as the channel reopens.
    for (const QString& line : lines) {
        const QString text = line.trimmed();
        if (!text.isEmpty()) {
            channel_->set_content(text.toStdString());
        }
    }

    if (!was_open) {
        hint_->setText(tr("mensagem armazenada (canal fechado)"));
        hint_->setStyleSheet("color: #f85149;");
    } else {
        hint_->setText(tr("mensagem publicada"));
        hint_->setStyleSheet("color: #3fb950;");
    }
}

void TransmitterPane::refresh() {
    const bool open = channel_ && channel_->is_open();
    open_action_->setText(open ? tr("Fechar canal") : tr("Abrir canal"));
    if (open) {
        state_->setText(tr("● canal ABERTO"));
        state_->setStyleSheet("color: #3fb950; font-weight: bold;");
        hint_->setText(tr("canal aberto"));
        hint_->setStyleSheet("color: #3fb950;");
    } else {
        state_->setText(tr("● canal FECHADO"));
        state_->setStyleSheet("color: #f85149; font-weight: bold;");
        hint_->setText(tr("canal fechado"));
        hint_->setStyleSheet("color: #f85149;");
    }
}
