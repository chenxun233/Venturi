#include "spsc_ring_queue.h"

template class SpscRingBuffer<TimeRecord>;
template class SpscRingBuffer<TraceCommand>;
template class SpscRingBuffer<OrderIntent>;
template class SpscRingBuffer<LatencyLogRecord>;
template class SpscRingBuffer<TxLogRecord>;
