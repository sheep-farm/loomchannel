#pragma once

/**
 * @file receiver_dock.h
 * @brief QDockWidget receiver: shows incoming text messages and tracks
 *        whether the channel is open or closed, driven only by the bus
 *        events (Broadcast / Stop) — it never touches the sender.
 */

#include <QDockWidget>

#include "loomchannel/connection.hpp"
#include "loomchannel/message_event.hpp"

class QLabel;
class QTextEdit;

class ReceiverDock : public QDockWidget {
    Q_OBJECT

public:
    explicit ReceiverDock(QWidget* parent = nullptr);
    ~ReceiverDock() override;

private:
    void set_state(bool open);
    void append_log(const QString& line);

    QLabel* state_;
    QTextEdit* log_;

    // RAII subscription: disconnects automatically in the destructor.
    loomchannel::Connection connection_;
};
