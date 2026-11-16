#pragma once

/**
 * @file receiver_dock.h
 * @brief Plain receiver dock: receives every message on the channel.
 */

#include "receiver_base.h"

class ReceiverDock : public ReceiverBase {
public:
    explicit ReceiverDock(QWidget* parent = nullptr);
};
