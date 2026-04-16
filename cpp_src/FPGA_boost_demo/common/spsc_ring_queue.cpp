#include "spsc_ring_queue.h"

template class SpscRingQueue<TimeRecord>;
template class SpscRingQueue<OrderIntent>;
template class SpscRingQueue<LatencyLogRecord>;
template class SpscRingQueue<TxLogRecord>;
