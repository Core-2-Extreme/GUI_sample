#include <stdbool.h>
#include <stdint.h>

#include "system/types.hpp"

#include "system/menu.hpp"
#include "system/variables.hpp"

#include "system/util/util.hpp"

extern "C"
{
	#include "system/draw/draw.h"
	#include "system/util/err.h"
	#include "system/util/expl.h"
	#include "system/util/hid.h"
	#include "system/util/hw_config.h"
	#include "system/util/log.h"
	#include "system/util/queue.h"
	#include "system/util/str.h"
	#include "system/util/thread_types.h"
}

//Include myself.
#include "sapp2.hpp"


typedef enum
{
	NONE,

	SLEEP_WAKE_UP_WITH_SHELL_REQUEST,
	SLEEP_WAKE_UP_WITH_BUTTON_REQUEST,
	SLEEP_WAKE_UP_WITH_SHELL_OR_BUTTON_REQUEST,
	CHANGE_WIFI_STATE_REQUEST,
	INCREASE_TOP_SCREEN_BRIGHTNESS_REQUEST,
	DECREASE_TOP_SCREEN_BRIGHTNESS_REQUEST,
	INCREASE_BOTTOM_SCREEN_BRIGHTNESS_REQUEST,
	DECREASE_BOTTOM_SCREEN_BRIGHTNESS_REQUEST,
	TURN_TOP_SCREEN_OFF_REQUEST,
	TURN_BOTTOM_SCREEN_OFF_REQUEST,

	MAX = 0xFFFFFFFF,
} Sapp2_command;

DEF_LOG_ENUM_DEBUG
(
	Sapp2_command,
	NONE,
	SLEEP_WAKE_UP_WITH_SHELL_REQUEST,
	SLEEP_WAKE_UP_WITH_BUTTON_REQUEST,
	SLEEP_WAKE_UP_WITH_SHELL_OR_BUTTON_REQUEST,
	CHANGE_WIFI_STATE_REQUEST,
	INCREASE_TOP_SCREEN_BRIGHTNESS_REQUEST,
	DECREASE_TOP_SCREEN_BRIGHTNESS_REQUEST,
	INCREASE_BOTTOM_SCREEN_BRIGHTNESS_REQUEST,
	DECREASE_BOTTOM_SCREEN_BRIGHTNESS_REQUEST,
	TURN_TOP_SCREEN_OFF_REQUEST,
	TURN_BOTTOM_SCREEN_OFF_REQUEST,
	MAX
);


bool sapp2_main_run = false;
bool sapp2_thread_run = false;
bool sapp2_already_init = false;
bool sapp2_thread_suspend = true;
Thread sapp2_init_thread = NULL, sapp2_exit_thread = NULL, sapp2_worker_thread = NULL;
Queue_data sapp2_command_queue = { 0, };
Str_data sapp2_status = { 0, };
Str_data sapp2_msg[DEF_SAPP2_NUM_OF_MSG] = { 0, };


static void Sapp2_draw_init_exit_message(void);
static void Sapp2_init_thread(void* arg);
static void Sapp2_exit_thread(void* arg);
static void Sapp2_worker_thread(void* arg);


bool Sapp2_query_init_flag(void)
{
	return sapp2_already_init;
}

bool Sapp2_query_running_flag(void)
{
	return sapp2_main_run;
}

