#pragma once

/**
 * @file sender_dock.h
 * @brief QDockWidget transmitter: owns the channel, opens/closes it and
 *        publishes text messages on the bus.
 */

#include <memory>

#include <QDockWidget>

namespace loomchannel {
class DefaultChannel;
}

class QLineEdit;
class QPushButton;
class QLabel;

class SenderDock : public QDockWidget {
    Q_OBJECT

public:
    explicit SenderDock(QWidget* parent = nullptr);
    ~SenderDock() override;

private slots:
    void toggle_channel(bool open);
    void send();

private:
    void refresh_status();

    QLineEdit* message_;
    QPushButton* send_button_;
    QPushButton* open_button_;
    QLabel* status_;

    // The channel object stays alive while closed so its content
    // survives a close/open cycle and is broadcast again on reopen.
    std::shared_ptr<loomchannel::DefaultChannel> channel_;
};
