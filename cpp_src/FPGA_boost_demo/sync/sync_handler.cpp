#include "sync_handler.h"

SyncHandler::SyncHandler(uint64_t trigger_period):
      m_trigger_period(trigger_period),
      m_trigger_countdown(0){ 
}

void SyncHandler::run(CapSignal& cap_signal) {
    if (m_trigger_period == 0) {
        return;
    }
    if (m_trigger_countdown == 0) {
        cap_signal.request.store(true, std::memory_order_release);
        m_trigger_countdown = m_trigger_period - 1;
        return;
    }
    --m_trigger_countdown;
}