void Sapp2_hid(Hid_info key)
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
		Sapp2_command command = NONE;

		if(Util_hid_is_pressed(key, *Draw_get_bot_ui_button()))
			Draw_get_bot_ui_button()->selected = true;
		else if (key.p_start || (Util_hid_is_released(key, *Draw_get_bot_ui_button()) && Draw_get_bot_ui_button()->selected))
			Sapp2_suspend();

		if(key.p_a)
			command = SLEEP_WAKE_UP_WITH_SHELL_REQUEST;
		else if(key.p_b)
			command = SLEEP_WAKE_UP_WITH_BUTTON_REQUEST;
		else if(key.p_y)
			command = SLEEP_WAKE_UP_WITH_SHELL_OR_BUTTON_REQUEST;
		else if(key.p_x)
			command = CHANGE_WIFI_STATE_REQUEST;
		else if(key.p_c_up || (key.h_c_up && key.held_time >= 18 && key.held_time % 3 == 0))
			command = INCREASE_TOP_SCREEN_BRIGHTNESS_REQUEST;
		else if(key.p_c_down || (key.h_c_down && key.held_time >= 18 && key.held_time % 3 == 0))
			command = DECREASE_TOP_SCREEN_BRIGHTNESS_REQUEST;
		else if(key.p_d_up || (key.h_d_up && key.held_time >= 18 && key.held_time % 3 == 0))
			command = INCREASE_BOTTOM_SCREEN_BRIGHTNESS_REQUEST;
		else if(key.p_d_down || (key.h_d_down && key.held_time >= 18 && key.held_time % 3 == 0))
			command = DECREASE_BOTTOM_SCREEN_BRIGHTNESS_REQUEST;
		else if(key.p_l)
			command = TURN_TOP_SCREEN_OFF_REQUEST;
		else if(key.p_r)
			command = TURN_BOTTOM_SCREEN_OFF_REQUEST;

		if(command != NONE)
		{
			uint32_t result = DEF_ERR_OTHER;

			DEF_LOG_RESULT_SMART(result, Util_queue_add(&sapp2_command_queue, command, NULL, 10000, QUEUE_OPTION_DO_NOT_ADD_IF_EXIST),
			(result == DEF_SUCCESS), result);
		}
	}

	if(!key.p_touch && !key.h_touch)
		Draw_get_bot_ui_button()->selected = false;

	if(Util_log_query_log_show_flag())
		Util_log_main(key);
}

void Sapp2_resume(void)
{
	sapp2_thread_suspend = false;
	sapp2_main_run = true;
	var_need_reflesh = true;
	Menu_suspend();
}

void Sapp2_suspend(void)
{
	sapp2_thread_suspend = true;
	sapp2_main_run = false;
	var_need_reflesh = true;
	Menu_resume();
}

uint32_t Sapp2_load_msg(const char* lang)
{
	char file_name[32] = { 0, };

	snprintf(file_name, sizeof(file_name), "sapp2_%s.txt", (lang ? lang : ""));
	return Util_load_msg(file_name, sapp2_msg, DEF_SAPP2_NUM_OF_MSG);
}

void Sapp2_init(bool draw)
{
	DEF_LOG_STRING("Initializing...");
	uint32_t result = DEF_ERR_OTHER;

	DEF_LOG_RESULT_SMART(result, Util_str_init(&sapp2_status), (result == DEF_SUCCESS), result);

	Util_add_watch(WATCH_HANDLE_SUB_APP2, &sapp2_status.sequencial_id, sizeof(sapp2_status.sequencial_id));

	if((var_model == CFG_MODEL_N2DSXL || var_model == CFG_MODEL_N3DSXL || var_model == CFG_MODEL_N3DS) && var_core_2_available)
		sapp2_init_thread = threadCreate(Sapp2_init_thread, (void*)(""), DEF_THREAD_STACKSIZE, DEF_THREAD_PRIORITY_NORMAL, 2, false);
	else
	{
		APT_SetAppCpuTimeLimit(80);
		sapp2_init_thread = threadCreate(Sapp2_init_thread, (void*)(""), DEF_THREAD_STACKSIZE, DEF_THREAD_PRIORITY_NORMAL, 1, false);
	}

	while(!sapp2_already_init)
	{
		if(draw)
			Sapp2_draw_init_exit_message();
		else
			Util_sleep(20000);
	}

	if(!(var_model == CFG_MODEL_N2DSXL || var_model == CFG_MODEL_N3DSXL || var_model == CFG_MODEL_N3DS) || !var_core_2_available)
		APT_SetAppCpuTimeLimit(10);

	DEF_LOG_RESULT_SMART(result, threadJoin(sapp2_init_thread, DEF_THREAD_WAIT_TIME), (result == DEF_SUCCESS), result);
	threadFree(sapp2_init_thread);

	Util_str_clear(&sapp2_status);
	Sapp2_resume();

	DEF_LOG_STRING("Initialized.");
}

