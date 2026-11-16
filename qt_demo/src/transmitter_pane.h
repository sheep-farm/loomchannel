#pragma once

/**
 * @file transmitter_pane.h
 * @brief Right-side tool pane: owns the channel, opens/closes it and
 *        publishes the editor's text, line by line.
 */

#include <memory>

#include <QWidget>

#include "loomchannel/channel.hpp"

class QAction;
class QLabel;
class QToolButton;

class TransmitterPane : public QWidget {
    Q_OBJECT

public:
    explicit TransmitterPane(QWidget* parent = nullptr);
    ~TransmitterPane() override;

    QAction* run_action() const { return run_action_; }
    QAction* open_action() const { return open_action_; }

    // Publishes each non-empty line as one message on the channel.
    void publish_lines(const QStringList& lines);

signals:
    void run_requested();

private:
    void toggle_channel(bool open);
    void refresh();

    QToolButton* run_button_;
    QToolButton* open_button_;
    QLabel* state_;
    QLabel* hint_;
    QAction* run_action_;
    QAction* open_action_;

    // The channel object stays alive while closed so its content
    // survives a close/open cycle and is broadcast again on reopen.
    std::shared_ptr<loomchannel::DefaultChannel> channel_;
};
