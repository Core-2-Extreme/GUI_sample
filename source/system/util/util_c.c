#include <stdbool.h>
#include <stdint.h>

#include <3ds/svc.h>

//Include myself.
#include "system/util/util_c.h"

void Util_sleep(int64_t us)
{
	svcSleepThread(us * 1000);
}
