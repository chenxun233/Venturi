#include "thread_affinity.h"

#include <cerrno>
#include <pthread.h>
#include <system_error>

cpu_set_t buildSingleCpuSet(int cpu_id) {
    cpu_set_t mask;
    CPU_ZERO(&mask);
    CPU_SET(cpu_id, &mask);
    return mask;
}

void clearCpuSetEntries(cpu_set_t& mask, std::initializer_list<int> cpu_ids) {
    for (const int cpu_id : cpu_ids) {
        CPU_CLR(cpu_id, &mask);
    }
}

bool hasAnyCpuSetEntries(const cpu_set_t& mask) {
    return CPU_COUNT(&mask) > 0;
}

cpu_set_t readCurrentThreadCpuSet() {
    cpu_set_t mask;
    CPU_ZERO(&mask);
    const int rc = sched_getaffinity(0, sizeof(mask), &mask);
    if (rc != 0) {
        throw std::system_error(errno, std::generic_category(), "sched_getaffinity");
    }
    return mask;
}

void pinCurrentThreadToMask(const cpu_set_t& mask) {
    const int rc = pthread_setaffinity_np(pthread_self(), sizeof(mask), &mask);
    if (rc != 0) {
        throw std::system_error(rc, std::generic_category(), "pthread_setaffinity_np");
    }
}

void pinCurrentThreadToCpu(int cpu_id) {
    const cpu_set_t mask = buildSingleCpuSet(cpu_id);
    pinCurrentThreadToMask(mask);
}
