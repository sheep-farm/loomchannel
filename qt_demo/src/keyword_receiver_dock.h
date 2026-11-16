#pragma once

/**
 * @file keyword_receiver_dock.h
 * @brief Third receiver: two named constraints composed on one
 *        subscription (keyword + max length), with the keyword retargeted
 *        from the UI without resubscribing.
 */

#include <memory>

#include "receiver_base.h"
#include "text_constraints.h"

class QLineEdit;

class KeywordReceiverDock : public ReceiverBase {
    Q_OBJECT

public:
    explicit KeywordReceiverDock(QWidget* parent = nullptr);

private:
    QLineEdit* keyword_edit_;

    // The constraint is mutated in place when the keyword changes; the
    // subscription stays untouched.
    std::shared_ptr<loomdemo::KeywordConstraint> constraint_;
};
