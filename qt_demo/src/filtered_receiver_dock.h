#pragma once

/**
 * @file filtered_receiver_dock.h
 * @brief Second receiver: subscribes to the same channel with a
 *        per-subscriber Constraint, so only matching messages reach it.
 */

#include "receiver_base.h"

class FilteredReceiverDock : public ReceiverBase {
public:
    explicit FilteredReceiverDock(QWidget* parent = nullptr);
};
