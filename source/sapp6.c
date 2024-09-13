//Includes.
#include "sapp6.h"

#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include "system/menu.h"
#include "system/sem.h"
#include "system/draw/draw.h"
#include "system/util/cpu_usage.h"
#include "system/util/err.h"
#include "system/util/expl.h"
#include "system/util/hid.h"
#include "system/util/log.h"
#include "system/util/str.h"
#include "system/util/thread_types.h"
#include "system/util/util.h"
#include "system/util/watch.h"

//Defines.
//N/A.

//Typedefs.
//N/A.

//Prototypes.
static void Sapp6_draw_init_exit_message(void);
static void Sapp6_init_thread(void* arg);
static void Sapp6_exit_thread(void* arg);
static void Sapp6_worker_thread(void* arg);

//Variables.
bool sapp6_main_run = false;
bool sapp6_thread_run = false;
bool sapp6_already_init = false;
bool sapp6_thread_suspend = true;
Thread sapp6_init_thread = NULL, sapp6_exit_thread = NULL, sapp6_worker_thread = NULL;
Str_data sapp6_status = { 0, };
Str_data sapp6_msg[DEF_SAPP6_NUM_OF_MSG] = { 0, };

//Code.
bool Sapp6_query_init_flag(void)
{
	return sapp6_already_init;
}

bool Sapp6_query_running_flag(void)
{
	return sapp6_main_run;
}

void Sapp6_hid(Hid_info key)
{
	Sem_config config = { 0, };

	//Do nothing if app is suspended.
	if(aptShouldJumpToHome())
		return;

	Sem_get_config(&config);

	if(Util_err_query_error_show_flag())
		Util_err_main(key);
	else if(Util_expl_query_show_flag())
		Util_expl_main(key, config.scroll_speed);
	else
	{
		if(Util_hid_is_pressed(key, *Draw_get_bot_ui_button()))
			Draw_get_bot_ui_button()->selected = true;
		else if (key.p_start || (Util_hid_is_released(key, *Draw_get_bot_ui_button()) && Draw_get_bot_ui_button()->selected))
			Sapp6_suspend();
	}

	if(!key.p_touch && !key.h_touch)
		Draw_get_bot_ui_button()->selected = false;

	if(Util_log_query_log_show_flag())
		Util_log_main(key);
}

void Sapp6_resume(void)
{
	sapp6_thread_suspend = false;
	sapp6_main_run = true;
	Draw_set_refresh_needed(true);
	Menu_suspend();
}

void Sapp6_suspend(void)
{
	sapp6_thread_suspend = true;
	sapp6_main_run = false;
	Draw_set_refresh_needed(true);
	Menu_resume();
}

uint32_t Sapp6_load_msg(const char* lang)
{
	char file_name[32] = { 0, };

	snprintf(file_name, sizeof(file_name), "sapp6_%s.txt", (lang ? lang : ""));
	return Util_load_msg(file_name, sapp6_msg, DEF_SAPP6_NUM_OF_MSG);
}

void Sapp6_init(bool draw)
{
	DEF_LOG_STRING("Initializing...");
	uint32_t result = DEF_ERR_OTHER;
	Sem_state state = { 0, };

	Sem_get_state(&state);
	DEF_LOG_RESULT_SMART(result, Util_str_init(&sapp6_status), (result == DEF_SUCCESS), result);

	Util_watch_add(WATCH_HANDLE_SUB_APP6, &sapp6_status.sequencial_id, sizeof(sapp6_status.sequencial_id));

	if(DEF_SEM_MODEL_IS_NEW(state.console_model) && Util_is_core_available(2))
		sapp6_init_thread = threadCreate(Sapp6_init_thread, NULL, DEF_THREAD_STACKSIZE, DEF_THREAD_PRIORITY_NORMAL, 2, false);
	else
	{
		APT_SetAppCpuTimeLimit(80);
		sapp6_init_thread = threadCreate(Sapp6_init_thread, NULL, DEF_THREAD_STACKSIZE, DEF_THREAD_PRIORITY_NORMAL, 1, false);
	}

	while(!sapp6_already_init)
	{
		if(draw)
			Sapp6_draw_init_exit_message();
		else
			Util_sleep(20000);
	}

	if(!DEF_SEM_MODEL_IS_NEW(state.console_model) || !Util_is_core_available(2))
		APT_SetAppCpuTimeLimit(10);

	DEF_LOG_RESULT_SMART(result, threadJoin(sapp6_init_thread, DEF_THREAD_WAIT_TIME), (result == DEF_SUCCESS), result);
	threadFree(sapp6_init_thread);

	Util_str_clear(&sapp6_status);
	Sapp6_resume();

	DEF_LOG_STRING("Initialized.");
}

