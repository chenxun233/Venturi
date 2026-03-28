#pragma once

#include "../common/shared_types.h"

class TxEngine {
public:
    TxEngine() = default;

    void sendIntent(const OrderIntent& intent) const;
};
