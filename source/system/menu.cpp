#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <string>

#include "3ds.h"

#include "system/sem.hpp"

extern "C"
{
#include "system/draw/draw.h"
#include "system/draw/exfont.h"
#include "system/util/cpu_usage.h"
#include "system/util/curl.h"
#include "system/util/err.h"
#include "system/util/expl.h"
#include "system/util/file.h"
#include "system/util/hid.h"
#include "system/util/httpc.h"
#include "system/util/hw_config.h"
#include "system/util/log.h"
#include "system/util/str.h"
#include "system/util/thread_types.h"
#include "system/util/watch.h"
#include "system/util/util_c.h"
}

#include "sapp0.hpp"
#include "sapp1.hpp"
#include "sapp2.hpp"
#include "sapp3.hpp"
#include "sapp4.hpp"
#include "sapp5.hpp"
#include "sapp6.hpp"
#include "sapp7.hpp"

//Include myself.
#include "system/menu.hpp"


#define DEF_MENU_NUM_OF_SUB_APP		(uint8_t)(8)


bool menu_thread_run = false;
bool menu_main_run = true;
bool menu_must_exit = false;
bool menu_check_exit_request = false;
bool menu_update_available = false;
bool menu_init_request[DEF_MENU_NUM_OF_SUB_APP] = { 0, };
bool menu_exit_request[DEF_MENU_NUM_OF_SUB_APP] = { 0, };
uint32_t menu_icon_texture_num[DEF_MENU_NUM_OF_SUB_APP + 1] = { UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX, };
void (*menu_worker_thread_callbacks[DEF_MENU_NUM_OF_CALLBACKS])(void) = { NULL, };
Str_data menu_msg[DEF_MENU_NUM_OF_MSG] = { 0, };
Thread menu_worker_thread = NULL;
LightLock menu_callback_mutex = 1;//Initially unlocked state.
Draw_image_data menu_icon_image[DEF_MENU_NUM_OF_SUB_APP + 2] = { 0, };
Draw_image_data menu_sapp_button[DEF_MENU_NUM_OF_SUB_APP] = { 0, };
Draw_image_data menu_sapp_close_button[DEF_MENU_NUM_OF_SUB_APP] = { 0, };
Draw_image_data menu_sem_button = { 0, };

#if (DEF_CURL_API_ENABLE || DEF_HTTPC_API_ENABLE)

Thread menu_send_app_info_thread = NULL, menu_update_thread = NULL;

#endif

void Menu_hid_callback(void);
void Menu_worker_thread(void* arg);

#if (DEF_CURL_API_ENABLE || DEF_HTTPC_API_ENABLE)

void Menu_send_app_info_thread(void* arg);
void Menu_update_thread(void* arg);

#endif


bool Menu_query_must_exit_flag(void)
{
	return menu_must_exit;
}

void Menu_set_must_exit_flag(bool flag)
{
	menu_must_exit = flag;
}

void Menu_resume(void)
{
	menu_main_run = true;
	Draw_set_refresh_needed(true);
}

void Menu_suspend(void)
{
	menu_main_run = false;
}

uint32_t Menu_load_msg(const char* lang)
{
	char file_name[32] = { 0, };

	snprintf(file_name, sizeof(file_name), "menu_%s.txt", (lang ? lang : ""));
	return Util_load_msg(file_name, menu_msg, DEF_MENU_NUM_OF_MSG);
}

