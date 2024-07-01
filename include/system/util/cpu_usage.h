#if !defined(CPU_USAGE_H)
#define CPU_USAGE_H
#include <stdbool.h>
#include <stdint.h>

#if DEF_ENABLE_CPU_MONITOR_API

extern "C"
{
/**
 * @brief Initialize cpu usage monitor API.
 * @return On success DEF_SUCCESS, on failure DEF_ERR_* or Nintendo API's error.
 * @warning Thread dangerous (untested)
*/
uint32_t Util_cpu_usage_monitor_init(void);

/**
 * @brief Uninitialize cpu usage monitor API.
 * Do nothing if cpu usage monitor api is not initialized.
 * @warning Thread dangerous (untested)
*/
void Util_cpu_usage_monitor_exit(void);

/**
 * @brief Get cpu usage.
 * Always return NAN if cpu usage monitor api is not initialized.
 * @param core_id (in) CPU core, -1 means all CPU.
 * @return Counter interval in ms.
 * @warning Thread dangerous (untested)
*/
float Util_cpu_usage_monitor_get_cpu_usage(int8_t core_id);

#else

#define Util_cpu_usage_monitor_init() DEF_ERR_DISABLED
#define Util_cpu_usage_monitor_exit()
#define Util_cpu_usage_monitor_get_cpu_usage(...) NAN

#endif //DEF_ENABLE_CPU_MONITOR_API
}
#endif //!defined(CPU_USAGE_H)
