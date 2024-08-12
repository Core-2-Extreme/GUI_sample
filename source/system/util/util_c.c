#include "system/util/util_c.h"

#include <stdbool.h>
#include <stdint.h>

#include "3ds.h"

void Util_sleep(uint64_t us)
{
	svcSleepThread(us * 1000);
}
