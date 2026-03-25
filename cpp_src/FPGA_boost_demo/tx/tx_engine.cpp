#include "tx_engine.h"

TxEngine::TxEngine(uint16_t queue_idx)
    : m_queue_idx(queue_idx) {
}

void TxEngine::sendBatch(const FPGAEventDesc* events, std::size_t count) {
    (void)events;
    (void)count;
}