void Menu_init(void)
{
	bool is_800px = false;
	bool is_3d = false;
	uint8_t dummy = 0;
	uint32_t result = DEF_ERR_OTHER;
	C2D_Image cache[2] = { 0, };
	Sem_config config = { 0, };
	Sem_state state = { 0, };

	for(uint16_t i = 0; i < DEF_MENU_NUM_OF_CALLBACKS; i++)
		menu_worker_thread_callbacks[i] = NULL;

	result = Util_log_init();
	DEF_LOG_RESULT(Util_log_init, (result == DEF_SUCCESS), result);
	DEF_LOG_FORMAT("Initializing...%s", DEF_MENU_CURRENT_APP_VER);

	osSetSpeedupEnable(true);
	aptSetSleepAllowed(true);
	svcSetThreadPriority(CUR_THREAD_HANDLE, DEF_THREAD_PRIORITY_HIGH - 1);

	//Init system modules.
	DEF_LOG_RESULT_SMART(result, fsInit(), (result == DEF_SUCCESS), result);
	DEF_LOG_RESULT_SMART(result, acInit(), (result == DEF_SUCCESS), result);
	DEF_LOG_RESULT_SMART(result, aptInit(), (result == DEF_SUCCESS), result);
	DEF_LOG_RESULT_SMART(result, mcuHwcInit(), (result == DEF_SUCCESS), result);
	DEF_LOG_RESULT_SMART(result, ptmuInit(), (result == DEF_SUCCESS), result);
	DEF_LOG_RESULT_SMART(result, romfsInit(), (result == DEF_SUCCESS), result);
	DEF_LOG_RESULT_SMART(result, cfguInit(), (result == DEF_SUCCESS), result);
	DEF_LOG_RESULT_SMART(result, amInit(), (result == DEF_SUCCESS), result);
	DEF_LOG_RESULT_SMART(result, APT_SetAppCpuTimeLimit(30), (result == DEF_SUCCESS), result);

	//Create directories.
	Util_file_save_to_file(".", DEF_MENU_MAIN_DIR, &dummy, 1, true);
	Util_file_save_to_file(".", DEF_MENU_MAIN_DIR "screen_recording/", &dummy, 1, true);
	Util_file_save_to_file(".", DEF_MENU_MAIN_DIR "error/", &dummy, 1, true);
	Util_file_save_to_file(".", DEF_MENU_MAIN_DIR "logs/", &dummy, 1, true);

	//Init our modules.
	DEF_LOG_RESULT_SMART(result, Util_init(), (result == DEF_SUCCESS), result);
	DEF_LOG_RESULT_SMART(result, Util_watch_init(), (result == DEF_SUCCESS), result);

	Sem_init();
	Sem_suspend();
	Sem_get_config(&config);
	Sem_get_state(&state);

	if(config.screen_mode == DEF_SEM_SCREEN_MODE_AUTO)
	{
		if(osGet3DSliderState())
			is_3d = true;
		else
			is_800px = true;
	}
	else if(config.screen_mode == DEF_SEM_SCREEN_MODE_800PX)
		is_800px = true;
	else if(config.screen_mode == DEF_SEM_SCREEN_MODE_3D)
		is_3d = true;

	DEF_LOG_RESULT_SMART(result, Draw_init(is_800px, is_3d), (result == DEF_SUCCESS), result);

	//Init screen.
	Draw_frame_ready();
	Draw_screen_ready(DRAW_SCREEN_TOP_LEFT, DEF_DRAW_WHITE);
	Draw_top_ui(false, false, DEF_SEM_WIFI_SIGNAL_DISABLED, 0, NULL);
	Draw_screen_ready(DRAW_SCREEN_BOTTOM, DEF_DRAW_WHITE);
	Draw_bot_ui();
	Draw_apply_draw();
	Sem_draw_init();

	//Init rest of our modules.
	DEF_LOG_RESULT_SMART(result, Util_httpc_init(DEF_MENU_HTTP_POST_BUFFER_SIZE), (result == DEF_SUCCESS), result);
	DEF_LOG_RESULT_SMART(result, Util_curl_init(DEF_MENU_SOCKET_BUFFER_SIZE), (result == DEF_SUCCESS), result);
	DEF_LOG_RESULT_SMART(result, Util_hid_init(), (result == DEF_SUCCESS), result);
	DEF_LOG_RESULT_SMART(result, Util_hid_add_callback(Menu_hid_callback), result, result);
	DEF_LOG_RESULT_SMART(result, Util_expl_init(), (result == DEF_SUCCESS), result);
	DEF_LOG_RESULT_SMART(result, Exfont_init(), (result == DEF_SUCCESS), result);
	DEF_LOG_RESULT_SMART(result, Util_err_init(), (result == DEF_SUCCESS), result);

	for (uint16_t i = 0; i < DEF_EXFONT_NUM_OF_FONT_NAME; i++)
		Exfont_set_external_font_request_state(i, true);

	Exfont_request_load_external_font();

	menu_thread_run = true;
	menu_worker_thread = threadCreate(Menu_worker_thread, (void*)(""), DEF_THREAD_STACKSIZE * 2, DEF_THREAD_PRIORITY_ABOVE_NORMAL, 0, false);

#if (DEF_CURL_API_ENABLE || DEF_HTTPC_API_ENABLE)
	menu_update_thread = threadCreate(Menu_update_thread, (void*)(""), DEF_THREAD_STACKSIZE, DEF_THREAD_PRIORITY_REALTIME, 1, true);
#endif

#if (DEF_CURL_API_ENABLE || DEF_HTTPC_API_ENABLE)
	if (config.is_send_info_allowed)
		menu_send_app_info_thread = threadCreate(Menu_send_app_info_thread, (void*)(""), DEF_THREAD_STACKSIZE, DEF_THREAD_PRIORITY_LOW, 1, true);
#endif

	//Load sub application icons.
#ifdef DEF_SAPP0_ENABLE_ICON
	menu_icon_texture_num[0] = Draw_get_free_sheet_num();
	DEF_LOG_RESULT_SMART(result, Draw_load_texture(DEF_SAPP0_ICON_PATH, menu_icon_texture_num[0], cache, 0, 1), (result == DEF_SUCCESS), result);
	menu_icon_image[0].c2d = cache[0];
#endif

#ifdef DEF_SAPP1_ENABLE_ICON
	menu_icon_texture_num[1] = Draw_get_free_sheet_num();
	DEF_LOG_RESULT_SMART(result, Draw_load_texture(DEF_SAPP1_ICON_PATH, menu_icon_texture_num[1], cache, 0, 1), (result == DEF_SUCCESS), result);
	menu_icon_image[1].c2d = cache[0];
#endif

#ifdef DEF_SAPP2_ENABLE_ICON
	menu_icon_texture_num[2] = Draw_get_free_sheet_num();
	DEF_LOG_RESULT_SMART(result, Draw_load_texture(DEF_SAPP2_ICON_PATH, menu_icon_texture_num[2], cache, 0, 1), (result == DEF_SUCCESS), result);
	menu_icon_image[2].c2d = cache[0];
#endif

#ifdef DEF_SAPP3_ENABLE_ICON
	menu_icon_texture_num[3] = Draw_get_free_sheet_num();
	DEF_LOG_RESULT_SMART(result, Draw_load_texture(DEF_SAPP3_ICON_PATH, menu_icon_texture_num[3], cache, 0, 1), (result == DEF_SUCCESS), result);
	menu_icon_image[3].c2d = cache[0];
#endif

#ifdef DEF_SAPP4_ENABLE_ICON
	menu_icon_texture_num[4] = Draw_get_free_sheet_num();
	DEF_LOG_RESULT_SMART(result, Draw_load_texture(DEF_SAPP4_ICON_PATH, menu_icon_texture_num[4], cache, 0, 1), (result == DEF_SUCCESS), result);
	menu_icon_image[4].c2d = cache[0];
#endif

#ifdef DEF_SAPP5_ENABLE_ICON
	menu_icon_texture_num[5] = Draw_get_free_sheet_num();
	DEF_LOG_RESULT_SMART(result, Draw_load_texture(DEF_SAPP5_ICON_PATH, menu_icon_texture_num[5], cache, 0, 1), (result == DEF_SUCCESS), result);
	menu_icon_image[5].c2d = cache[0];
#endif

#ifdef DEF_SAPP6_ENABLE_ICON
	menu_icon_texture_num[6] = Draw_get_free_sheet_num();
	DEF_LOG_RESULT_SMART(result, Draw_load_texture(DEF_SAPP6_ICON_PATH, menu_icon_texture_num[6], cache, 0, 1), (result == DEF_SUCCESS), result);
	menu_icon_image[6].c2d = cache[0];
#endif

#ifdef DEF_SAPP7_ENABLE_ICON
	menu_icon_texture_num[7] = Draw_get_free_sheet_num();
	DEF_LOG_RESULT_SMART(result, Draw_load_texture(DEF_SAPP7_ICON_PATH, menu_icon_texture_num[7], cache, 0, 1), (result == DEF_SUCCESS), result);
	menu_icon_image[7].c2d = cache[0];
#endif

#ifdef DEF_SEM_ENABLE_ICON
	menu_icon_texture_num[8] = Draw_get_free_sheet_num();
	DEF_LOG_RESULT_SMART(result, Draw_load_texture(DEF_SEM_ICON_PATH, menu_icon_texture_num[8], cache, 0, 2), (result == DEF_SUCCESS), result);
	menu_icon_image[8].c2d = cache[0];
	menu_icon_image[9].c2d = cache[1];
#endif

	for(uint8_t i = 0; i < DEF_MENU_NUM_OF_SUB_APP; i++)
	{
		menu_sapp_button[i] = Draw_get_empty_image();
		menu_sapp_close_button[i] = Draw_get_empty_image();
	}
	menu_sem_button = Draw_get_empty_image();

	Util_watch_add(WATCH_HANDLE_MAIN_MENU, &menu_must_exit, sizeof(menu_must_exit));
	Util_watch_add(WATCH_HANDLE_MAIN_MENU, &menu_check_exit_request, sizeof(menu_check_exit_request));

	Util_watch_add(WATCH_HANDLE_MAIN_MENU, &menu_sem_button.selected, sizeof(menu_sem_button.selected));
	for(uint8_t i = 0; i < DEF_MENU_NUM_OF_SUB_APP; i++)
	{
		Util_watch_add(WATCH_HANDLE_MAIN_MENU, &menu_init_request[i], sizeof(menu_init_request[i]));
		Util_watch_add(WATCH_HANDLE_MAIN_MENU, &menu_exit_request[i], sizeof(menu_exit_request[i]));

		Util_watch_add(WATCH_HANDLE_MAIN_MENU, &menu_sapp_button[i].selected, sizeof(menu_sapp_button[i].selected));
		Util_watch_add(WATCH_HANDLE_MAIN_MENU, &menu_sapp_close_button[i].selected, sizeof(menu_sapp_close_button[i].selected));
	}

	Menu_resume();
	DEF_LOG_STRING("Initialized.");
}

