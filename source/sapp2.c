//Includes.
#include "sapp2.h"

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
#include "system/util/hw_config.h"
#include "system/util/log.h"
#include "system/util/queue.h"
#include "system/util/str.h"
#include "system/util/thread_types.h"
#include "system/util/util.h"
#include "system/util/watch.h"

//Defines.
//System UI.
#define DEF_SAPP2_HID_SYSTEM_UI_SEL(k)			(bool)((DEF_HID_PHY_PR(k.touch) && DEF_HID_INIT_IN((*Draw_get_bot_ui_button()), k)) || DEF_HID_PHY_PR(k.start))
#define DEF_SAPP2_HID_SYSTEM_UI_CFM(k)			(bool)(((DEF_HID_PR_EM(k.touch, 1) || DEF_HID_HD(k.touch)) && DEF_HID_INIT_LAST_IN((*Draw_get_bot_ui_button()), k)) || (DEF_HID_PR_EM(k.start, 1) || DEF_HID_HD(k.start)))
#define DEF_SAPP2_HID_SYSTEM_UI_DESEL(k)		(bool)(DEF_HID_PHY_NP(k.touch) && DEF_HID_PHY_NP(k.start))
//Enter sleep mode.
#define DEF_SAPP2_HID_SLEEP_WAKE_SHELL_CFM(k)	(bool)(DEF_HID_PR_EM(k.a, 1) || DEF_HID_HD(k.a))
#define DEF_SAPP2_HID_SLEEP_WAKE_BUTTON_CFM(k)	(bool)(DEF_HID_PR_EM(k.b, 1) || DEF_HID_HD(k.b))
#define DEF_SAPP2_HID_SLEEP_WAKE_ANY_CFM(k)		(bool)(DEF_HID_PR_EM(k.y, 1) || DEF_HID_HD(k.y))
//Toggle Wi-Fi state.
#define DEF_SAPP2_HID_TOGGLE_WIFI_CFM(k)		(bool)(DEF_HID_PR_EM(k.x, 1) || DEF_HID_HD(k.x))
//Change brightness (top LCD).
#define DEF_SAPP2_HID_INCREASE_BRIGHTNESS_TOP_PRE_CFM(k)			(bool)(DEF_HID_PHY_PR(k.c_up) || DEF_HID_HE(k.c_up))
#define DEF_SAPP2_HID_INCREASE_BRIGHTNESS_TOP_UPDATE_RANGE(k)		DEF_HID_HE_NEW_INTERVAL(k.c_up, 150, is_new_range)
#define DEF_SAPP2_HID_INCREASE_BRIGHTNESS_TOP_CFM(k)				(bool)(DEF_HID_PHY_PR(k.c_up) || DEF_HID_HE_MT(k.c_up, 1000) || is_new_range)
#define DEF_SAPP2_HID_DECREASE_BRIGHTNESS_TOP_PRE_CFM(k)			(bool)(DEF_HID_PHY_PR(k.c_down) || DEF_HID_HE(k.c_down))
#define DEF_SAPP2_HID_DECREASE_BRIGHTNESS_TOP_UPDATE_RANGE(k)		DEF_HID_HE_NEW_INTERVAL(k.c_down, 150, is_new_range)
#define DEF_SAPP2_HID_DECREASE_BRIGHTNESS_TOP_CFM(k)				(bool)(DEF_HID_PHY_PR(k.c_down) || DEF_HID_HE_MT(k.c_down, 1000) || is_new_range)
//Change brightness (bottom LCD).
#define DEF_SAPP2_HID_INCREASE_BRIGHTNESS_BOTTOM_PRE_CFM(k)			(bool)(DEF_HID_PHY_PR(k.d_up) || DEF_HID_HE(k.d_up))
#define DEF_SAPP2_HID_INCREASE_BRIGHTNESS_BOTTOM_UPDATE_RANGE(k)	DEF_HID_HE_NEW_INTERVAL(k.d_up, 150, is_new_range)
#define DEF_SAPP2_HID_INCREASE_BRIGHTNESS_BOTTOM_CFM(k)				(bool)(DEF_HID_PHY_PR(k.d_up) || DEF_HID_HE_MT(k.d_up, 1000) || is_new_range)
#define DEF_SAPP2_HID_DECREASE_BRIGHTNESS_BOTTOM_PRE_CFM(k)			(bool)(DEF_HID_PHY_PR(k.d_down) || DEF_HID_HE(k.d_down))
#define DEF_SAPP2_HID_DECREASE_BRIGHTNESS_BOTTOM_UPDATE_RANGE(k)	DEF_HID_HE_NEW_INTERVAL(k.d_down, 150, is_new_range)
#define DEF_SAPP2_HID_DECREASE_BRIGHTNESS_BOTTOM_CFM(k)				(bool)(DEF_HID_PHY_PR(k.d_down) || DEF_HID_HE_MT(k.d_down, 1000) || is_new_range)
//Turn off LCDs.
#define DEF_SAPP2_HID_TURN_OFF_TOP_CFM(k)		(bool)(DEF_HID_PR_EM(k.l, 1) || DEF_HID_HD(k.l))
#define DEF_SAPP2_HID_TURN_OFF_BOTTOM_CFM(k)	(bool)(DEF_HID_PR_EM(k.r, 1) || DEF_HID_HD(k.r))

