#pragma once

/**
 * @file keyword_pane.h
 * @brief Output pane with two named constraints composed on one
 *        subscription (keyword + max length), with the keyword retargeted
 *        from the UI without resubscribing.
 */

#include <memory>

#include "receiver_pane.h"
#include "text_constraints.h"

class QLineEdit;

class KeywordPane : public ReceiverPane {
public:
    explicit KeywordPane(QWidget* parent = nullptr);

private:
    QLineEdit* keyword_edit_;

    // The constraint is mutated in place when the keyword changes; the
    // subscription stays untouched.
    std::shared_ptr<loomdemo::KeywordConstraint> constraint_;
};
