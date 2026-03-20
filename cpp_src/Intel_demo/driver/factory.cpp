#include "factory.h"

std::unique_ptr<Intel82599Dev> createDevice( const std::string pci_addr, 
                                        const uint8_t num_of_quueue, 
                                        const uint16_t num_of_buf, 
                                        const uint32_t buf_size, 
                                        const uint64_t INTERRUPT_INITIAL_INTERVAL, 
                                        const uint32_t timeout_ms) {
    std::unique_ptr<Intel82599Dev> device1 = std::make_unique<Intel82599Dev>(pci_addr);
    device1->initHardware();
    device1->setRxRingBuffers(num_of_quueue,num_of_buf, buf_size);
    device1->setTxRingBuffers(num_of_quueue,num_of_buf, buf_size);
    device1->initializeInterrupt(INTERRUPT_INITIAL_INTERVAL, timeout_ms);
    device1->enableDevQueues()       ;
    device1->enableDevInterrupt()    ;
    device1->setPromisc(true)        ;
    device1->wait4Link()             ;
    return device1;
}