void Menu_exit(void)
{
	DEF_LOG_STRING("Exiting...");
	bool draw = !aptShouldClose();
	uint32_t result = DEF_ERR_OTHER;

	menu_thread_run = false;

	#ifdef DEF_SAPP0_ENABLE
	if (Sapp0_query_init_flag())
		Sapp0_exit(draw);
	#endif
	#ifdef DEF_SAPP1_ENABLE
	if (Sapp1_query_init_flag())
		Sapp1_exit(draw);
	#endif
	#ifdef DEF_SAPP2_ENABLE
	if (Sapp2_query_init_flag())
		Sapp2_exit(draw);
	#endif
	#ifdef DEF_SAPP3_ENABLE
	if (Sapp3_query_init_flag())
		Sapp3_exit(draw);
	#endif
	#ifdef DEF_SAPP4_ENABLE
	if (Sapp4_query_init_flag())
		Sapp4_exit(draw);
	#endif
	#ifdef DEF_SAPP5_ENABLE
	if (Sapp5_query_init_flag())
		Sapp5_exit(draw);
	#endif
	#ifdef DEF_SAPP6_ENABLE
	if (Sapp6_query_init_flag())
		Sapp6_exit(draw);
	#endif
	#ifdef DEF_SAPP7_ENABLE
	if (Sapp7_query_init_flag())
		Sapp7_exit(draw);
	#endif
	if (Sem_query_init_flag())
		Sem_exit();

	for(uint8_t i = 0; i < (DEF_MENU_NUM_OF_SUB_APP + 1); i++)
		Draw_free_texture(menu_icon_texture_num[i]);

	Util_hid_remove_callback(Menu_hid_callback);
	Util_hid_exit();
	Util_expl_exit();
	Exfont_exit();
	Util_err_exit();
	Util_exit();
	Util_cpu_usage_exit();

	DEF_LOG_RESULT_SMART(result, threadJoin(menu_worker_thread, DEF_THREAD_WAIT_TIME), (result == DEF_SUCCESS), result);
	threadFree(menu_worker_thread);

#if (DEF_CURL_API_ENABLE || DEF_HTTPC_API_ENABLE)
	DEF_LOG_RESULT_SMART(result, threadJoin(menu_send_app_info_thread, DEF_THREAD_WAIT_TIME), (result == DEF_SUCCESS), result);
	DEF_LOG_RESULT_SMART(result, threadJoin(menu_update_thread, DEF_THREAD_WAIT_TIME), (result == DEF_SUCCESS), result);
#endif

	Util_watch_remove(WATCH_HANDLE_MAIN_MENU, &menu_must_exit);
	Util_watch_remove(WATCH_HANDLE_MAIN_MENU, &menu_check_exit_request);

	Util_watch_remove(WATCH_HANDLE_MAIN_MENU, &menu_sem_button.selected);
	for(uint8_t i = 0; i < DEF_MENU_NUM_OF_SUB_APP; i++)
	{
		Util_watch_remove(WATCH_HANDLE_MAIN_MENU, &menu_init_request[i]);
		Util_watch_remove(WATCH_HANDLE_MAIN_MENU, &menu_exit_request[i]);

		Util_watch_remove(WATCH_HANDLE_MAIN_MENU, &menu_sapp_button[i].selected);
		Util_watch_remove(WATCH_HANDLE_MAIN_MENU, &menu_sapp_close_button[i].selected);
	}

	Util_watch_exit();
	Util_log_exit();
	Util_httpc_exit();
	Util_curl_exit();

	fsExit();
	acExit();
	aptExit();
	mcuHwcExit();
	ptmuExit();
	romfsExit();
	cfguExit();
	amExit();
	Draw_exit();

	DEF_LOG_STRING("Exited.");
}

bool Menu_add_worker_thread_callback(void (*callback)(void))
{
	LightLock_Lock(&menu_callback_mutex);

	for(uint16_t i = 0; i < DEF_MENU_NUM_OF_CALLBACKS; i++)
	{
		if(menu_worker_thread_callbacks[i] == callback)
			goto success;//Already exist.
	}

	for(uint16_t i = 0; i < DEF_MENU_NUM_OF_CALLBACKS; i++)
	{
		if(!menu_worker_thread_callbacks[i])
		{
			menu_worker_thread_callbacks[i] = callback;
			goto success;
		}
	}

	//No free spaces left.
	LightLock_Unlock(&menu_callback_mutex);
	return false;

	success:
	LightLock_Unlock(&menu_callback_mutex);
	return true;
}

void Menu_remove_worker_thread_callback(void (*callback)(void))
{
	LightLock_Lock(&menu_callback_mutex);

	for(uint16_t i = 0; i < DEF_MENU_NUM_OF_CALLBACKS; i++)
	{
		if(menu_worker_thread_callbacks[i] == callback)
		{
			menu_worker_thread_callbacks[i] = NULL;
			break;
		}
	}

	LightLock_Unlock(&menu_callback_mutex);
}

