#include <stdbool.h>
#include <stdint.h>

#include "system/types.hpp"

#include "system/menu.hpp"
#include "system/variables.hpp"

#include "system/draw/draw.hpp"

#include "system/util/util.hpp"

extern "C"
{
	#include "system/util/error.h"
	#include "system/util/explorer.h"
	#include "system/util/hid.h"
	#include "system/util/log.h"
	#include "system/util/str.h"
	#include "system/util/thread_types.h"
}

//Include myself.
#include "sub_app7.hpp"


bool sapp7_main_run = false;
bool sapp7_thread_run = false;
bool sapp7_already_init = false;
bool sapp7_thread_suspend = true;
Thread sapp7_init_thread = NULL, sapp7_exit_thread = NULL, sapp7_worker_thread = NULL;
Util_str sapp7_status = { 0, };
Util_str sapp7_msg[DEF_SAPP7_NUM_OF_MSG] = { 0, };


static void Sapp7_draw_init_exit_message(void);
static void Sapp7_init_thread(void* arg);
static void Sapp7_exit_thread(void* arg);
static void Sapp7_worker_thread(void* arg);


bool Sapp7_query_init_flag(void)
{
	return sapp7_already_init;
}

bool Sapp7_query_running_flag(void)
{
	return sapp7_main_run;
}

void Sapp7_hid(Hid_info key)
{
	//Do nothing if app is suspended.
	if(aptShouldJumpToHome())
		return;

	if(Util_err_query_error_show_flag())
		Util_err_main(key);
	else if(Util_expl_query_show_flag())
		Util_expl_main(key);
	else
	{
		if(Util_hid_is_pressed(key, *Draw_get_bot_ui_button()))
			Draw_get_bot_ui_button()->selected = true;
		else if (key.p_start || (Util_hid_is_released(key, *Draw_get_bot_ui_button()) && Draw_get_bot_ui_button()->selected))
			Sapp7_suspend();
	}

	if(!key.p_touch && !key.h_touch)
		Draw_get_bot_ui_button()->selected = false;

	if(Util_log_query_log_show_flag())
		Util_log_main(key);
}

void Sapp7_resume(void)
{
	sapp7_thread_suspend = false;
	sapp7_main_run = true;
	var_need_reflesh = true;
	Menu_suspend();
}

void Sapp7_suspend(void)
{
	sapp7_thread_suspend = true;
	sapp7_main_run = false;
	var_need_reflesh = true;
	Menu_resume();
}

uint32_t Sapp7_load_msg(const char* lang)
{
	char file_name[32] = { 0, };

	snprintf(file_name, sizeof(file_name), "sapp7_%s.txt", (lang ? lang : ""));
	return Util_load_msg(file_name, sapp7_msg, DEF_SAPP7_NUM_OF_MSG);
}

void Sapp7_init(bool draw)
{
	DEF_LOG_STRING("Initializing...");
	uint32_t result = DEF_ERR_OTHER;

	DEF_LOG_RESULT_SMART(result, Util_str_init(&sapp7_status), (result == DEF_SUCCESS), result);

	Util_add_watch(WATCH_HANDLE_SUB_APP7, &sapp7_status.sequencial_id, sizeof(sapp7_status.sequencial_id));

	if((var_model == CFG_MODEL_N2DSXL || var_model == CFG_MODEL_N3DSXL || var_model == CFG_MODEL_N3DS) && var_core_2_available)
		sapp7_init_thread = threadCreate(Sapp7_init_thread, (void*)(""), DEF_STACKSIZE, DEF_THREAD_PRIORITY_NORMAL, 2, false);
	else
	{
		APT_SetAppCpuTimeLimit(80);
		sapp7_init_thread = threadCreate(Sapp7_init_thread, (void*)(""), DEF_STACKSIZE, DEF_THREAD_PRIORITY_NORMAL, 1, false);
	}

	while(!sapp7_already_init)
	{
		if(draw)
			Sapp7_draw_init_exit_message();
		else
			Util_sleep(20000);
	}

	if(!(var_model == CFG_MODEL_N2DSXL || var_model == CFG_MODEL_N3DSXL || var_model == CFG_MODEL_N3DS) || !var_core_2_available)
		APT_SetAppCpuTimeLimit(10);

	DEF_LOG_RESULT_SMART(result, threadJoin(sapp7_init_thread, DEF_THREAD_WAIT_TIME), (result == DEF_SUCCESS), result);
	threadFree(sapp7_init_thread);

	Util_str_clear(&sapp7_status);
	Sapp7_resume();

	DEF_LOG_STRING("Initialized.");
}

void Sapp7_exit(bool draw)
{
	DEF_LOG_STRING("Exiting...");
	uint32_t result = DEF_ERR_OTHER;

	sapp7_exit_thread = threadCreate(Sapp7_exit_thread, (void*)(""), DEF_STACKSIZE, DEF_THREAD_PRIORITY_NORMAL, 1, false);

	while(sapp7_already_init)
	{
		if(draw)
			Sapp7_draw_init_exit_message();
		else
			Util_sleep(20000);
	}

	DEF_LOG_RESULT_SMART(result, threadJoin(sapp7_exit_thread, DEF_THREAD_WAIT_TIME), (result == DEF_SUCCESS), result);
	threadFree(sapp7_exit_thread);

	Util_remove_watch(WATCH_HANDLE_SUB_APP7, &sapp7_status.sequencial_id);
	Util_str_free(&sapp7_status);
	var_need_reflesh = true;

	DEF_LOG_STRING("Exited.");
}

