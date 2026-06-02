#pragma once

#include <initializer_list>
#include <sched.h>



void pinCurrentThreadToCpu(int cpu_id);