void Menu_main(void)
{
	bool is_800px = false;
	bool is_3d = false;
	uint32_t color = DEF_DRAW_BLACK;
	uint32_t back_color = DEF_DRAW_WHITE;
	Sem_config config = { 0, };
	Sem_state state = { 0, };

	Sem_get_config(&config);
	Sem_get_state(&state);

	is_800px = (config.screen_mode == DEF_SEM_SCREEN_MODE_800PX);
	is_3d = (config.screen_mode == DEF_SEM_SCREEN_MODE_3D);
	if(config.screen_mode == DEF_SEM_SCREEN_MODE_AUTO)
	{
		if(osGet3DSliderState())
		{
			is_3d = true;
			is_800px = false;
		}
		else
		{
			is_3d = false;
			is_800px = ((state.console_model == DEF_SEM_MODEL_OLD2DS) ? false : true);
		}
	}

	if(state.console_model == DEF_SEM_MODEL_OLD2DS && is_800px)
	{
		is_800px = false;
		config.screen_mode = DEF_SEM_SCREEN_MODE_AUTO;
		Sem_set_config(&config);
	}
	if((state.console_model == DEF_SEM_MODEL_OLD2DS || state.console_model == DEF_SEM_MODEL_NEW2DSXL) && is_3d)
	{
		is_3d = false;
		config.screen_mode = DEF_SEM_SCREEN_MODE_AUTO;
		Sem_set_config(&config);
	}

	//Update screen mode here.
	if(is_3d != Draw_is_3d_mode() || is_800px != Draw_is_800px_mode())
	{
		uint32_t result = DEF_ERR_OTHER;

		DEF_LOG_RESULT_SMART(result, Draw_reinit(is_800px, is_3d), (result == DEF_SUCCESS), result);
		Draw_set_refresh_needed(true);
	}

	if(config.is_debug)
		Draw_set_refresh_needed(true);

	if (menu_main_run)
	{
		Watch_handle_bit watch_handle_bit = (DEF_WATCH_HANDLE_BIT_GLOBAL | DEF_WATCH_HANDLE_BIT_MAIN_MENU);

		//Check if we should update the screen.
		if(Util_watch_is_changed(watch_handle_bit) || Draw_is_refresh_needed() || !config.is_eco)
		{
			Draw_set_refresh_needed(false);

			if (config.is_night)
			{
				color = DEF_DRAW_WHITE;
				back_color = DEF_DRAW_BLACK;
			}

			Draw_frame_ready();
			Draw_screen_ready(DRAW_SCREEN_TOP_LEFT, back_color);

			if(menu_check_exit_request)
			{
				Draw_align(&menu_msg[DEF_MENU_EXIST_MSG], 0, 105, 0.5, 0.5, color, DRAW_X_ALIGN_CENTER, DRAW_Y_ALIGN_CENTER, 400, 20);
				Draw_align(&menu_msg[DEF_MENU_CONFIRM_MSG], 10, 140, 0.5, 0.5, DEF_DRAW_GREEN, DRAW_X_ALIGN_RIGHT, DRAW_Y_ALIGN_CENTER, 190, 20);
				Draw_align(&menu_msg[DEF_MENU_CANCEL_MSG], 210, 140, 0.5, 0.5, DEF_DRAW_RED, DRAW_X_ALIGN_LEFT, DRAW_Y_ALIGN_CENTER, 190, 20);
			}
			else if(menu_update_available)
			{
				Draw(&menu_msg[DEF_MENU_NEW_VERSION_MSG], 10, 30, 0.7, 0.7, DEF_DRAW_RED);
				Draw(&menu_msg[DEF_MENU_HOW_TO_UPDATE_MSG], 10, 60, 0.5, 0.5, color);
			}

			if(Util_log_query_log_show_flag())
				Util_log_draw();

			Draw_top_ui(config.is_eco, state.is_charging, state.wifi_signal, state.battery_level, state.msg);

			if(config.is_debug)
				Draw_debug_info(config.is_night, state.free_ram, state.free_linear_ram);

			if(Util_cpu_usage_query_show_flag())
				Util_cpu_usage_draw();

			Draw_screen_ready(DRAW_SCREEN_BOTTOM, back_color);

			#ifdef DEF_SAPP0_ENABLE
			Draw_texture(&menu_sapp_button[0], menu_sapp_button[0].selected ? DEF_DRAW_AQUA : DEF_DRAW_WEAK_AQUA, 0, 0, 60, 60);

			#ifdef DEF_SAPP0_ENABLE_ICON
			Draw_texture(&menu_icon_image[0], DEF_DRAW_NO_COLOR, 0, 0, 60, 60);
			#endif
			#ifdef DEF_SAPP0_ENABLE_NAME
			Draw_align_c(DEF_SAPP0_NAME, 0, 0, 0.4, 0.4, color, DRAW_X_ALIGN_CENTER, DRAW_Y_ALIGN_CENTER, 60, 60);
			#endif

			if(Sapp0_query_init_flag())
			{
				Draw_texture(&menu_sapp_close_button[0], menu_sapp_close_button[0].selected ? DEF_DRAW_RED : DEF_DRAW_WEAK_RED, 45, 0, 15, 15);
				Draw_c("X", 47.5, 0, 0.5, 0.5, DEF_DRAW_RED);
			}
			#endif
			#ifdef DEF_SAPP1_ENABLE
			Draw_texture(&menu_sapp_button[1], menu_sapp_button[1].selected ? DEF_DRAW_AQUA : DEF_DRAW_WEAK_AQUA, 80, 0, 60, 60);

			#ifdef DEF_SAPP1_ENABLE_ICON
			Draw_texture(&menu_icon_image[1], DEF_DRAW_NO_COLOR, 80, 0, 60, 60);
			#endif
			#ifdef DEF_SAPP1_ENABLE_NAME
			Draw_align_c(DEF_SAPP1_NAME, 80, 0, 0.4, 0.4, color, DRAW_X_ALIGN_CENTER, DRAW_Y_ALIGN_CENTER, 60, 60);
			#endif

			if(Sapp1_query_init_flag())
			{
				Draw_texture(&menu_sapp_close_button[1], menu_sapp_close_button[1].selected ? DEF_DRAW_RED : DEF_DRAW_WEAK_RED, 125, 0, 15, 15.0);
				Draw_c("X", 127.5, 0, 0.5, 0.5, DEF_DRAW_RED);
			}
			#endif
			#ifdef DEF_SAPP2_ENABLE
			Draw_texture(&menu_sapp_button[2], menu_sapp_button[2].selected ? DEF_DRAW_AQUA : DEF_DRAW_WEAK_AQUA, 160, 0, 60, 60);

			#ifdef DEF_SAPP2_ENABLE_ICON
			Draw_texture(&menu_icon_image[2], DEF_DRAW_NO_COLOR, 160, 0, 60, 60);
			#endif
			#ifdef DEF_SAPP2_ENABLE_NAME
			Draw_align_c(DEF_SAPP2_NAME, 160, 0, 0.4, 0.4, color, DRAW_X_ALIGN_CENTER, DRAW_Y_ALIGN_CENTER, 60, 60);
			#endif

			if(Sapp2_query_init_flag())
			{
				Draw_texture(&menu_sapp_close_button[2], menu_sapp_close_button[2].selected ? DEF_DRAW_RED : DEF_DRAW_WEAK_RED, 205, 0, 15, 15.0);
				Draw_c("X", 207.5, 0, 0.5, 0.5, DEF_DRAW_RED);
			}
			#endif
			#ifdef DEF_SAPP3_ENABLE
			Draw_texture(&menu_sapp_button[3], menu_sapp_button[3].selected ? DEF_DRAW_AQUA : DEF_DRAW_WEAK_AQUA, 240, 0, 60, 60);

			#ifdef DEF_SAPP3_ENABLE_ICON
			Draw_texture(&menu_icon_image[3], DEF_DRAW_NO_COLOR, 240, 0, 60, 60);
			#endif
			#ifdef DEF_SAPP3_ENABLE_NAME
			Draw_align_c(DEF_SAPP3_NAME, 240, 0, 0.4, 0.4, color, DRAW_X_ALIGN_CENTER, DRAW_Y_ALIGN_CENTER, 60, 60);
			#endif

			if(Sapp3_query_init_flag())
			{
				Draw_texture(&menu_sapp_close_button[3], menu_sapp_close_button[3].selected ? DEF_DRAW_RED : DEF_DRAW_WEAK_RED, 285, 0, 15, 15.0);
				Draw_c("X", 287.5, 0, 0.5, 0.5, DEF_DRAW_RED);
			}
			#endif
			#ifdef DEF_SAPP4_ENABLE
			Draw_texture(&menu_sapp_button[4], menu_sapp_button[4].selected ? DEF_DRAW_AQUA : DEF_DRAW_WEAK_AQUA, 0, 80, 60, 60);

			#ifdef DEF_SAPP4_ENABLE_ICON
			Draw_texture(&menu_icon_image[4], DEF_DRAW_NO_COLOR, 0, 80, 60, 60);
			#endif
			#ifdef DEF_SAPP4_ENABLE_NAME
			Draw_align_c(DEF_SAPP4_NAME, 0, 80, 0.4, 0.4, color, DRAW_X_ALIGN_CENTER, DRAW_Y_ALIGN_CENTER, 60, 60);
			#endif

			if(Sapp4_query_init_flag())
			{
				Draw_texture(&menu_sapp_close_button[4], menu_sapp_close_button[4].selected ? DEF_DRAW_RED : DEF_DRAW_WEAK_RED, 45, 80, 15, 15.0);
				Draw_c("X", 47.5, 80, 0.5, 0.5, DEF_DRAW_RED);
			}
			#endif
			#ifdef DEF_SAPP5_ENABLE
			Draw_texture(&menu_sapp_button[5], menu_sapp_button[5].selected ? DEF_DRAW_AQUA : DEF_DRAW_WEAK_AQUA, 80, 80, 60, 60);

			#ifdef DEF_SAPP5_ENABLE_ICON
			Draw_texture(&menu_icon_image[5], DEF_DRAW_NO_COLOR, 80, 80, 60, 60);
			#endif
			#ifdef DEF_SAPP5_ENABLE_NAME
			Draw_align_c(DEF_SAPP5_NAME, 80, 80, 0.4, 0.4, color, DRAW_X_ALIGN_CENTER, DRAW_Y_ALIGN_CENTER, 60, 60);
			#endif

			if(Sapp5_query_init_flag())
			{
				Draw_texture(&menu_sapp_close_button[5], menu_sapp_close_button[5].selected ? DEF_DRAW_RED : DEF_DRAW_WEAK_RED, 125, 80, 15, 15.0);
				Draw_c("X", 127.5, 80, 0.5, 0.5, DEF_DRAW_RED);
			}
			#endif
			#ifdef DEF_SAPP6_ENABLE
			Draw_texture(&menu_sapp_button[6], menu_sapp_button[6].selected ? DEF_DRAW_AQUA : DEF_DRAW_WEAK_AQUA, 160, 80, 60, 60);

			#ifdef DEF_SAPP6_ENABLE_ICON
			Draw_texture(&menu_icon_image[6], DEF_DRAW_NO_COLOR, 160, 80, 60, 60);
			#endif
			#ifdef DEF_SAPP6_ENABLE_NAME
			Draw_align_c(DEF_SAPP6_NAME, 160, 80, 0.4, 0.4, color, DRAW_X_ALIGN_CENTER, DRAW_Y_ALIGN_CENTER, 60, 60);
			#endif

			if(Sapp6_query_init_flag())
			{
				Draw_texture(&menu_sapp_close_button[6], menu_sapp_close_button[6].selected ? DEF_DRAW_RED : DEF_DRAW_WEAK_RED, 205, 80, 15, 15.0);
				Draw_c("X", 207.5, 80, 0.5, 0.5, DEF_DRAW_RED);
			}
			#endif
			#ifdef DEF_SAPP7_ENABLE
			Draw_texture(&menu_sapp_button[7], menu_sapp_button[7].selected ? DEF_DRAW_AQUA : DEF_DRAW_WEAK_AQUA, 240, 80, 60, 60);

			#ifdef DEF_SAPP7_ENABLE_ICON
			Draw_texture(&menu_icon_image[7], DEF_DRAW_NO_COLOR, 240, 80, 60, 60);
			#endif
			#ifdef DEF_SAPP7_ENABLE_NAME
			Draw_align_c(DEF_SAPP7_NAME, 240, 80, 0.4, 0.4, color, DRAW_X_ALIGN_CENTER, DRAW_Y_ALIGN_CENTER, 60, 60);
			#endif

			if(Sapp7_query_init_flag())
			{
				Draw_texture(&menu_sapp_close_button[7], menu_sapp_close_button[7].selected ? DEF_DRAW_RED : DEF_DRAW_WEAK_RED, 285, 80, 15, 15.0);
				Draw_c("X", 287.5, 80, 0.5, 0.5, DEF_DRAW_RED);
			}
			#endif

			Draw_texture(&menu_sem_button, menu_sem_button.selected ? DEF_DRAW_AQUA : DEF_DRAW_WEAK_AQUA, 260, 170, 60, 60);

			#ifdef DEF_SEM_ENABLE_ICON
			Draw_texture(&menu_icon_image[8 + config.is_night], DEF_DRAW_NO_COLOR, 260, 170, 60, 60);
			#endif
			#ifdef DEF_SEM_ENABLE_NAME
			Draw_c(DEF_SEM_NAME, 270, 205, 0.4, 0.4, color);
			#endif

			if(Util_err_query_error_show_flag())
				Util_err_draw();

			Draw_bot_ui();

			Draw_apply_draw();
		}
		else
			gspWaitForVBlank();

		#ifdef DEF_SAPP0_ENABLE
		if(menu_init_request[0])
		{
			Sapp0_init(true);
			menu_init_request[0] = false;
		}
		else if(menu_exit_request[0])
		{
			Sapp0_exit(true);
			menu_exit_request[0] = false;
		}
		#endif
		#ifdef DEF_SAPP1_ENABLE
		if(menu_init_request[1])
		{
			Sapp1_init(true);
			menu_init_request[1] = false;
		}
		else if(menu_exit_request[1])
		{
			Sapp1_exit(true);
			menu_exit_request[1] = false;
		}
		#endif
		#ifdef DEF_SAPP2_ENABLE
		if(menu_init_request[2])
		{
			Sapp2_init(true);
			menu_init_request[2] = false;
		}
		else if(menu_exit_request[2])
		{
			Sapp2_exit(true);
			menu_exit_request[2] = false;
		}
		#endif
		#ifdef DEF_SAPP3_ENABLE
		if(menu_init_request[3])
		{
			Sapp3_init(true);
			menu_init_request[3] = false;
		}
		else if(menu_exit_request[3])
		{
			Sapp3_exit(true);
			menu_exit_request[3] = false;
		}
		#endif
		#ifdef DEF_SAPP4_ENABLE
		if(menu_init_request[4])
		{
			Sapp4_init(true);
			menu_init_request[4] = false;
		}
		else if(menu_exit_request[4])
		{
			Sapp4_exit(true);
			menu_exit_request[4] = false;
		}
		#endif
		#ifdef DEF_SAPP5_ENABLE
		if(menu_init_request[5])
		{
			Sapp5_init(true);
			menu_init_request[5] = false;
		}
		else if(menu_exit_request[5])
		{
			Sapp5_exit(true);
			menu_exit_request[5] = false;
		}
		#endif
		#ifdef DEF_SAPP6_ENABLE
		if(menu_init_request[6])
		{
			Sapp6_init(true);
			menu_init_request[6] = false;
		}
		else if(menu_exit_request[6])
		{
			Sapp6_exit(true);
			menu_exit_request[6] = false;
		}
		#endif
		#ifdef DEF_SAPP7_ENABLE
		if(menu_init_request[7])
		{
			Sapp7_init(true);
			menu_init_request[7] = false;
		}
		else if(menu_exit_request[7])
		{
			Sapp7_exit(true);
			menu_exit_request[7] = false;
		}
		#endif
	}
	#ifdef DEF_SAPP0_ENABLE
	else if (Sapp0_query_running_flag())
		Sapp0_main();
	#endif
	#ifdef DEF_SAPP1_ENABLE
	else if (Sapp1_query_running_flag())
		Sapp1_main();
	#endif
	#ifdef DEF_SAPP2_ENABLE
	else if (Sapp2_query_running_flag())
		Sapp2_main();
	#endif
	#ifdef DEF_SAPP3_ENABLE
	else if (Sapp3_query_running_flag())
		Sapp3_main();
	#endif
	#ifdef DEF_SAPP4_ENABLE
	else if (Sapp4_query_running_flag())
		Sapp4_main();
	#endif
	#ifdef DEF_SAPP5_ENABLE
	else if (Sapp5_query_running_flag())
		Sapp5_main();
	#endif
	#ifdef DEF_SAPP6_ENABLE
	else if (Sapp6_query_running_flag())
		Sapp6_main();
	#endif
	#ifdef DEF_SAPP7_ENABLE
	else if (Sapp7_query_running_flag())
		Sapp7_main();
	#endif
	else if (Sem_query_running_flag())
		Sem_main();
	else
		menu_main_run = true;
}

