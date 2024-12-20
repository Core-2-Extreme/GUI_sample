//Includes.
#include "sapp5.h"

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
//System UI.
#define DEF_SAPP5_HID_SYSTEM_UI_SEL(k)			(bool)((DEF_HID_PHY_PR(k.touch) && DEF_HID_INIT_IN((*Draw_get_bot_ui_button()), k)) || DEF_HID_PHY_PR(k.start))
#define DEF_SAPP5_HID_SYSTEM_UI_CFM(k)			(bool)(((DEF_HID_PR_EM(k.touch, 1) || DEF_HID_HD(k.touch)) && DEF_HID_INIT_LAST_IN((*Draw_get_bot_ui_button()), k)) || (DEF_HID_PR_EM(k.start, 1) || DEF_HID_HD(k.start)))
#define DEF_SAPP5_HID_SYSTEM_UI_DESEL(k)		(bool)(DEF_HID_PHY_NP(k.touch) && DEF_HID_PHY_NP(k.start))

//Typedefs.
//N/A.

//Prototypes.
static void Sapp5_draw_init_exit_message(void);
static void Sapp5_init_thread(void* arg);
static void Sapp5_exit_thread(void* arg);
static void Sapp5_worker_thread(void* arg);

//Variables.
static bool sapp5_main_run = false;
static bool sapp5_thread_run = false;
static bool sapp5_already_init = false;
static bool sapp5_thread_suspend = true;
static Thread sapp5_init_thread = NULL, sapp5_exit_thread = NULL, sapp5_worker_thread = NULL;
static Str_data sapp5_status = { 0, };
static Str_data sapp5_msg[DEF_SAPP5_NUM_OF_MSG] = { 0, };

//Code.
bool Sapp5_query_init_flag(void)
{
	return sapp5_already_init;
}

bool Sapp5_query_running_flag(void)
{
	return sapp5_main_run;
}

void Sapp5_hid(Hid_info key)
{
	Sem_config config = { 0, };

	//Do nothing if app is suspended.
	if(aptShouldJumpToHome())
		return;

	Sem_get_config(&config);

	if(Util_err_query_show_flag())
		Util_err_main(key);
	else if(Util_expl_query_show_flag())
		Util_expl_main(key, config.scroll_speed);
	else
	{
		//Notify user that button is being pressed.
		if(DEF_SAPP5_HID_SYSTEM_UI_SEL(key))
			Draw_get_bot_ui_button()->selected = true;

		//Execute functions if conditions are satisfied.
		if(DEF_SAPP5_HID_SYSTEM_UI_CFM(key))
			Sapp5_suspend();
	}

	//Notify user that button is NOT being pressed anymore.
	if(DEF_SAPP5_HID_SYSTEM_UI_DESEL(key))
		Draw_get_bot_ui_button()->selected = false;

	if(Util_log_query_show_flag())
		Util_log_main(key);
}

void Sapp5_resume(void)
{
	sapp5_thread_suspend = false;
	sapp5_main_run = true;
	//Reset key state on scene change.
	Util_hid_reset_key_state(HID_KEY_BIT_ALL);
	Draw_set_refresh_needed(true);
	Menu_suspend();
}

void Sapp5_suspend(void)
{
	sapp5_thread_suspend = true;
	sapp5_main_run = false;
	Draw_set_refresh_needed(true);
	Menu_resume();
}

uint32_t Sapp5_load_msg(const char* lang)
{
	char file_name[32] = { 0, };

	snprintf(file_name, sizeof(file_name), "sapp5_%s.txt", (lang ? lang : ""));
	return Util_load_msg(file_name, sapp5_msg, DEF_SAPP5_NUM_OF_MSG);
}

