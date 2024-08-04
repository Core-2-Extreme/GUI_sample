extern "C"
{
#include "system/util/cpu_usage.h"
}

#if DEF_ENABLE_CPU_MONITOR_API
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>

#include "system/util/error_types.h"
#include "system/util/util.hpp"
extern "C"
{
#include "system/util/log.h"
#include "system/util/thread_types.h"
}

extern "C"
{
bool util_cpu_usage_monitor_init = false;
bool util_cpu_usage_reset_counter_request[4] = { false, false, false, false, };
uint8_t util_cpu_usage_core_id[4] = { 0, 1, 2, 3, };
uint16_t util_cpu_usage_counter_cache[4] = { 0, 0, 0, 0, };
uint32_t util_cpu_usage_max_core_1 = 0;
float util_cpu_usage_per_core[4] = { NAN, NAN, NAN, NAN, };
float util_cpu_usage = NAN;
Thread util_cpu_usage_thread_handle[5] = { 0, 0, 0, 0, };
Handle timer_handle = 0;

void Util_cpu_usage_counter_thread(void* arg);
void Util_cpu_usage_calculate_thread(void* arg);

uint32_t Util_cpu_usage_monitor_init(void)
{
	if(util_cpu_usage_monitor_init)
		goto already_inited;

	util_cpu_usage_monitor_init = true;
	for(int i = 0; i < 4; i++)
	{
		//This may fail depending on core availability.
		util_cpu_usage_thread_handle[i] = threadCreate(Util_cpu_usage_counter_thread, &util_cpu_usage_core_id[i], 2048, DEF_SYSTEM_THREAD_PRIORITY_IDLE, i, false);
	}

	util_cpu_usage_thread_handle[4] = threadCreate(Util_cpu_usage_calculate_thread, NULL, 2048, DEF_SYSTEM_THREAD_PRIORITY_REALTIME, 0, false);
	if(!util_cpu_usage_thread_handle[4])
	{
		DEF_LOG_RESULT(threadCreate, false, DEF_ERR_OTHER);
		goto nintendo_api_failed;
	}

	return DEF_SUCCESS;

	nintendo_api_failed:
	util_cpu_usage_monitor_init = false;
	return DEF_ERR_OTHER;

	already_inited:
	return DEF_ERR_ALREADY_INITIALIZED;
}

void Util_cpu_usage_monitor_exit(void)
{
	if(!util_cpu_usage_monitor_init)
		return;

	util_cpu_usage_monitor_init = false;
	svcSignalEvent(timer_handle);
	for(int i = 0; i < 5; i++)
	{
		if(util_cpu_usage_thread_handle[i])
		{
			threadJoin(util_cpu_usage_thread_handle[i], DEF_THREAD_WAIT_TIME);
			threadFree(util_cpu_usage_thread_handle[i]);
		}
	}
}

float Util_cpu_usage_monitor_get_cpu_usage(int8_t core_id)
{
	if(!util_cpu_usage_monitor_init)
		return NAN;

	if(core_id < -1 || core_id > 3)
		return NAN;

	if(core_id == -1)
		return util_cpu_usage;
	else
		return util_cpu_usage_per_core[core_id];
}


void Util_cpu_usage_counter_thread(void* arg)
{
	uint8_t core_id = 0;

	if(!arg || *(uint8_t*)arg > 3)
	{
		DEF_LOG_STRING("Invalid arg!!!!!");
		DEF_LOG_STRING("Thread exit.");
		threadExit(0);
	}

	core_id = *(uint8_t*)arg;
	DEF_LOG_FORMAT("#%" PRIu8 " thread started.", core_id);

	//This thread will run at the lowest priority.
	while(util_cpu_usage_monitor_init)
	{
		//1ms
		Util_sleep(1000);
		//In ideal condition (CPU usage is 0%), it should be 1000 in 1000ms.
		util_cpu_usage_counter_cache[core_id]++;

		if(util_cpu_usage_reset_counter_request[core_id])
		{
			util_cpu_usage_counter_cache[core_id] = 0;
			util_cpu_usage_reset_counter_request[core_id] = false;
		}
	}

	DEF_LOG_FORMAT("#%" PRIu8 " thread exit.", core_id);
	threadExit(0);
}

void Util_cpu_usage_calculate_thread(void* arg)
{
	DEF_LOG_STRING("Thread started.");

	uint8_t div = 0;
	float total_cpu_usage = 0;
	float cpu_usage_cache = 0;

	svcCreateTimer(&timer_handle, RESET_PULSE);
	svcSetTimer(timer_handle, 0, 1000000000);//1000ms

	while(util_cpu_usage_monitor_init)
	{
		total_cpu_usage = 0;
		div = 0;

		//Update cpu usage every 1000ms.
		svcWaitSynchronization(timer_handle, U64_MAX);

		for(int i = 0; i < 4; i++)
		{
			//Core is not available
			if(!util_cpu_usage_thread_handle[i])
				util_cpu_usage_per_core[i] = NAN;
			else
			{
				//If this flag is not cleared here, it means core usage is kept 100% for a second so that it couldn't reset counter.
				if(util_cpu_usage_reset_counter_request[i])
					cpu_usage_cache = 100;
				else//Estimated CPU usage (%) = (1000 - util_cpu_usage_counter_cache) / 10.0
					cpu_usage_cache = util_cpu_usage_per_core[i] = (1000 - (util_cpu_usage_counter_cache[i] > 1000 ? 1000 : util_cpu_usage_counter_cache[i])) / 10.0;

				if(i == 1)
				{
					if(Util_get_core_1_max() != 0)
						util_cpu_usage_per_core[i] = cpu_usage_cache / (100.0 / Util_get_core_1_max());
					else
						util_cpu_usage_per_core[i] = 0;
				}
				else
					util_cpu_usage_per_core[i] = cpu_usage_cache;

				total_cpu_usage += util_cpu_usage_per_core[i];
				div++;
				util_cpu_usage_reset_counter_request[i] = true;
			}
		}

		if(div != 0)
			util_cpu_usage = total_cpu_usage / div;
	}

	svcCancelTimer(timer_handle);
	svcCloseHandle(timer_handle);

	DEF_LOG_STRING("Thread exit.");
	threadExit(0);
}
}
#endif //DEF_ENABLE_CPU_MONITOR_API