void Menu_hid_callback(void)
{
	Hid_info key = { 0, };

	Util_hid_query_key_state(&key);
	if(menu_main_run)
	{
		if(!aptShouldJumpToHome())
		{
			if (Util_err_query_error_show_flag())
				Util_err_main(key);
			else
			{
				if(menu_check_exit_request)
				{
					if (key.p_a)
						menu_must_exit = true;
					else if (key.p_b)
						menu_check_exit_request = false;
				}
				else
				{
					if(Util_hid_is_pressed(key, *Draw_get_bot_ui_button()))
						Draw_get_bot_ui_button()->selected = true;
					else if (key.p_start || (Util_hid_is_released(key, *Draw_get_bot_ui_button()) && Draw_get_bot_ui_button()->selected))
						menu_check_exit_request = true;
					else if (key.p_select)
						Util_log_set_log_show_flag(!Util_log_query_log_show_flag());
					#ifdef DEF_SAPP0_ENABLE
					else if (Util_hid_is_pressed(key, menu_sapp_close_button[0]) && Sapp0_query_init_flag())
						menu_sapp_close_button[0].selected = true;
					else if (Util_hid_is_released(key, menu_sapp_close_button[0]) && Sapp0_query_init_flag() && menu_sapp_close_button[0].selected)
					{
						menu_exit_request[0] = true;
						while(menu_exit_request[0])
							Util_sleep(20000);
					}
					else if (Util_hid_is_pressed(key, menu_sapp_button[0]))
						menu_sapp_button[0].selected = true;
					else if (Util_hid_is_released(key, menu_sapp_button[0]) && menu_sapp_button[0].selected)
					{
						if (!Sapp0_query_init_flag())
						{
							menu_init_request[0] = true;
							while(menu_init_request[0])
								Util_sleep(20000);
						}
						else
							Sapp0_resume();
					}
					#endif
					#ifdef DEF_SAPP1_ENABLE
					else if (Util_hid_is_pressed(key, menu_sapp_close_button[1]) && Sapp1_query_init_flag())
						menu_sapp_close_button[1].selected = true;
					else if (Util_hid_is_released(key, menu_sapp_close_button[1]) && Sapp1_query_init_flag() && menu_sapp_close_button[1].selected)
					{
						menu_exit_request[1] = true;
						while(menu_exit_request[1])
							Util_sleep(20000);
					}
					else if (Util_hid_is_pressed(key, menu_sapp_button[1]))
						menu_sapp_button[1].selected = true;
					else if (Util_hid_is_released(key, menu_sapp_button[1]) && menu_sapp_button[1].selected)
					{
						if (!Sapp1_query_init_flag())
						{
							menu_init_request[1] = true;
							while(menu_init_request[1])
								Util_sleep(20000);
						}
						else
							Sapp1_resume();
					}
					#endif
					#ifdef DEF_SAPP2_ENABLE
					else if (Util_hid_is_pressed(key, menu_sapp_close_button[2]) && Sapp2_query_init_flag())
						menu_sapp_close_button[2].selected = true;
					else if (Util_hid_is_released(key, menu_sapp_close_button[2]) && Sapp2_query_init_flag() && menu_sapp_close_button[2].selected)
					{
						menu_exit_request[2] = true;
						while(menu_exit_request[2])
							Util_sleep(20000);
					}
					else if (Util_hid_is_pressed(key, menu_sapp_button[2]))
						menu_sapp_button[2].selected = true;
					else if (Util_hid_is_released(key, menu_sapp_button[2]) && menu_sapp_button[2].selected)
					{
						if (!Sapp2_query_init_flag())
						{
							menu_init_request[2] = true;
							while(menu_init_request[2])
								Util_sleep(20000);
						}
						else
							Sapp2_resume();
					}
					#endif
					#ifdef DEF_SAPP3_ENABLE
					else if (Util_hid_is_pressed(key, menu_sapp_close_button[3]) && Sapp3_query_init_flag())
						menu_sapp_close_button[3].selected = true;
					else if (Util_hid_is_released(key, menu_sapp_close_button[3]) && Sapp3_query_init_flag() && menu_sapp_close_button[3].selected)
					{
						menu_exit_request[3] = true;
						while(menu_exit_request[3])
							Util_sleep(20000);
					}
					else if (Util_hid_is_pressed(key, menu_sapp_button[3]))
						menu_sapp_button[3].selected = true;
					else if (Util_hid_is_released(key, menu_sapp_button[3]) && menu_sapp_button[3].selected)
					{
						if (!Sapp3_query_init_flag())
						{
							menu_init_request[3] = true;
							while(menu_init_request[3])
								Util_sleep(20000);
						}
						else
							Sapp3_resume();
					}
					#endif
					#ifdef DEF_SAPP4_ENABLE
					else if (Util_hid_is_pressed(key, menu_sapp_close_button[4]) && Sapp4_query_init_flag())
						menu_sapp_close_button[4].selected = true;
					else if (Util_hid_is_released(key, menu_sapp_close_button[4]) && Sapp4_query_init_flag() && menu_sapp_close_button[4].selected)
					{
						menu_exit_request[4] = true;
						while(menu_exit_request[4])
							Util_sleep(20000);
					}
					else if (Util_hid_is_pressed(key, menu_sapp_button[4]))
						menu_sapp_button[4].selected = true;
					else if (Util_hid_is_released(key, menu_sapp_button[4]) && menu_sapp_button[4].selected)
					{
						if (!Sapp4_query_init_flag())
						{
							menu_init_request[4] = true;
							while(menu_init_request[4])
								Util_sleep(20000);
						}
						else
							Sapp4_resume();
					}
					#endif
					#ifdef DEF_SAPP5_ENABLE
					else if (Util_hid_is_pressed(key, menu_sapp_close_button[5]) && Sapp5_query_init_flag())
						menu_sapp_close_button[5].selected = true;
					else if (Util_hid_is_released(key, menu_sapp_close_button[5]) && Sapp5_query_init_flag() && menu_sapp_close_button[5].selected)
					{
						menu_exit_request[5] = true;
						while(menu_exit_request[5])
							Util_sleep(20000);
					}
					else if (Util_hid_is_pressed(key, menu_sapp_button[5]))
						menu_sapp_button[5].selected = true;
					else if (Util_hid_is_released(key, menu_sapp_button[5]) && menu_sapp_button[5].selected)
					{
						if (!Sapp5_query_init_flag())
						{
							menu_init_request[5] = true;
							while(menu_init_request[5])
								Util_sleep(20000);
						}
						else
							Sapp5_resume();
					}
					#endif
					#ifdef DEF_SAPP6_ENABLE
					else if (Util_hid_is_pressed(key, menu_sapp_close_button[6]) && Sapp6_query_init_flag())
						menu_sapp_close_button[6].selected = true;
					else if (Util_hid_is_released(key, menu_sapp_close_button[6]) && Sapp6_query_init_flag() && menu_sapp_close_button[6].selected)
					{
						menu_exit_request[6] = true;
						while(menu_exit_request[6])
							Util_sleep(20000);
					}
					else if (Util_hid_is_pressed(key, menu_sapp_button[6]))
						menu_sapp_button[6].selected = true;
					else if (Util_hid_is_released(key, menu_sapp_button[6]) && menu_sapp_button[6].selected)
					{
						if (!Sapp6_query_init_flag())
						{
							menu_init_request[6] = true;
							while(menu_init_request[6])
								Util_sleep(20000);
						}
						else
							Sapp6_resume();
					}
					#endif
					#ifdef DEF_SAPP7_ENABLE
					else if (Util_hid_is_pressed(key, menu_sapp_close_button[7]) && Sapp7_query_init_flag())
						menu_sapp_close_button[7].selected = true;
					else if (Util_hid_is_released(key, menu_sapp_close_button[7]) && Sapp7_query_init_flag() && menu_sapp_close_button[7].selected)
					{
						menu_exit_request[7] = true;
						while(menu_exit_request[7])
							Util_sleep(20000);
					}
					else if (Util_hid_is_pressed(key, menu_sapp_button[7]))
						menu_sapp_button[7].selected = true;
					else if (Util_hid_is_released(key, menu_sapp_button[7]) && menu_sapp_button[7].selected)
					{
						if (!Sapp7_query_init_flag())
						{
							menu_init_request[7] = true;
							while(menu_init_request[7])
								Util_sleep(20000);
						}
						else
							Sapp7_resume();
					}
					#endif
					else if (Util_hid_is_pressed(key, menu_sem_button))
						menu_sem_button.selected = true;
					else if (Util_hid_is_released(key, menu_sem_button) && menu_sem_button.selected)
						Sem_resume();
				}

				if(!key.p_touch && !key.h_touch)
				{
					for(uint8_t i = 0; i < DEF_MENU_NUM_OF_SUB_APP; i++)
					{
						menu_sapp_button[i].selected = false;
						menu_sapp_close_button[i].selected = false;
					}
					menu_sem_button.selected = false;
					Draw_get_bot_ui_button()->selected = false;
				}
			}

			if(Util_log_query_log_show_flag())
				Util_log_main(key);
		}
	}
	#ifdef DEF_SAPP0_ENABLE
	else if (Sapp0_query_running_flag())
		Sapp0_hid(key);
	#endif
	#ifdef DEF_SAPP1_ENABLE
	else if (Sapp1_query_running_flag())
		Sapp1_hid(key);
	#endif
	#ifdef DEF_SAPP2_ENABLE
	else if (Sapp2_query_running_flag())
		Sapp2_hid(key);
	#endif
	#ifdef DEF_SAPP3_ENABLE
	else if (Sapp3_query_running_flag())
		Sapp3_hid(key);
	#endif
	#ifdef DEF_SAPP4_ENABLE
	else if (Sapp4_query_running_flag())
		Sapp4_hid(key);
	#endif
	#ifdef DEF_SAPP5_ENABLE
	else if (Sapp5_query_running_flag())
		Sapp5_hid(key);
	#endif
	#ifdef DEF_SAPP6_ENABLE
	else if (Sapp6_query_running_flag())
		Sapp6_hid(key);
	#endif
	#ifdef DEF_SAPP7_ENABLE
	else if (Sapp7_query_running_flag())
		Sapp7_hid(key);
	#endif
	else if (Sem_query_running_flag())
		Sem_hid(key);
}