void Sapp6_exit(bool draw)
{
	DEF_LOG_STRING("Exiting...");
	uint32_t result = DEF_ERR_OTHER;

	sapp6_exit_thread = threadCreate(Sapp6_exit_thread, NULL, DEF_THREAD_STACKSIZE, DEF_THREAD_PRIORITY_NORMAL, 1, false);

	while(sapp6_already_init)
	{
		if(draw)
			Sapp6_draw_init_exit_message();
		else
			Util_sleep(20000);
	}

	DEF_LOG_RESULT_SMART(result, threadJoin(sapp6_exit_thread, DEF_THREAD_WAIT_TIME), (result == DEF_SUCCESS), result);
	threadFree(sapp6_exit_thread);

	Util_watch_remove(WATCH_HANDLE_SUB_APP6, &sapp6_status.sequencial_id);
	Util_str_free(&sapp6_status);
	Draw_set_refresh_needed(true);

	DEF_LOG_STRING("Exited.");
}

void Sapp6_main(void)
{
	uint32_t color = DEF_DRAW_BLACK;
	uint32_t back_color = DEF_DRAW_WHITE;
	Watch_handle_bit watch_handle_bit = (DEF_WATCH_HANDLE_BIT_GLOBAL | DEF_WATCH_HANDLE_BIT_SUB_APP6);
	Sem_config config = { 0, };
	Sem_state state = { 0, };

	Sem_get_config(&config);
	Sem_get_state(&state);

	if (config.is_night)
	{
		color = DEF_DRAW_WHITE;
		back_color = DEF_DRAW_BLACK;
	}

	//Check if we should update the screen.
	if(Util_watch_is_changed(watch_handle_bit) || Draw_is_refresh_needed() || !config.is_eco)
	{
		Draw_set_refresh_needed(false);
		Draw_frame_ready();

		if(config.is_top_lcd_on)
		{
			Draw_screen_ready(DRAW_SCREEN_TOP_LEFT, back_color);

			Draw(&sapp6_msg[0], 0, 20, 0.5, 0.5, color);
			if(Util_log_query_log_show_flag())
				Util_log_draw();

			Draw_top_ui(config.is_eco, state.is_charging, state.wifi_signal, state.battery_level, state.msg);

			if(config.is_debug)
				Draw_debug_info(config.is_night, state.free_ram, state.free_linear_ram);

			if(Util_cpu_usage_query_show_flag())
				Util_cpu_usage_draw();

			if(Draw_is_3d_mode())
			{
				Draw_screen_ready(DRAW_SCREEN_TOP_RIGHT, back_color);

				if(Util_log_query_log_show_flag())
					Util_log_draw();

				Draw_top_ui(config.is_eco, state.is_charging, state.wifi_signal, state.battery_level, state.msg);

				if(config.is_debug)
					Draw_debug_info(config.is_night, state.free_ram, state.free_linear_ram);

				if(Util_cpu_usage_query_show_flag())
					Util_cpu_usage_draw();
			}
		}

		if(config.is_bottom_lcd_on)
		{
			Draw_screen_ready(DRAW_SCREEN_BOTTOM, back_color);

			Draw_c(DEF_SAPP6_VER, 0, 0, 0.4, 0.4, DEF_DRAW_GREEN);

			if(Util_expl_query_show_flag())
				Util_expl_draw();

			if(Util_err_query_error_show_flag())
				Util_err_draw();

			Draw_bot_ui();
		}

		Draw_apply_draw();
	}
	else
		gspWaitForVBlank();
}

