#include "definitions.hpp"
#include "system/types.hpp"

#include "system/menu.hpp"
#include "system/variables.hpp"

#include "system/draw/draw.hpp"

#include "system/util/change_setting.hpp"
#include "system/util/error.hpp"
#include "system/util/hid.hpp"
#include "system/util/log.hpp"
#include "system/util/queue.hpp"
#include "system/util/util.hpp"

extern "C"
{
	#include "system/util/string.h"
}

//Include myself.
#include "sub_app2.hpp"


enum Sapp2_command
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
};


bool sapp2_main_run = false;
bool sapp2_thread_run = false;
bool sapp2_already_init = false;
bool sapp2_thread_suspend = true;
std::string sapp2_msg[DEF_SAPP2_NUM_OF_MSG];
Thread sapp2_init_thread, sapp2_exit_thread, sapp2_worker_thread;
Queue sapp2_command_queue;
Util_string sapp2_status = { 0, };


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
			Result_with_string result = Util_queue_add(&sapp2_command_queue, command, NULL, 10000, QUEUE_OPTION_DO_NOT_ADD_IF_EXIST);
			Util_log_save(DEF_SAPP2_HID_CALLBACK_STR, "Util_queue_add()..." + result.string + result.error_description, result.code);
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

Result_with_string Sapp2_load_msg(std::string lang)
{
	return Util_load_msg("sapp2_" + lang + ".txt", sapp2_msg, DEF_SAPP2_NUM_OF_MSG);
}