void Sapp7_main(void)
{
	int color = DEF_DRAW_BLACK;
	int back_color = DEF_DRAW_WHITE;
	Watch_handle_bit watch_handle_bit = (DEF_WATCH_HANDLE_BIT_GLOBAL | DEF_WATCH_HANDLE_BIT_SUB_APP7);

	if (var_night_mode)
	{
		color = DEF_DRAW_WHITE;
		back_color = DEF_DRAW_BLACK;
	}

	//Check if we should update the screen.
	if(Util_is_watch_changed(watch_handle_bit) || var_need_reflesh || !var_eco_mode)
	{
		var_need_reflesh = false;
		Draw_frame_ready();

		if(var_turn_on_top_lcd)
		{
			Draw_screen_ready(DRAW_SCREEN_TOP_LEFT, back_color);

			Draw(&sapp7_msg[0], 0, 20, 0.5, 0.5, color);
			if(Util_log_query_log_show_flag())
				Util_log_draw();

			Draw_top_ui();

			if(var_monitor_cpu_usage)
				Draw_cpu_usage_info();

			if(Draw_is_3d_mode())
			{
				Draw_screen_ready(DRAW_SCREEN_TOP_RIGHT, back_color);

				if(Util_log_query_log_show_flag())
					Util_log_draw();

				Draw_top_ui();

				if(var_monitor_cpu_usage)
					Draw_cpu_usage_info();
			}
		}

		if(var_turn_on_bottom_lcd)
		{
			Draw_screen_ready(DRAW_SCREEN_BOTTOM, back_color);

			Draw_c(DEF_SAPP7_VER, 0, 0, 0.4, 0.4, DEF_DRAW_GREEN);

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

static void Sapp7_draw_init_exit_message(void)
{
	int color = DEF_DRAW_BLACK;
	int back_color = DEF_DRAW_WHITE;
	Watch_handle_bit watch_handle_bit = (DEF_WATCH_HANDLE_BIT_GLOBAL | DEF_WATCH_HANDLE_BIT_SUB_APP7);

	if (var_night_mode)
	{
		color = DEF_DRAW_WHITE;
		back_color = DEF_DRAW_BLACK;
	}

	//Check if we should update the screen.
	if(Util_is_watch_changed(watch_handle_bit) || var_need_reflesh || !var_eco_mode)
	{
		var_need_reflesh = false;
		Draw_frame_ready();

		Draw_screen_ready(DRAW_SCREEN_TOP_LEFT, back_color);

		if(Util_log_query_log_show_flag())
			Util_log_draw();

		Draw_top_ui();
		if(var_monitor_cpu_usage)
			Draw_cpu_usage_info();

		Draw(&sapp7_status, 0, 20, 0.65, 0.65, color);

		//Draw the same things on right screen if 3D mode is enabled.
		//So that user can easily see them.
		if(Draw_is_3d_mode())
		{
			Draw_screen_ready(DRAW_SCREEN_TOP_RIGHT, back_color);

			if(Util_log_query_log_show_flag())
				Util_log_draw();

			Draw_top_ui();
			if(var_monitor_cpu_usage)
				Draw_cpu_usage_info();

			Draw(&sapp7_status, 0, 20, 0.65, 0.65, color);
		}

		Draw_apply_draw();
	}
	else
		gspWaitForVBlank();
}

static void Sapp7_init_thread(void* arg)
{
	DEF_LOG_STRING("Thread started.");
	// uint32_t result = DEF_ERR_OTHER;

	Util_str_set(&sapp7_status, "Initializing variables...");
	//Empty.

	Util_str_add(&sapp7_status, "\nInitializing queue...");
	//Empty.

	Util_str_add(&sapp7_status, "\nStarting threads...");
	sapp7_thread_run = true;
	sapp7_worker_thread = threadCreate(Sapp7_worker_thread, (void*)(""), DEF_STACKSIZE, DEF_THREAD_PRIORITY_NORMAL, 1, false);

	sapp7_already_init = true;

	DEF_LOG_STRING("Thread exit.");
	threadExit(0);
}

static void Sapp7_exit_thread(void* arg)
{
	DEF_LOG_STRING("Thread started.");
	uint32_t result = DEF_ERR_OTHER;

	sapp7_thread_suspend = false;
	sapp7_thread_run = false;

	Util_str_set(&sapp7_status, "Exiting threads...");
	DEF_LOG_RESULT_SMART(result, threadJoin(sapp7_worker_thread, DEF_THREAD_WAIT_TIME), (result == DEF_SUCCESS), result);

	Util_str_add(&sapp7_status, "\nCleaning up...");
	threadFree(sapp7_worker_thread);

	sapp7_already_init = false;

	DEF_LOG_STRING("Thread exit.");
	threadExit(0);
}

static void Sapp7_worker_thread(void* arg)
{
	DEF_LOG_STRING("Thread started.");

	while (sapp7_thread_run)
	{
		if(false)
		{

		}
		else
			Util_sleep(DEF_ACTIVE_THREAD_SLEEP_TIME);

		while (sapp7_thread_suspend)
			Util_sleep(DEF_INACTIVE_THREAD_SLEEP_TIME);
	}

	DEF_LOG_STRING("Thread exit.");
	threadExit(0);
}
