#pragma once
#include "../driver/fpga_dev.h"
#include "../decoder/fpga_rx_decoder.h"
#include "../common/shared_types.h"
#include "../latency/latency_log_printer.h"
#include "../latency/trace_buffer.h"
#include <array>
#include <cstddef>
#include <cstdint>
#include <ctime>

class Regression;







class FPGARxEngine{

public:
    explicit        FPGARxEngine(FPGADev& device, uint16_t que_idx);
    void            attachTraceBuffer(TraceBuffer& trace_buffer) { m_trace_buffer = &trace_buffer; }
    void            attachLogPrinter(LatencyLogPrinter& log_printer) { m_log_printer = &log_printer; }
    void            attachRegression(Regression& regression) { m_regression = &regression; }
    std::size_t     pollBatch(std::size_t batch_size, bool get_time);
    std::size_t     pollBatchSync(std::size_t batch_size,
                                  bool get_time,
                                  FpgaSyncSnapshot& snapshot);
    const std::array<FPGAEventDesc, MAX_POLL_RECORDS>& readEventBuffer() const;


private:
    timespec            m_ts_captured {};
    FPGARxDecoder       m_decoder;
    TraceBuffer*        m_trace_buffer{nullptr};
    LatencyLogPrinter*  m_log_printer{nullptr};
    Regression*         m_regression{nullptr};
    uint16_t            m_que_idx {0};
    std::array<FPGAEventDesc, MAX_POLL_RECORDS> m_event_buffer;


};