void Sapp2_exit(bool draw)
{
	DEF_LOG_STRING("Exiting...");
	uint32_t result = DEF_ERR_OTHER;

	sapp2_exit_thread = threadCreate(Sapp2_exit_thread, (void*)(""), DEF_THREAD_STACKSIZE, DEF_THREAD_PRIORITY_NORMAL, 1, false);

	while(sapp2_already_init)
	{
		if(draw)
			Sapp2_draw_init_exit_message();
		else
			Util_sleep(20000);
	}

	DEF_LOG_RESULT_SMART(result, threadJoin(sapp2_exit_thread, DEF_THREAD_WAIT_TIME), (result == DEF_SUCCESS), result);
	threadFree(sapp2_exit_thread);

	Util_remove_watch(WATCH_HANDLE_SUB_APP2, &sapp2_status.sequencial_id);
	Util_str_free(&sapp2_status);
	var_need_reflesh = true;

	DEF_LOG_STRING("Exited.");
}

void Sapp2_main(void)
{
	uint32_t color = DEF_DRAW_BLACK;
	uint32_t back_color = DEF_DRAW_WHITE;
	Watch_handle_bit watch_handle_bit = (DEF_WATCH_HANDLE_BIT_GLOBAL | DEF_WATCH_HANDLE_BIT_SUB_APP2);

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
			char msg[64];
			Draw_screen_ready(DRAW_SCREEN_TOP_LEFT, back_color);

			Draw(&sapp2_msg[0], 0, 20, 0.45, 0.45, color);

			Draw_c("Press A to sleep and wake up if you reopen the shell.", 0, 40, 0.425, 0.425, color);
			Draw_c("Press B to sleep and wake up if you press the home button.", 0, 50, 0.425, 0.425, color);
			Draw_c("Press Y to sleep and wake up if you reopen the shell", 0, 60, 0.425, 0.425, color);
			Draw_c("or press the home button.", 0, 70, 0.425, 0.425, color);

			snprintf(msg, sizeof(msg), "Press X to toggle wifi state. Current wifi state : %s", (var_wifi_enabled ? "enable" : "disable"));
			Draw_c(msg, 0, 90, 0.425, 0.425, color);

			Draw_c("Press or hold circle pad up to increase top screen brightness.", 0, 110, 0.425, 0.425, color);
			Draw_c("Press or hold circle pad down to decrease top screen brightness.", 0, 120, 0.425, 0.425, color);
			snprintf(msg, sizeof(msg), "Current top lcd brightness : %d", var_top_lcd_brightness);
			Draw_c(msg, 0, 130, 0.425, 0.425, color);

			Draw_c("Press or hold direction pad up to increase bottom screen brightness.", 0, 150, 0.425, 0.425, color);
			Draw_c("Press or hold direction pad down to decrease bottom screen brightness.", 0, 160, 0.425, 0.425, color);
			snprintf(msg, sizeof(msg), "Current bottom lcd brightness : %d", var_bottom_lcd_brightness);
			Draw_c(msg, 0, 170, 0.425, 0.425, color);

			Draw_c("Press L to turn top screen off. (turn it back on in 5s)", 0, 190, 0.425, 0.425, color);
			Draw_c("Press R to turn bottom screen off. (turn it back on in 5s)", 0, 200, 0.425, 0.425, color);

			Draw_c("Changing screen brightness and state may not work on O2DS.", 0, 220, 0.45, 0.45, DEF_DRAW_RED);

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

			Draw_c(DEF_SAPP2_VER, 0, 0, 0.4, 0.4, DEF_DRAW_GREEN);

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

static void Sapp2_draw_init_exit_message(void)
{
	uint32_t color = DEF_DRAW_BLACK;
	uint32_t back_color = DEF_DRAW_WHITE;
	Watch_handle_bit watch_handle_bit = (DEF_WATCH_HANDLE_BIT_GLOBAL | DEF_WATCH_HANDLE_BIT_SUB_APP2);

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

		Draw(&sapp2_status, 0, 20, 0.65, 0.65, color);

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

			Draw(&sapp2_status, 0, 20, 0.65, 0.65, color);
		}

		Draw_apply_draw();
	}
	else
		gspWaitForVBlank();
}

static void Sapp2_init_thread(void* arg)
{
	DEF_LOG_STRING("Thread started.");
	uint32_t result = DEF_ERR_OTHER;

	Util_str_set(&sapp2_status, "Initializing variables...");
	//Empty.

	Util_str_add(&sapp2_status, "\nInitializing queue...");
	//Create the queue for commands.
	DEF_LOG_RESULT_SMART(result, Util_queue_create(&sapp2_command_queue, 10), (result == DEF_SUCCESS), result);

	Util_str_add(&sapp2_status, "\nStarting threads...");
	sapp2_thread_run = true;
	sapp2_worker_thread = threadCreate(Sapp2_worker_thread, (void*)(""), DEF_THREAD_STACKSIZE, DEF_THREAD_PRIORITY_NORMAL, 1, false);

	sapp2_already_init = true;

	DEF_LOG_STRING("Thread exit.");
	threadExit(0);
}