//Typedefs.
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

	MAX = INT32_MAX,
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
)

//Prototypes.
static void Sapp2_draw_init_exit_message(void);
static void Sapp2_init_thread(void* arg);
static void Sapp2_exit_thread(void* arg);
static void Sapp2_worker_thread(void* arg);

//Variables.
static bool sapp2_main_run = false;
static bool sapp2_thread_run = false;
static bool sapp2_already_init = false;
static bool sapp2_thread_suspend = true;
static Thread sapp2_init_thread = NULL, sapp2_exit_thread = NULL, sapp2_worker_thread = NULL;
static Queue_data sapp2_command_queue = { 0, };
static Str_data sapp2_status = { 0, };
static Str_data sapp2_msg[DEF_SAPP2_NUM_OF_MSG] = { 0, };

//Code.
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
		bool is_new_range = false;//Used by UPDATE_RANGE and CONFIRMED macro.
		Sapp2_command command = NONE;

		//Notify user that button is being pressed.
		if(DEF_SAPP2_HID_SYSTEM_UI_SEL(key))
			Draw_get_bot_ui_button()->selected = true;

		//Execute functions if conditions are satisfied.
		if(DEF_SAPP2_HID_SYSTEM_UI_CFM(key))
			Sapp2_suspend();
		else if(DEF_SAPP2_HID_SLEEP_WAKE_SHELL_CFM(key))
			command = SLEEP_WAKE_UP_WITH_SHELL_REQUEST;
		else if(DEF_SAPP2_HID_SLEEP_WAKE_BUTTON_CFM(key))
			command = SLEEP_WAKE_UP_WITH_BUTTON_REQUEST;
		else if(DEF_SAPP2_HID_SLEEP_WAKE_ANY_CFM(key))
			command = SLEEP_WAKE_UP_WITH_SHELL_OR_BUTTON_REQUEST;
		else if(DEF_SAPP2_HID_TOGGLE_WIFI_CFM(key))
			command = CHANGE_WIFI_STATE_REQUEST;
		else if(DEF_SAPP2_HID_INCREASE_BRIGHTNESS_TOP_PRE_CFM(key))
		{
			DEF_SAPP2_HID_INCREASE_BRIGHTNESS_TOP_UPDATE_RANGE(key);

			if(DEF_SAPP2_HID_INCREASE_BRIGHTNESS_TOP_CFM(key))
				command = INCREASE_TOP_SCREEN_BRIGHTNESS_REQUEST;
		}
		else if(DEF_SAPP2_HID_DECREASE_BRIGHTNESS_TOP_PRE_CFM(key))
		{
			DEF_SAPP2_HID_DECREASE_BRIGHTNESS_TOP_UPDATE_RANGE(key);

			if(DEF_SAPP2_HID_DECREASE_BRIGHTNESS_TOP_CFM(key))
				command = DECREASE_TOP_SCREEN_BRIGHTNESS_REQUEST;
		}
		else if(DEF_SAPP2_HID_INCREASE_BRIGHTNESS_BOTTOM_PRE_CFM(key))
		{
			DEF_SAPP2_HID_INCREASE_BRIGHTNESS_BOTTOM_UPDATE_RANGE(key);

			if(DEF_SAPP2_HID_INCREASE_BRIGHTNESS_BOTTOM_CFM(key))
				command = INCREASE_BOTTOM_SCREEN_BRIGHTNESS_REQUEST;
		}
		else if(DEF_SAPP2_HID_DECREASE_BRIGHTNESS_BOTTOM_PRE_CFM(key))
		{
			DEF_SAPP2_HID_DECREASE_BRIGHTNESS_BOTTOM_UPDATE_RANGE(key);

			if(DEF_SAPP2_HID_DECREASE_BRIGHTNESS_BOTTOM_CFM(key))
				command = DECREASE_BOTTOM_SCREEN_BRIGHTNESS_REQUEST;
		}
		else if(DEF_SAPP2_HID_TURN_OFF_TOP_CFM(key))
			command = TURN_TOP_SCREEN_OFF_REQUEST;
		else if(DEF_SAPP2_HID_TURN_OFF_BOTTOM_CFM(key))
			command = TURN_BOTTOM_SCREEN_OFF_REQUEST;

		if(command != NONE)
		{
			uint32_t result = DEF_ERR_OTHER;

			DEF_LOG_RESULT_SMART(result, Util_queue_add(&sapp2_command_queue, command, NULL, 10000, QUEUE_OPTION_DO_NOT_ADD_IF_EXIST),
			(result == DEF_SUCCESS), result);
		}
	}

	//Notify user that button is NOT being pressed anymore.
	if(DEF_SAPP2_HID_SYSTEM_UI_DESEL(key))
		Draw_get_bot_ui_button()->selected = false;

	if(Util_log_query_show_flag())
		Util_log_main(key);
}