void Sapp2_init(bool draw)
{
	Util_log_save(DEF_SAPP2_INIT_STR, "Initializing...");
	Result_with_string result;

	result.code = Util_string_init(&sapp2_status);
	Util_log_save(DEF_SAPP2_INIT_STR, "Util_string_init()..." + result.string + result.error_description, result.code);

	Util_add_watch(WATCH_HANDLE_SUB_APP2, &sapp2_status.sequencial_id, sizeof(sapp2_status.sequencial_id));

	if((var_model == CFG_MODEL_N2DSXL || var_model == CFG_MODEL_N3DSXL || var_model == CFG_MODEL_N3DS) && var_core_2_available)
		sapp2_init_thread = threadCreate(Sapp2_init_thread, (void*)(""), DEF_STACKSIZE, DEF_THREAD_PRIORITY_NORMAL, 2, false);
	else
	{
		APT_SetAppCpuTimeLimit(80);
		sapp2_init_thread = threadCreate(Sapp2_init_thread, (void*)(""), DEF_STACKSIZE, DEF_THREAD_PRIORITY_NORMAL, 1, false);
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

	Util_log_save(DEF_SAPP2_EXIT_STR, "threadJoin()...", threadJoin(sapp2_init_thread, DEF_THREAD_WAIT_TIME));
	threadFree(sapp2_init_thread);

	Util_string_clear(&sapp2_status);
	Sapp2_resume();

	Util_log_save(DEF_SAPP2_INIT_STR, "Initialized.");
}

void Sapp2_exit(bool draw)
{
	Util_log_save(DEF_SAPP2_EXIT_STR, "Exiting...");

	sapp2_exit_thread = threadCreate(Sapp2_exit_thread, (void*)(""), DEF_STACKSIZE, DEF_THREAD_PRIORITY_NORMAL, 1, false);

	while(sapp2_already_init)
	{
		if(draw)
			Sapp2_draw_init_exit_message();
		else
			Util_sleep(20000);
	}

	Util_log_save(DEF_SAPP2_EXIT_STR, "threadJoin()...", threadJoin(sapp2_exit_thread, DEF_THREAD_WAIT_TIME));
	threadFree(sapp2_exit_thread);

	Util_remove_watch(WATCH_HANDLE_SUB_APP2, &sapp2_status.sequencial_id);
	Util_string_free(&sapp2_status);
	var_need_reflesh = true;

	Util_log_save(DEF_SAPP2_EXIT_STR, "Exited.");
}

void Sapp2_main(void)
{
	int color = DEF_DRAW_BLACK;
	int back_color = DEF_DRAW_WHITE;
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
			Draw_screen_ready(SCREEN_TOP_LEFT, back_color);

			Draw(sapp2_msg[0], 0, 20, 0.45, 0.45, color);

			Draw("Press A to sleep and wake up if you reopen the shell.", 0, 40, 0.425, 0.425, color);
			Draw("Press B to sleep and wake up if you press the home button.", 0, 50, 0.425, 0.425, color);
			Draw("Press Y to sleep and wake up if you reopen the shell", 0, 60, 0.425, 0.425, color);
			Draw("or press the home button.", 0, 70, 0.425, 0.425, color);

			snprintf(msg, sizeof(msg), "Press X to toggle wifi state. Current wifi state : %s", (var_wifi_enabled ? "enable" : "disable"));
			Draw(msg, 0, 90, 0.425, 0.425, color);

			Draw("Press or hold circle pad up to increase top screen brightness.", 0, 110, 0.425, 0.425, color);
			Draw("Press or hold circle pad down to decrease top screen brightness.", 0, 120, 0.425, 0.425, color);
			snprintf(msg, sizeof(msg), "Current top lcd brightness : %d", var_top_lcd_brightness);
			Draw(msg, 0, 130, 0.425, 0.425, color);

			Draw("Press or hold direction pad up to increase bottom screen brightness.", 0, 150, 0.425, 0.425, color);
			Draw("Press or hold direction pad down to decrease bottom screen brightness.", 0, 160, 0.425, 0.425, color);
			snprintf(msg, sizeof(msg), "Current bottom lcd brightness : %d", var_bottom_lcd_brightness);
			Draw(msg, 0, 170, 0.425, 0.425, color);

			Draw("Press L to turn top screen off. (turn it back on in 5s)", 0, 190, 0.425, 0.425, color);
			Draw("Press R to turn bottom screen off. (turn it back on in 5s)", 0, 200, 0.425, 0.425, color);

			Draw("Changing screen brightness and state may not work on O2DS.", 0, 220, 0.45, 0.45, DEF_DRAW_RED);

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

			Draw(DEF_SAPP2_VER, 0, 0, 0.4, 0.4, DEF_DRAW_GREEN);

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
	int color = DEF_DRAW_BLACK;
	int back_color = DEF_DRAW_WHITE;
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
		Draw_screen_ready(SCREEN_TOP_LEFT, back_color);
		Draw_top_ui();
		if(var_monitor_cpu_usage)
			Draw_cpu_usage_info();

		Draw(sapp2_status.buffer, 0, 20, 0.65, 0.65, color);

		Draw_apply_draw();
	}
	else
		gspWaitForVBlank();
}

static void Sapp2_init_thread(void* arg)
{
	Util_log_save(DEF_SAPP2_INIT_STR, "Thread started.");
	Result_with_string result;

	Util_string_set(&sapp2_status, "Initializing variables...");
	//Empty.

	Util_string_add(&sapp2_status, "\nInitializing queue...");
	//Create the queue for commands.
	result = Util_queue_create(&sapp2_command_queue, 10);
	Util_log_save(DEF_SAPP2_INIT_STR, "Util_queue_create()..." + result.string + result.error_description, result.code);

	Util_string_add(&sapp2_status, "\nStarting threads...");
	sapp2_thread_run = true;
	sapp2_worker_thread = threadCreate(Sapp2_worker_thread, (void*)(""), DEF_STACKSIZE, DEF_THREAD_PRIORITY_NORMAL, 1, false);

	sapp2_already_init = true;

	Util_log_save(DEF_SAPP2_INIT_STR, "Thread exit.");
	threadExit(0);
}