#if (DEF_CURL_API_ENABLE || DEF_HTTPC_API_ENABLE)
void Menu_send_app_info_thread(void* arg)
{
	DEF_LOG_STRING("Thread started.");
	bool is_new3ds = false;
	uint8_t* dl_data = NULL;
	char system_ver_char[0x50] = " ";
	std::string new3ds = "";
	OS_VersionBin os_ver = { 0, };
	Sem_config config = { 0, };
	Sem_state state = { 0, };

	Sem_get_config(&config);
	Sem_get_state(&state);

	osGetSystemVersionDataString(&os_ver, &os_ver, system_ver_char, 0x50);
	std::string system_ver = system_ver_char;

	APT_CheckNew3DS(&is_new3ds);
	new3ds = is_new3ds ? "yes" : "no";

	std::string send_data = (std::string)"{ \"app_ver\": \"" + DEF_MENU_CURRENT_APP_VER + "\",\"system_ver\" : \""
	+ system_ver + "\",\"start_num_of_app\" : \"" + std::to_string(state.num_of_launch) + "\",\"language\" : \""
	+ config.lang + "\",\"new3ds\" : \"" + new3ds + "\",\"time_to_enter_sleep\" : \"" + std::to_string(config.time_to_turn_off_lcd)
	+ "\",\"scroll_speed\" : \"" + std::to_string(config.scroll_speed) + "\" }";

#if DEF_CURL_API_ENABLE
	Util_curl_post_and_dl_data(DEF_MENU_SEND_APP_INFO_URL, (uint8_t*)send_data.c_str(), send_data.length(), &dl_data, 0x10000, NULL, NULL, NULL, 5, NULL);
#else
	Util_httpc_post_and_dl_data(DEF_MENU_SEND_APP_INFO_URL, (uint8_t*)send_data.c_str(), send_data.length(), &dl_data, 0x10000, NULL, NULL, 5, NULL);
#endif

	free(dl_data);
	dl_data = NULL;

	DEF_LOG_STRING("Thread exit.");
	threadExit(0);
}
#endif