void Sapp2_resume(void)
{
	sapp2_thread_suspend = false;
	sapp2_main_run = true;
	//Reset key state on scene change.
	Util_hid_reset_key_state(HID_KEY_BIT_ALL);
	Draw_set_refresh_needed(true);
	Menu_suspend();
}

void Sapp2_suspend(void)
{
	sapp2_thread_suspend = true;
	sapp2_main_run = false;
	Draw_set_refresh_needed(true);
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
	Sem_state state = { 0, };

	Sem_get_state(&state);
	DEF_LOG_RESULT_SMART(result, Util_str_init(&sapp2_status), (result == DEF_SUCCESS), result);

	Util_watch_add(WATCH_HANDLE_SUB_APP2, &sapp2_status.sequencial_id, sizeof(sapp2_status.sequencial_id));

	if(DEF_SEM_MODEL_IS_NEW(state.console_model) && Util_is_core_available(2))
		sapp2_init_thread = threadCreate(Sapp2_init_thread, NULL, DEF_THREAD_STACKSIZE, DEF_THREAD_PRIORITY_NORMAL, 2, false);
	else
	{
		APT_SetAppCpuTimeLimit(80);
		sapp2_init_thread = threadCreate(Sapp2_init_thread, NULL, DEF_THREAD_STACKSIZE, DEF_THREAD_PRIORITY_NORMAL, 1, false);
	}

	while(!sapp2_already_init)
	{
		if(draw)
			Sapp2_draw_init_exit_message();
		else
			Util_sleep(20000);
	}

	if(!DEF_SEM_MODEL_IS_NEW(state.console_model) || !Util_is_core_available(2))
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

	sapp2_exit_thread = threadCreate(Sapp2_exit_thread, NULL, DEF_THREAD_STACKSIZE, DEF_THREAD_PRIORITY_NORMAL, 1, false);

	while(sapp2_already_init)
	{
		if(draw)
			Sapp2_draw_init_exit_message();
		else
			Util_sleep(20000);
	}

	DEF_LOG_RESULT_SMART(result, threadJoin(sapp2_exit_thread, DEF_THREAD_WAIT_TIME), (result == DEF_SUCCESS), result);
	threadFree(sapp2_exit_thread);

	Util_watch_remove(WATCH_HANDLE_SUB_APP2, &sapp2_status.sequencial_id);
	Util_str_free(&sapp2_status);
	Draw_set_refresh_needed(true);

	DEF_LOG_STRING("Exited.");
}