static void Sapp6_draw_init_exit_message(void)
{
	uint32_t color = DEF_DRAW_BLACK;
	uint32_t back_color = DEF_DRAW_WHITE;
	Watch_handle_bit watch_handle_bit = (DEF_WATCH_HANDLE_BIT_GLOBAL | DEF_WATCH_HANDLE_BIT_SUB_APP6);
	Sem_config config = { 0, };
	Sem_state state = { 0, };

	Sem_get_config(&config);
	Sem_get_state(&state);

	if (config.is_night)
	{
		color = DEF_DRAW_WHITE;
		back_color = DEF_DRAW_BLACK;
	}

	//Check if we should update the screen.
	if(Util_watch_is_changed(watch_handle_bit) || Draw_is_refresh_needed() || !config.is_eco)
	{
		Draw_set_refresh_needed(false);
		Draw_frame_ready();

		Draw_screen_ready(DRAW_SCREEN_TOP_LEFT, back_color);

		if(Util_log_query_log_show_flag())
			Util_log_draw();

		Draw_top_ui(config.is_eco, state.is_charging, state.wifi_signal, state.battery_level, state.msg);

		if(config.is_debug)
			Draw_debug_info(config.is_night, state.free_ram, state.free_linear_ram);

		if(Util_cpu_usage_query_show_flag())
			Util_cpu_usage_draw();

		Draw(&sapp6_status, 0, 20, 0.65, 0.65, color);

		//Draw the same things on right screen if 3D mode is enabled.
		//So that user can easily see them.
		if(Draw_is_3d_mode())
		{
			Draw_screen_ready(DRAW_SCREEN_TOP_RIGHT, back_color);

			if(Util_log_query_log_show_flag())
				Util_log_draw();

			Draw_top_ui(config.is_eco, state.is_charging, state.wifi_signal, state.battery_level, state.msg);

			if(config.is_debug)
				Draw_debug_info(config.is_night, state.free_ram, state.free_linear_ram);

			if(Util_cpu_usage_query_show_flag())
				Util_cpu_usage_draw();

			Draw(&sapp6_status, 0, 20, 0.65, 0.65, color);
		}

		Draw_apply_draw();
	}
	else
		gspWaitForVBlank();
}

static void Sapp6_init_thread(void* arg)
{
	(void)arg;
	DEF_LOG_STRING("Thread started.");
	// uint32_t result = DEF_ERR_OTHER;

	Util_str_set(&sapp6_status, "Initializing variables...");
	//Empty.

	Util_str_add(&sapp6_status, "\nInitializing queue...");
	//Empty.

	Util_str_add(&sapp6_status, "\nStarting threads...");
	sapp6_thread_run = true;
	sapp6_worker_thread = threadCreate(Sapp6_worker_thread, NULL, DEF_THREAD_STACKSIZE, DEF_THREAD_PRIORITY_NORMAL, 1, false);

	sapp6_already_init = true;

	DEF_LOG_STRING("Thread exit.");
	threadExit(0);
}

static void Sapp6_exit_thread(void* arg)
{
	(void)arg;
	DEF_LOG_STRING("Thread started.");
	uint32_t result = DEF_ERR_OTHER;

	sapp6_thread_suspend = false;
	sapp6_thread_run = false;

	Util_str_set(&sapp6_status, "Exiting threads...");
	DEF_LOG_RESULT_SMART(result, threadJoin(sapp6_worker_thread, DEF_THREAD_WAIT_TIME), (result == DEF_SUCCESS), result);

	Util_str_add(&sapp6_status, "\nCleaning up...");
	threadFree(sapp6_worker_thread);

	sapp6_already_init = false;

	DEF_LOG_STRING("Thread exit.");
	threadExit(0);
}

static void Sapp6_worker_thread(void* arg)
{
	(void)arg;
	DEF_LOG_STRING("Thread started.");

	while (sapp6_thread_run)
	{
		if(false)
		{

		}
		else
			Util_sleep(DEF_THREAD_ACTIVE_SLEEP_TIME);

		while (sapp6_thread_suspend)
			Util_sleep(DEF_THREAD_INACTIVE_SLEEP_TIME);
	}

	DEF_LOG_STRING("Thread exit.");
	threadExit(0);
}