static void Sapp2_exit_thread(void* arg)
{
	Util_log_save(DEF_SAPP2_EXIT_STR, "Thread started.");

	sapp2_thread_suspend = false;
	sapp2_thread_run = false;

	Util_string_set(&sapp2_status, "Exiting threads...");
	Util_log_save(DEF_SAPP2_EXIT_STR, "threadJoin()...", threadJoin(sapp2_worker_thread, DEF_THREAD_WAIT_TIME));

	Util_string_add(&sapp2_status, "\nCleaning up...");
	threadFree(sapp2_worker_thread);

	//Delete the queue.
	Util_queue_delete(&sapp2_command_queue);

	sapp2_already_init = false;

	Util_log_save(DEF_SAPP2_EXIT_STR, "Thread exit.");
	threadExit(0);
}

static void Sapp2_worker_thread(void* arg)
{
	Util_log_save(DEF_SAPP2_WORKER_THREAD_STR, "Thread started.");
	Result_with_string result;

	while (sapp2_thread_run)
	{
		u32 event_id = 0;

		while (sapp2_thread_suspend)
			Util_sleep(DEF_INACTIVE_THREAD_SLEEP_TIME);

		result = Util_queue_get(&sapp2_command_queue, &event_id, NULL, DEF_ACTIVE_THREAD_SLEEP_TIME);
		if(result.code != 0)
		{
			//No commands have arrived.
			continue;
		}

		//Got a command.
		Util_log_save(DEF_SAPP2_WORKER_THREAD_STR, "Received event : " + std::to_string(event_id));

		switch ((Sapp2_command)event_id)
		{
			case SLEEP_WAKE_UP_WITH_SHELL_REQUEST:
			{
				aptSetSleepAllowed(true);

				result = Util_cset_sleep_system(WAKE_UP_EVENT_OPEN_SHELL);
				Util_log_save(DEF_SAPP2_WORKER_THREAD_STR, "Util_cset_sleep_system()..." + result.string + result.error_description, result.code);

				break;
			}
			case SLEEP_WAKE_UP_WITH_BUTTON_REQUEST:
			{
				aptSetSleepAllowed(true);

				result = Util_cset_sleep_system(WAKE_UP_EVENT_PRESS_HOME_BUTTON);
				Util_log_save(DEF_SAPP2_WORKER_THREAD_STR, "Util_cset_sleep_system()..." + result.string + result.error_description, result.code);

				break;
			}
			case SLEEP_WAKE_UP_WITH_SHELL_OR_BUTTON_REQUEST:
			{
				aptSetSleepAllowed(true);

				result = Util_cset_sleep_system((Wake_up_event)(WAKE_UP_EVENT_OPEN_SHELL | WAKE_UP_EVENT_PRESS_HOME_BUTTON));
				Util_log_save(DEF_SAPP2_WORKER_THREAD_STR, "Util_cset_sleep_system()..." + result.string + result.error_description, result.code);

				break;
			}
			case CHANGE_WIFI_STATE_REQUEST:
			{
				result = Util_cset_set_wifi_state(!var_wifi_enabled);
				Util_log_save(DEF_SAPP2_WORKER_THREAD_STR, "Util_cset_set_wifi_state()..." + result.string + result.error_description, result.code);
				if(result.code == 0)
					var_wifi_enabled = !var_wifi_enabled;

				break;
			}
			case INCREASE_TOP_SCREEN_BRIGHTNESS_REQUEST:
			{
				//This isn't necessary because screen brightness will be changed automatically
				//in Menu_worker_thread() if you change the value of var_top_lcd_brightness.
				/*
				int brightness = var_top_lcd_brightness;
				if(brightness + 1 <= 180)
					brightness++;

				result = Util_cset_set_screen_brightness(true, false, brightness);
				Util_log_save(DEF_SAPP2_WORKER_THREAD_STR, "Util_cset_set_screen_brightness()..." + result.string + result.error_description, result.code);
				if(result.code == 0)
					var_top_lcd_brightness = brightness;
				*/

				if(var_top_lcd_brightness + 1 <= 180)
					var_top_lcd_brightness++;

				break;
			}
			case DECREASE_TOP_SCREEN_BRIGHTNESS_REQUEST:
			{
				//This isn't necessary because screen brightness will be changed automatically
				//in Menu_worker_thread() if you change the value of var_top_lcd_brightness.
				/*
				int brightness = var_top_lcd_brightness;
				if(brightness - 1 >= 0)
					brightness--;

				result = Util_cset_set_screen_brightness(true, false, brightness);
				Util_log_save(DEF_SAPP2_WORKER_THREAD_STR, "Util_cset_set_screen_brightness()..." + result.string + result.error_description, result.code);
				if(result.code == 0)
					var_top_lcd_brightness = brightness;
				*/

				if(var_top_lcd_brightness - 1 >= 0)
					var_top_lcd_brightness--;

				break;
			}
			case INCREASE_BOTTOM_SCREEN_BRIGHTNESS_REQUEST:
			{
				//This isn't necessary because screen brightness will be changed automatically
				//in Menu_worker_thread() if you change the value of var_bottom_lcd_brightness.
				/*
				int brightness = var_bottom_lcd_brightness;
				if(brightness + 1 <= 180)
					brightness++;

				result = Util_cset_set_screen_brightness(true, false, brightness);
				Util_log_save(DEF_SAPP2_WORKER_THREAD_STR, "Util_cset_set_screen_brightness()..." + result.string + result.error_description, result.code);
				if(result.code == 0)
					var_bottom_lcd_brightness = brightness;
				*/

				if(var_bottom_lcd_brightness + 1 <= 180)
					var_bottom_lcd_brightness++;

				break;
			}
			case DECREASE_BOTTOM_SCREEN_BRIGHTNESS_REQUEST:
			{
				//This isn't necessary because screen brightness will be changed automatically
				//in Menu_worker_thread() if you change the value of var_bottom_lcd_brightness.
				/*
				int brightness = var_bottom_lcd_brightness;
				if(brightness - 1 >= 0)
					brightness--;

				result = Util_cset_set_screen_brightness(true, false, brightness);
				Util_log_save(DEF_SAPP2_WORKER_THREAD_STR, "Util_cset_set_screen_brightness()..." + result.string + result.error_description, result.code);
				if(result.code == 0)
					var_bottom_lcd_brightness = brightness;
				*/

				if(var_bottom_lcd_brightness - 1 >= 0)
					var_bottom_lcd_brightness--;

				break;
			}
			case TURN_TOP_SCREEN_OFF_REQUEST:
			{
				//This isn't necessary because screen state will be changed automatically
				//in Menu_worker_thread() if you change the value of var_turn_on_top_lcd.
				/*
				result = Util_cset_set_screen_state(true, false, false);
				Util_log_save(DEF_SAPP2_WORKER_THREAD_STR, "Util_cset_set_screen_state()..." + result.string + result.error_description, result.code);
				if(result.code == 0)
					var_turn_on_top_lcd = false;

				Util_sleep(5000000);

				result = Util_cset_set_screen_state(true, false, true);
				Util_log_save(DEF_SAPP2_WORKER_THREAD_STR, "Util_cset_set_screen_state()..." + result.string + result.error_description, result.code);
				if(result.code == 0)
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
				result = Util_cset_set_screen_state(false, true, false);
				Util_log_save(DEF_SAPP2_WORKER_THREAD_STR, "Util_cset_set_screen_state()..." + result.string + result.error_description, result.code);
				if(result.code == 0)
					var_turn_on_bottom_lcd = false;

				Util_sleep(5000000);

				result = Util_cset_set_screen_state(false, true, true);
				Util_log_save(DEF_SAPP2_WORKER_THREAD_STR, "Util_cset_set_screen_state()..." + result.string + result.error_description, result.code);
				if(result.code == 0)
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

	Util_log_save(DEF_SAPP2_WORKER_THREAD_STR, "Thread exit.");
	threadExit(0);
}