void Sapp2_main(void)
{
	uint32_t color = DEF_DRAW_BLACK;
	uint32_t back_color = DEF_DRAW_WHITE;
	Watch_handle_bit watch_handle_bit = (DEF_WATCH_HANDLE_BIT_GLOBAL | DEF_WATCH_HANDLE_BIT_SUB_APP2);
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
			char msg[64];
			Draw_screen_ready(DRAW_SCREEN_TOP_LEFT, back_color);

			Draw(&sapp2_msg[0], 0, 20, 0.45, 0.45, color);

			Draw_c("Press A to sleep and wake up if you reopen the shell.", 0, 40, 0.425, 0.425, color);
			Draw_c("Press B to sleep and wake up if you press the home button.", 0, 50, 0.425, 0.425, color);
			Draw_c("Press Y to sleep and wake up if you reopen the shell", 0, 60, 0.425, 0.425, color);
			Draw_c("or press the home button.", 0, 70, 0.425, 0.425, color);

			snprintf(msg, sizeof(msg), "Press X to toggle wifi state. Current wifi state : %s", (config.is_wifi_on ? "enable" : "disable"));
			Draw_c(msg, 0, 90, 0.425, 0.425, color);

			Draw_c("Press or hold circle pad up to increase top screen brightness.", 0, 110, 0.425, 0.425, color);
			Draw_c("Press or hold circle pad down to decrease top screen brightness.", 0, 120, 0.425, 0.425, color);
			snprintf(msg, sizeof(msg), "Current top lcd brightness : %d", config.top_lcd_brightness);
			Draw_c(msg, 0, 130, 0.425, 0.425, color);

			Draw_c("Press or hold direction pad up to increase bottom screen brightness.", 0, 150, 0.425, 0.425, color);
			Draw_c("Press or hold direction pad down to decrease bottom screen brightness.", 0, 160, 0.425, 0.425, color);
			snprintf(msg, sizeof(msg), "Current bottom lcd brightness : %d", config.bottom_lcd_brightness);
			Draw_c(msg, 0, 170, 0.425, 0.425, color);

			Draw_c("Press L to turn top screen off. (turn it back on in 5s)", 0, 190, 0.425, 0.425, color);
			Draw_c("Press R to turn bottom screen off. (turn it back on in 5s)", 0, 200, 0.425, 0.425, color);

			Draw_c("Changing screen brightness and state may not work on O2DS.", 0, 220, 0.45, 0.45, DEF_DRAW_RED);

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

			Draw_c(DEF_SAPP2_VER, 0, 0, 0.4, 0.4, DEF_DRAW_GREEN);

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

static void Sapp2_draw_init_exit_message(void)
{
	uint32_t color = DEF_DRAW_BLACK;
	uint32_t back_color = DEF_DRAW_WHITE;
	Watch_handle_bit watch_handle_bit = (DEF_WATCH_HANDLE_BIT_GLOBAL | DEF_WATCH_HANDLE_BIT_SUB_APP2);
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

		Draw(&sapp2_status, 0, 20, 0.65, 0.65, color);

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

			Draw(&sapp2_status, 0, 20, 0.65, 0.65, color);
		}

		Draw_apply_draw();
	}
	else
		gspWaitForVBlank();
}

static void Sapp2_init_thread(void* arg)
{
	(void)arg;
	DEF_LOG_STRING("Thread started.");
	uint32_t result = DEF_ERR_OTHER;

	Util_str_set(&sapp2_status, "Initializing variables...");
	//Empty.

	Util_str_add(&sapp2_status, "\nInitializing queue...");
	//Create the queue for commands.
	DEF_LOG_RESULT_SMART(result, Util_queue_create(&sapp2_command_queue, 10), (result == DEF_SUCCESS), result);

	Util_str_add(&sapp2_status, "\nStarting threads...");
	sapp2_thread_run = true;
	sapp2_worker_thread = threadCreate(Sapp2_worker_thread, NULL, DEF_THREAD_STACKSIZE, DEF_THREAD_PRIORITY_NORMAL, 1, false);

	sapp2_already_init = true;

	DEF_LOG_STRING("Thread exit.");
	threadExit(0);
}

