#pragma once

/**
 * @file receiver_base.h
 * @brief Shared plumbing for receiver docks: state indicator, message
 *        log and the bus subscription. Subclasses only choose the
 *        per-subscriber constraints (and may show a note above the log).
 */

#include <memory>
#include <vector>

#include <QDockWidget>

#include "loomchannel/connection.hpp"
#include "loomchannel/constraint.hpp"
#include "loomchannel/message_event.hpp"

class QLabel;
class QTextEdit;

class ReceiverBase : public QDockWidget {
    Q_OBJECT

public:
    ReceiverBase(const QString& title,
                 std::vector<std::shared_ptr<loomchannel::Constraint>> constraints,
                 const QString& note,
                 QWidget* parent = nullptr);
    ~ReceiverBase() override;

protected:
    void set_state(bool open);
    void append_log(const QString& line);

private:
    QLabel* state_;
    QTextEdit* log_;

    // RAII subscription: disconnects automatically in the destructor.
    loomchannel::Connection connection_;
};
