#include "thread_affinity.h"

#include <cerrno>
#include <pthread.h>
#include <system_error>


void pinCurrentThreadToCpu(int cpu_id) {
    cpu_set_t mask;
    CPU_ZERO(&mask);
    CPU_SET(cpu_id, &mask);
    const int rc = pthread_setaffinity_np(pthread_self(), sizeof(mask), &mask);
    if (rc != 0) {
        throw std::system_error(rc, std::generic_category(), "pthread_setaffinity_np");
    }
}
