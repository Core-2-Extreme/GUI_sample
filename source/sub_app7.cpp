#include "definitions.hpp"
#include "system/types.hpp"

#include "system/menu.hpp"
#include "system/variables.hpp"

#include "system/draw/draw.hpp"

#include "system/util/error.hpp"
#include "system/util/hid.hpp"
#include "system/util/log.hpp"
#include "system/util/util.hpp"

extern "C"
{
	#include "system/util/str.h"
}

//Include myself.
#include "sub_app7.hpp"


bool sapp7_main_run = false;
bool sapp7_thread_run = false;
bool sapp7_already_init = false;
bool sapp7_thread_suspend = true;
std::string sapp7_msg[DEF_SAPP7_NUM_OF_MSG];
Thread sapp7_init_thread, sapp7_exit_thread, sapp7_worker_thread;
Util_str sapp7_status = { 0, };


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

Result_with_string Sapp7_load_msg(std::string lang)
{
	return Util_load_msg("sapp7_" + lang + ".txt", sapp7_msg, DEF_SAPP7_NUM_OF_MSG);
}

void Sapp7_init(bool draw)
{
	Util_log_save(DEF_SAPP7_INIT_STR, "Initializing...");
	Result_with_string result;

	result.code = Util_str_init(&sapp7_status);
	Util_log_save(DEF_SAPP7_INIT_STR, "Util_str_init()..." + result.string + result.error_description, result.code);

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

	Util_log_save(DEF_SAPP7_EXIT_STR, "threadJoin()...", threadJoin(sapp7_init_thread, DEF_THREAD_WAIT_TIME));
	threadFree(sapp7_init_thread);

	Util_str_clear(&sapp7_status);
	Sapp7_resume();

	Util_log_save(DEF_SAPP7_INIT_STR, "Initialized.");
}

void Sapp7_exit(bool draw)
{
	Util_log_save(DEF_SAPP7_EXIT_STR, "Exiting...");

	sapp7_exit_thread = threadCreate(Sapp7_exit_thread, (void*)(""), DEF_STACKSIZE, DEF_THREAD_PRIORITY_NORMAL, 1, false);

	while(sapp7_already_init)
	{
		if(draw)
			Sapp7_draw_init_exit_message();
		else
			Util_sleep(20000);
	}

	Util_log_save(DEF_SAPP7_EXIT_STR, "threadJoin()...", threadJoin(sapp7_exit_thread, DEF_THREAD_WAIT_TIME));
	threadFree(sapp7_exit_thread);

	Util_remove_watch(WATCH_HANDLE_SUB_APP7, &sapp7_status.sequencial_id);
	Util_str_free(&sapp7_status);
	var_need_reflesh = true;

	Util_log_save(DEF_SAPP7_EXIT_STR, "Exited.");
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
			Draw_screen_ready(SCREEN_TOP_LEFT, back_color);

			Draw(sapp7_msg[0], 0, 20, 0.5, 0.5, color);
			if(Util_log_query_log_show_flag())
				Util_log_draw();

			Draw_top_ui();

			if(var_monitor_cpu_usage)
				Draw_cpu_usage_info();

			if(Draw_is_3d_mode())
			{
				Draw_screen_ready(SCREEN_TOP_RIGHT, back_color);

				if(Util_log_query_log_show_flag())
					Util_log_draw();

				Draw_top_ui();

				if(var_monitor_cpu_usage)
					Draw_cpu_usage_info();
			}
		}

		if(var_turn_on_bottom_lcd)
		{
			Draw_screen_ready(SCREEN_BOTTOM, back_color);

			Draw(DEF_SAPP7_VER, 0, 0, 0.4, 0.4, DEF_DRAW_GREEN);

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
		Draw_screen_ready(SCREEN_TOP_LEFT, back_color);
		Draw_top_ui();
		if(var_monitor_cpu_usage)
			Draw_cpu_usage_info();

		Draw(sapp7_status.buffer, 0, 20, 0.65, 0.65, color);

		Draw_apply_draw();
	}
	else
		gspWaitForVBlank();
}

static void Sapp7_init_thread(void* arg)
{
	Util_log_save(DEF_SAPP7_INIT_STR, "Thread started.");
	Result_with_string result;

	Util_str_set(&sapp7_status, "Initializing variables...");
	//Empty.

	Util_str_add(&sapp7_status, "\nInitializing queue...");
	//Empty.

	Util_str_add(&sapp7_status, "\nStarting threads...");
	sapp7_thread_run = true;
	sapp7_worker_thread = threadCreate(Sapp7_worker_thread, (void*)(""), DEF_STACKSIZE, DEF_THREAD_PRIORITY_NORMAL, 1, false);

	sapp7_already_init = true;

	Util_log_save(DEF_SAPP7_INIT_STR, "Thread exit.");
	threadExit(0);
}

static void Sapp7_exit_thread(void* arg)
{
	Util_log_save(DEF_SAPP7_EXIT_STR, "Thread started.");

	sapp7_thread_suspend = false;
	sapp7_thread_run = false;

	Util_str_set(&sapp7_status, "Exiting threads...");
	Util_log_save(DEF_SAPP7_EXIT_STR, "threadJoin()...", threadJoin(sapp7_worker_thread, DEF_THREAD_WAIT_TIME));

	Util_str_add(&sapp7_status, "\nCleaning up...");
	threadFree(sapp7_worker_thread);

	sapp7_already_init = false;

	Util_log_save(DEF_SAPP7_EXIT_STR, "Thread exit.");
	threadExit(0);
}

static void Sapp7_worker_thread(void* arg)
{
	Util_log_save(DEF_SAPP7_WORKER_THREAD_STR, "Thread started.");

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

	Util_log_save(DEF_SAPP7_WORKER_THREAD_STR, "Thread exit.");
	threadExit(0);
}
