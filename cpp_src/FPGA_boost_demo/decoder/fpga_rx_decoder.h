#pragma once

#include "../common/shared_types.h"
#include <cstdint>

class FPGARxDecoder {
public:
    FPGARxDecoder() = default;
    void decodeRawRecord(const uint8_t* record, FPGAEventDesc& event) const;

private:
};