void Sapp5_init(bool draw)
{
	DEF_LOG_STRING("Initializing...");
	uint32_t result = DEF_ERR_OTHER;
	Sem_state state = { 0, };

	Sem_get_state(&state);
	DEF_LOG_RESULT_SMART(result, Util_str_init(&sapp5_status), (result == DEF_SUCCESS), result);

	Util_watch_add(WATCH_HANDLE_SUB_APP5, &sapp5_status.sequencial_id, sizeof(sapp5_status.sequencial_id));

	if(DEF_SEM_MODEL_IS_NEW(state.console_model) && Util_is_core_available(2))
		sapp5_init_thread = threadCreate(Sapp5_init_thread, NULL, DEF_THREAD_STACKSIZE, DEF_THREAD_PRIORITY_NORMAL, 2, false);
	else
	{
		APT_SetAppCpuTimeLimit(80);
		sapp5_init_thread = threadCreate(Sapp5_init_thread, NULL, DEF_THREAD_STACKSIZE, DEF_THREAD_PRIORITY_NORMAL, 1, false);
	}

	while(!sapp5_already_init)
	{
		if(draw)
			Sapp5_draw_init_exit_message();
		else
			Util_sleep(20000);
	}

	if(!DEF_SEM_MODEL_IS_NEW(state.console_model) || !Util_is_core_available(2))
		APT_SetAppCpuTimeLimit(10);

	DEF_LOG_RESULT_SMART(result, threadJoin(sapp5_init_thread, DEF_THREAD_WAIT_TIME), (result == DEF_SUCCESS), result);
	threadFree(sapp5_init_thread);

	Util_str_clear(&sapp5_status);
	Sapp5_resume();

	DEF_LOG_STRING("Initialized.");
}

void Sapp5_exit(bool draw)
{
	DEF_LOG_STRING("Exiting...");
	uint32_t result = DEF_ERR_OTHER;

	sapp5_exit_thread = threadCreate(Sapp5_exit_thread, NULL, DEF_THREAD_STACKSIZE, DEF_THREAD_PRIORITY_NORMAL, 1, false);

	while(sapp5_already_init)
	{
		if(draw)
			Sapp5_draw_init_exit_message();
		else
			Util_sleep(20000);
	}

	DEF_LOG_RESULT_SMART(result, threadJoin(sapp5_exit_thread, DEF_THREAD_WAIT_TIME), (result == DEF_SUCCESS), result);
	threadFree(sapp5_exit_thread);

	Util_watch_remove(WATCH_HANDLE_SUB_APP5, &sapp5_status.sequencial_id);
	Util_str_free(&sapp5_status);
	Draw_set_refresh_needed(true);

	DEF_LOG_STRING("Exited.");
}

void Sapp5_main(void)
{
	uint32_t color = DEF_DRAW_BLACK;
	uint32_t back_color = DEF_DRAW_WHITE;
	Watch_handle_bit watch_handle_bit = (DEF_WATCH_HANDLE_BIT_GLOBAL | DEF_WATCH_HANDLE_BIT_SUB_APP5);
	Sem_config config = { 0, };
	Sem_state state = { 0, };

	if(Util_err_query_show_flag())
		watch_handle_bit |= DEF_WATCH_HANDLE_BIT_ERR;
	if(Util_expl_query_show_flag())
		watch_handle_bit |= DEF_WATCH_HANDLE_BIT_EXPL;
	if(Util_log_query_show_flag())
		watch_handle_bit |= DEF_WATCH_HANDLE_BIT_LOG;

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

			Draw(&sapp5_msg[0], 0, 20, 0.5, 0.5, color);
			if(Util_log_query_show_flag())
				Util_log_draw();

			Draw_top_ui(config.is_eco, state.is_charging, state.wifi_signal, state.battery_level, state.msg);

			if(config.is_debug)
				Draw_debug_info(config.is_night, state.free_ram, state.free_linear_ram);

			if(Util_cpu_usage_query_show_flag())
				Util_cpu_usage_draw();

			if(Draw_is_3d_mode())
			{
				Draw_screen_ready(DRAW_SCREEN_TOP_RIGHT, back_color);

				if(Util_log_query_show_flag())
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

			Draw_c(DEF_SAPP5_VER, 0, 0, 0.4, 0.4, DEF_DRAW_GREEN);

			if(Util_expl_query_show_flag())
				Util_expl_draw();

			if(Util_err_query_show_flag())
				Util_err_draw();

			Draw_bot_ui();
		}

		Draw_apply_draw();
	}
	else
		gspWaitForVBlank();
}

