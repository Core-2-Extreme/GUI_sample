#include <stdbool.h>
#include <stdint.h>

extern "C"
{
#include <3ds/types.h>
#include <3ds/services/apt.h>
}

#include "system/menu.hpp"


int main()
{
	Menu_init();

	// Main loop
	while (aptMainLoop())
	{
		if (Menu_query_must_exit_flag())
			break;

		Menu_main();
	}

	Menu_exit();
	return 0;
}