static void Sapp2_exit_thread(void* arg)
{
	(void)arg;
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
	(void)arg;
	DEF_LOG_STRING("Thread started.");

	while (sapp2_thread_run)
	{
		uint32_t event_id = 0;
		uint32_t result = DEF_ERR_OTHER;

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
				Sem_config config = { 0, };

				Sem_get_config(&config);

				//Toggle Wifi state.
				config.is_wifi_on = !config.is_wifi_on;

				//Update config so that settings menu will do the job for us.
				//Settings menu internally calls Util_hw_config_set_wifi_state().
				Sem_set_config(&config);

				break;
			}

			case INCREASE_TOP_SCREEN_BRIGHTNESS_REQUEST:
			{
				Sem_config config = { 0, };

				Sem_get_config(&config);

				if(config.top_lcd_brightness < 180)
				{
					config.top_lcd_brightness++;

					//Update config so that settings menu will do the job for us.
					//Settings menu internally calls Util_hw_config_set_screen_brightness().
					//Currently, if top and bottom LCD have a different brightness,
					//brighter one will be shown in settings menu.
					Sem_set_config(&config);

					//Update brightness value on the screen.
					Draw_set_refresh_needed(true);
				}

				break;
			}

			case DECREASE_TOP_SCREEN_BRIGHTNESS_REQUEST:
			{
				Sem_config config = { 0, };

				Sem_get_config(&config);

				if(config.top_lcd_brightness > 0)
				{
					config.top_lcd_brightness--;

					//Update config so that settings menu will do the job for us.
					//Settings menu internally calls Util_hw_config_set_screen_brightness().
					//Currently, if top and bottom LCD have a different brightness,
					//brighter one will be shown in settings menu.
					Sem_set_config(&config);

					//Update brightness value on the screen.
					Draw_set_refresh_needed(true);
				}

				break;
			}

			case INCREASE_BOTTOM_SCREEN_BRIGHTNESS_REQUEST:
			{
				Sem_config config = { 0, };

				Sem_get_config(&config);

				if(config.bottom_lcd_brightness < 180)
				{
					config.bottom_lcd_brightness++;

					//Update config so that settings menu will do the job for us.
					//Settings menu internally calls Util_hw_config_set_screen_brightness().
					//Currently, if top and bottom LCD have a different brightness,
					//brighter one will be shown in settings menu.
					Sem_set_config(&config);

					//Update brightness value on the screen.
					Draw_set_refresh_needed(true);
				}

				break;
			}

			case DECREASE_BOTTOM_SCREEN_BRIGHTNESS_REQUEST:
			{
				Sem_config config = { 0, };

				Sem_get_config(&config);

				if(config.bottom_lcd_brightness > 0)
				{
					config.bottom_lcd_brightness--;

					//Update config so that settings menu will do the job for us.
					//Settings menu internally calls Util_hw_config_set_screen_brightness().
					//Currently, if top and bottom LCD have a different brightness,
					//brighter one will be shown in settings menu.
					Sem_set_config(&config);

					//Update brightness value on the screen.
					Draw_set_refresh_needed(true);
				}

				break;
			}

			case TURN_TOP_SCREEN_OFF_REQUEST:
			{
				Sem_config config = { 0, };

				Sem_get_config(&config);

				//Turn it off.
				config.is_top_lcd_on = false;

				//Update config so that settings menu will do the job for us.
				//Settings menu internally calls Util_hw_config_set_screen_state().
				Sem_set_config(&config);

				//Wait for 5 seconds and turn it back on again.
				Util_sleep(5000000);

				//Turn it on.
				config.is_top_lcd_on = true;

				//Update config so that settings menu will do the job for us.
				//Settings menu internally calls Util_hw_config_set_screen_state().
				Sem_set_config(&config);

				break;
			}

			case TURN_BOTTOM_SCREEN_OFF_REQUEST:
			{
				Sem_config config = { 0, };

				Sem_get_config(&config);

				//Turn it off.
				config.is_bottom_lcd_on = false;

				//Update config so that settings menu will do the job for us.
				//Settings menu internally calls Util_hw_config_set_screen_state().
				Sem_set_config(&config);

				//Wait for 5 seconds and turn it back on again.
				Util_sleep(5000000);

				//Turn it on.
				config.is_bottom_lcd_on = true;

				//Update config so that settings menu will do the job for us.
				//Settings menu internally calls Util_hw_config_set_screen_state().
				Sem_set_config(&config);

				break;
			}

			default:
				break;
		}
	}

	DEF_LOG_STRING("Thread exit.");
	threadExit(0);
}