static void Sapp2_exit_thread(void* arg)
{
	DEF_LOG_STRING("Thread started.");
	uint32_t result = DEF_ERR_OTHER;

	sapp2_thread_suspend = false;
	sapp2_thread_run = false;

	Util_str_set(&sapp2_status, "Exiting threads...");
	DEF_LOG_RESULT_SMART(result, threadJoin(sapp2_worker_thread, DEF_THREAD_WAIT_TIME), (result == DEF_SUCCESS), result);

	Util_str_add(&sapp2_status, "\nCleaning up...");
	threadFree(sapp2_worker_thread);

	//Delete the queue.
	Util_queue_delete(&sapp2_command_queue);

	sapp2_already_init = false;

	DEF_LOG_STRING("Thread exit.");
	threadExit(0);
}

static void Sapp2_worker_thread(void* arg)
{
	DEF_LOG_STRING("Thread started.");
	uint32_t result = DEF_ERR_OTHER;

	while (sapp2_thread_run)
	{
		uint32_t event_id = 0;

		while (sapp2_thread_suspend)
			Util_sleep(DEF_THREAD_INACTIVE_SLEEP_TIME);

		result = Util_queue_get(&sapp2_command_queue, &event_id, NULL, DEF_THREAD_ACTIVE_SLEEP_TIME);
		if(result != DEF_SUCCESS)
		{
			//No commands have arrived.
			continue;
		}

		//Got a command.
		DEF_LOG_FORMAT("Received event : %s (%" PRIu32 ")", Sapp2_command_get_name((Sapp2_command)event_id), event_id);

		switch ((Sapp2_command)event_id)
		{
			case SLEEP_WAKE_UP_WITH_SHELL_REQUEST:
			{
				aptSetSleepAllowed(true);

				DEF_LOG_RESULT_SMART(result, Util_hw_config_sleep_system(HW_CONFIG_WAKEUP_BIT_OPEN_SHELL), (result == DEF_SUCCESS), result);

				break;
			}
			case SLEEP_WAKE_UP_WITH_BUTTON_REQUEST:
			{
				aptSetSleepAllowed(true);

				DEF_LOG_RESULT_SMART(result, Util_hw_config_sleep_system(HW_CONFIG_WAKEUP_BIT_PRESS_HOME_BUTTON), (result == DEF_SUCCESS), result);

				break;
			}
			case SLEEP_WAKE_UP_WITH_SHELL_OR_BUTTON_REQUEST:
			{
				Hw_config_wakeup_bit wakeup_bits = (HW_CONFIG_WAKEUP_BIT_OPEN_SHELL | HW_CONFIG_WAKEUP_BIT_PRESS_HOME_BUTTON);
				aptSetSleepAllowed(true);

				DEF_LOG_RESULT_SMART(result, Util_hw_config_sleep_system(wakeup_bits), (result == DEF_SUCCESS), result);

				break;
			}
			case CHANGE_WIFI_STATE_REQUEST:
			{
				DEF_LOG_RESULT_SMART(result, Util_hw_config_set_wifi_state(!var_wifi_enabled), (result == DEF_SUCCESS), result);
				if(result == DEF_SUCCESS)
					var_wifi_enabled = !var_wifi_enabled;

				break;
			}
			case INCREASE_TOP_SCREEN_BRIGHTNESS_REQUEST:
			{
				//This isn't necessary because screen brightness will be changed automatically
				//in Menu_worker_thread() if you change the value of var_top_lcd_brightness.
				/*
				uint8_t brightness = var_top_lcd_brightness;
				if(brightness + 1 <= 180)
					brightness++;

				DEF_LOG_RESULT_SMART(result, Util_hw_config_set_screen_brightness(true, false, brightness), (result == DEF_SUCCESS), result);
				if(result == DEF_SUCCESS)
					var_top_lcd_brightness = brightness;
				*/

				if(var_top_lcd_brightness + 1 <= 180)
				{
					var_top_lcd_brightness++;
					//Update brightness value on the screen.
					var_need_reflesh = true;
				}

				break;
			}
			case DECREASE_TOP_SCREEN_BRIGHTNESS_REQUEST:
			{
				//This isn't necessary because screen brightness will be changed automatically
				//in Menu_worker_thread() if you change the value of var_top_lcd_brightness.
				/*
				uint8_t brightness = var_top_lcd_brightness;
				if(brightness - 1 >= 0)
					brightness--;

				DEF_LOG_RESULT_SMART(result, Util_hw_config_set_screen_brightness(true, false, brightness), (result == DEF_SUCCESS), result);
				if(result == DEF_SUCCESS)
					var_top_lcd_brightness = brightness;
				*/

				if(var_top_lcd_brightness - 1 >= 0)
				{
					var_top_lcd_brightness--;
					//Update brightness value on the screen.
					var_need_reflesh = true;
				}

				break;
			}
			case INCREASE_BOTTOM_SCREEN_BRIGHTNESS_REQUEST:
			{
				//This isn't necessary because screen brightness will be changed automatically
				//in Menu_worker_thread() if you change the value of var_bottom_lcd_brightness.
				/*
				uint8_t brightness = var_bottom_lcd_brightness;
				if(brightness + 1 <= 180)
					brightness++;

				DEF_LOG_RESULT_SMART(result, Util_hw_config_set_screen_brightness(false, true, brightness), (result == DEF_SUCCESS), result);
				if(result == DEF_SUCCESS)
					var_bottom_lcd_brightness = brightness;
				*/

				if(var_bottom_lcd_brightness + 1 <= 180)
				{
					var_bottom_lcd_brightness++;
					//Update brightness value on the screen.
					var_need_reflesh = true;
				}

				break;
			}
			case DECREASE_BOTTOM_SCREEN_BRIGHTNESS_REQUEST:
			{
				//This isn't necessary because screen brightness will be changed automatically
				//in Menu_worker_thread() if you change the value of var_bottom_lcd_brightness.
				/*
				uint8_t brightness = var_bottom_lcd_brightness;
				if(brightness - 1 >= 0)
					brightness--;

				DEF_LOG_RESULT_SMART(result, Util_hw_config_set_screen_brightness(false, true, brightness), (result == DEF_SUCCESS), result);
				if(result == DEF_SUCCESS)
					var_bottom_lcd_brightness = brightness;
				*/

				if(var_bottom_lcd_brightness - 1 >= 0)
				{
					var_bottom_lcd_brightness--;
					//Update brightness value on the screen.
					var_need_reflesh = true;
				}

				break;
			}
			case TURN_TOP_SCREEN_OFF_REQUEST:
			{
				//This isn't necessary because screen state will be changed automatically
				//in Menu_worker_thread() if you change the value of var_turn_on_top_lcd.
				/*
				DEF_LOG_RESULT_SMART(result, Util_hw_config_set_screen_state(true, false, false), (result == DEF_SUCCESS), result);
				if(result == DEF_SUCCESS)
					var_turn_on_top_lcd = false;

				Util_sleep(5000000);

				DEF_LOG_RESULT_SMART(result, Util_hw_config_set_screen_state(true, false, true), (result == DEF_SUCCESS), result);
				if(result == DEF_SUCCESS)
					var_turn_on_top_lcd = true;
				*/

				var_turn_on_top_lcd = false;
				Util_sleep(5000000);
				var_turn_on_top_lcd = true;

				break;
			}
			case TURN_BOTTOM_SCREEN_OFF_REQUEST:
			{
				//This isn't necessary because screen state will be changed automatically
				//in Menu_worker_thread() if you change the value of var_turn_on_bottom_lcd.
				/*
				DEF_LOG_RESULT_SMART(result, Util_hw_config_set_screen_state(false, true, false), (result == DEF_SUCCESS), result);
				if(result == DEF_SUCCESS)
					var_turn_on_bottom_lcd = false;

				Util_sleep(5000000);

				DEF_LOG_RESULT_SMART(result, Util_hw_config_set_screen_state(false, true, true), (result == DEF_SUCCESS), result);
				if(result == DEF_SUCCESS)
					var_turn_on_bottom_lcd = true;
				*/

				var_turn_on_bottom_lcd = false;
				Util_sleep(5000000);
				var_turn_on_bottom_lcd = true;

				break;
			}
			default:
				break;
		}
	}

	DEF_LOG_STRING("Thread exit.");
	threadExit(0);
}