void Menu_worker_thread(void* arg)
{
	DEF_LOG_STRING("Thread started.");

	while (menu_thread_run)
	{
		LightLock_Lock(&menu_callback_mutex);

		//Call callback functions.
		for(uint16_t i = 0; i < DEF_MENU_NUM_OF_CALLBACKS; i++)
		{
			if(menu_worker_thread_callbacks[i])
				menu_worker_thread_callbacks[i]();
		}

		LightLock_Unlock(&menu_callback_mutex);

		gspWaitForVBlank();
	}

	DEF_LOG_STRING("Thread exit.");
	threadExit(0);
}

#if (DEF_CURL_API_ENABLE || DEF_HTTPC_API_ENABLE)
void Menu_update_thread(void* arg)
{
	DEF_LOG_STRING("Thread started.");
	uint8_t* http_buffer = NULL;
	uint32_t result = DEF_ERR_OTHER;

#if DEF_CURL_API_ENABLE
	DEF_LOG_RESULT_SMART(result, Util_curl_dl_data(DEF_SEM_CHECK_UPDATE_URL, &http_buffer, 0x1000, NULL, NULL, 3, NULL), (result == DEF_SUCCESS), result);
#else
	DEF_LOG_RESULT_SMART(result, Util_httpc_dl_data(DEF_SEM_CHECK_UPDATE_URL, &http_buffer, 0x1000, NULL, NULL, 3, NULL), (result == DEF_SUCCESS), result);
#endif

	if(result == DEF_SUCCESS)
	{
		size_t pos[2] = { 0, 0, };
		std::string data = (char*)http_buffer;

		pos[0] = data.find("<newest>");
		pos[1] = data.find("</newest>");
		if(pos[0] != std::string::npos && pos[1] != std::string::npos)
		{
			data = data.substr(pos[0] + 8, pos[1] - (pos[0] + 8));
			if(DEF_MENU_CURRENT_APP_VER_INT < (uint32_t)atoi(data.c_str()))
				menu_update_available = true;
		}
	}

	free(http_buffer);
	http_buffer = NULL;

	DEF_LOG_STRING("Thread exit.");
	threadExit(0);
}
#endif
