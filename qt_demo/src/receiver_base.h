#pragma once

/**
 * @file receiver_base.h
 * @brief Shared plumbing for receiver docks: state indicator, message
 *        log and the bus subscription. Subclasses build their widgets,
 *        then call subscribe() with whatever constraints they want.
 */

#include <memory>
#include <vector>

#include <QDockWidget>

#include "loomchannel/connection.hpp"
#include "loomchannel/constraint.hpp"
#include "loomchannel/message_event.hpp"

class QLabel;
class QTextEdit;
class QVBoxLayout;

class ReceiverBase : public QDockWidget {
    Q_OBJECT

public:
    ReceiverBase(const QString& title, const QString& note,
                 QWidget* parent = nullptr);
    ~ReceiverBase() override;

protected:
    // Subclasses call this exactly once, after building their widgets,
    // because a constraint may need to point at them.
    void subscribe(
        std::vector<std::shared_ptr<loomchannel::Constraint>> constraints = {});

    void set_state(bool open);
    void set_note(const QString& note);
    // Inserts a widget between the note and the message log.
    void insert_header_widget(QWidget* widget);
    void append_log(const QString& line);

private:
    QLabel* state_;
    QLabel* note_label_;
    QTextEdit* log_;
    QVBoxLayout* layout_;
    bool subscribed_ = false;

    // RAII subscription: disconnects automatically in the destructor.
    loomchannel::Connection connection_;
};
