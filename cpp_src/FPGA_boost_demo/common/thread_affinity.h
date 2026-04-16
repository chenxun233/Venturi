#pragma once

#include <initializer_list>
#include <sched.h>

cpu_set_t buildSingleCpuSet(int cpu_id);
void clearCpuSetEntries(cpu_set_t& mask, std::initializer_list<int> cpu_ids);
bool hasAnyCpuSetEntries(const cpu_set_t& mask);
cpu_set_t readCurrentThreadCpuSet();
void pinCurrentThreadToMask(const cpu_set_t& mask);
void pinCurrentThreadToCpu(int cpu_id);