static void Sapp5_draw_init_exit_message(void)
{
	uint32_t color = DEF_DRAW_BLACK;
	uint32_t back_color = DEF_DRAW_WHITE;
	Watch_handle_bit watch_handle_bit = (DEF_WATCH_HANDLE_BIT_GLOBAL | DEF_WATCH_HANDLE_BIT_SUB_APP5);
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

		if(Util_log_query_show_flag())
			Util_log_draw();

		Draw_top_ui(config.is_eco, state.is_charging, state.wifi_signal, state.battery_level, state.msg);

		if(config.is_debug)
			Draw_debug_info(config.is_night, state.free_ram, state.free_linear_ram);

		if(Util_cpu_usage_query_show_flag())
			Util_cpu_usage_draw();

		Draw(&sapp5_status, 0, 20, 0.65, 0.65, color);

		//Draw the same things on right screen if 3D mode is enabled.
		//So that user can easily see them.
		if(Draw_is_3d_mode())
		{
			Draw_screen_ready(DRAW_SCREEN_TOP_RIGHT, back_color);

			if(Util_log_query_show_flag())
				Util_log_draw();

			Draw_top_ui(config.is_eco, state.is_charging, state.wifi_signal, state.battery_level, state.msg);

			if(config.is_debug)
				Draw_debug_info(config.is_night, state.free_ram, state.free_linear_ram);

			if(Util_cpu_usage_query_show_flag())
				Util_cpu_usage_draw();

			Draw(&sapp5_status, 0, 20, 0.65, 0.65, color);
		}

		Draw_apply_draw();
	}
	else
		gspWaitForVBlank();
}

static void Sapp5_init_thread(void* arg)
{
	(void)arg;
	DEF_LOG_STRING("Thread started.");
	// uint32_t result = DEF_ERR_OTHER;

	Util_str_set(&sapp5_status, "Initializing variables...");
	//Empty.

	Util_str_add(&sapp5_status, "\nInitializing queue...");
	//Empty.

	Util_str_add(&sapp5_status, "\nStarting threads...");
	sapp5_thread_run = true;
	sapp5_worker_thread = threadCreate(Sapp5_worker_thread, NULL, DEF_THREAD_STACKSIZE, DEF_THREAD_PRIORITY_NORMAL, 1, false);

	sapp5_already_init = true;

	DEF_LOG_STRING("Thread exit.");
	threadExit(0);
}

static void Sapp5_exit_thread(void* arg)
{
	(void)arg;
	DEF_LOG_STRING("Thread started.");
	uint32_t result = DEF_ERR_OTHER;

	sapp5_thread_suspend = false;
	sapp5_thread_run = false;

	Util_str_set(&sapp5_status, "Exiting threads...");
	DEF_LOG_RESULT_SMART(result, threadJoin(sapp5_worker_thread, DEF_THREAD_WAIT_TIME), (result == DEF_SUCCESS), result);

	Util_str_add(&sapp5_status, "\nCleaning up...");
	threadFree(sapp5_worker_thread);

	sapp5_already_init = false;

	DEF_LOG_STRING("Thread exit.");
	threadExit(0);
}

static void Sapp5_worker_thread(void* arg)
{
	(void)arg;
	DEF_LOG_STRING("Thread started.");

	while (sapp5_thread_run)
	{
		if(false)
		{

		}
		else
			Util_sleep(DEF_THREAD_ACTIVE_SLEEP_TIME);

		while (sapp5_thread_suspend)
			Util_sleep(DEF_THREAD_INACTIVE_SLEEP_TIME);
	}

	DEF_LOG_STRING("Thread exit.");
	threadExit(0);
}
