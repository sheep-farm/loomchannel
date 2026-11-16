#pragma once

/**
 * @file receiver_pane.h
 * @brief Output pane for the Qt Creator-style layout: a plain QWidget
 *        (no QDockWidget) placed in a fixed area, with the state
 *        indicator, the message log and the bus subscription.
 *
 * Subclasses build their widgets, then call subscribe() with whatever
 * constraints they want.
 */

#include <memory>
#include <vector>

#include <QWidget>

#include "loomchannel/connection.hpp"
#include "loomchannel/constraint.hpp"
#include "loomchannel/message_event.hpp"
#include "text_constraints.h"

class QLabel;
class QPlainTextEdit;
class QVBoxLayout;

class ReceiverPane : public QWidget {
    Q_OBJECT

public:
    ReceiverPane(const QString& note = {}, QWidget* parent = nullptr);
    ~ReceiverPane() override;

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
    QPlainTextEdit* log_;
    QVBoxLayout* layout_;
    bool subscribed_ = false;

    // RAII subscription: disconnects automatically in the destructor.
    loomchannel::Connection connection_;
};

/** Plain output pane: receives everything on the channel. */
class ConsolePane : public ReceiverPane {
public:
    ConsolePane() { subscribe(); }
};

/** Output pane with the demo prefix constraint. */
class JdbcPane : public ReceiverPane {
public:
    JdbcPane()
        : ReceiverPane(QObject::tr("só entrega mensagens que começam com \"jdbc:\"")) {
        subscribe({std::make_shared<loomdemo::PrefixConstraint>("jdbc:")});
    }
};
