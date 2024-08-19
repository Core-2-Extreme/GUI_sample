#include <stdbool.h>
#include <stdint.h>

extern "C"
{
#include "3ds.h"
#include "system/menu.h"
}


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
