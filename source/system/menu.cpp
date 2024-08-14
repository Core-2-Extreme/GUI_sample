#include <stdbool.h>
#include <stdint.h>
#include <string>

#include "3ds.h"

#include "system/sem.hpp"
#include "system/variables.hpp"

#include "system/util/util.hpp"

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


bool menu_thread_run = false;
bool menu_main_run = true;
bool menu_must_exit = false;
bool menu_check_exit_request = false;
bool menu_update_available = false;
bool menu_init_request[9] = { false, false, false, false, false, false, false, false, false, };
bool menu_exit_request[8] = { false, false, false, false, false, false, false, false, };
uint32_t menu_icon_texture_num[9] = { UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX, };
void (*menu_worker_thread_callbacks[DEF_MENU_NUM_OF_CALLBACKS])(void) = { NULL, };
Str_data menu_msg[DEF_MENU_NUM_OF_MSG] = { 0, };
Thread menu_worker_thread = NULL;
LightLock menu_callback_mutex = 1;//Initially unlocked state.
Draw_image_data menu_icon_image[10] = { 0, };
Draw_image_data menu_sapp_button[8] = { 0, };
Draw_image_data menu_sapp_close_button[8] = { 0, };
Draw_image_data menu_sem_button = { 0, };

#if (DEF_CURL_API_ENABLE || DEF_HTTPC_API_ENABLE)

Thread menu_check_connectivity_thread, menu_send_app_info_thread, menu_update_thread;

#endif

void Menu_get_system_info(void);
void Menu_hid_callback(void);
void Menu_worker_thread(void* arg);

#if (DEF_CURL_API_ENABLE || DEF_HTTPC_API_ENABLE)

void Menu_send_app_info_thread(void* arg);
void Menu_check_connectivity_thread(void* arg);
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
	var_need_reflesh = true;
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

void Menu_check_core_thread(void* arg)
{
	threadExit(0);
}

void Menu_init(void)
{
	bool is_800px = false;
	bool is_3d = false;
	uint8_t* data = NULL;
	uint8_t dummy = 0;
	uint8_t region = 0;
	uint8_t model = 0;
	uint32_t read_size = 0;
	uint32_t result = DEF_ERR_OTHER;
	Thread core_2 = NULL, core_3 = NULL;
	C2D_Image cache[2] = { 0, };

	for(int i = 0; i < DEF_MENU_NUM_OF_CALLBACKS; i++)
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

	if(Util_file_load_from_file("fake_model.txt", DEF_MENU_MAIN_DIR, &data, 1, 0, &read_size) == DEF_SUCCESS && *data <= 5)
	{
		var_fake_model = true;
		var_model = *data;
		free(data);
		data = NULL;
	}

	if(CFGU_SecureInfoGetRegion(&region) == 0)
	{
		if(region == CFG_REGION_CHN)
			var_system_region = 1;
		else if(region == CFG_REGION_KOR)
			var_system_region = 2;
		else if(region == CFG_REGION_TWN)
			var_system_region = 3;
		else
			var_system_region = 0;
	}

	if(CFGU_GetSystemModel(&model) == 0)
	{
		DEF_LOG_FORMAT("Model : %s", var_model_name[model].c_str());
		if(!var_fake_model)
			var_model = model;
	}
	else
		DEF_LOG_STRING("Model : Unknown");

	if(var_fake_model)
		DEF_LOG_FORMAT("Using fake model : %s", var_model_name[var_model].c_str());

	if(var_model == CFG_MODEL_2DS || var_model == CFG_MODEL_3DSXL || var_model == CFG_MODEL_3DS)
		osSetSpeedupEnable(false);

	//Init our modules.
	DEF_LOG_RESULT_SMART(result, Util_init(), (result == DEF_SUCCESS), result);
	DEF_LOG_RESULT_SMART(result, Util_watch_init(), (result == DEF_SUCCESS), result);

	Sem_init();
	Sem_suspend();

	if(var_screen_mode == DEF_SEM_SCREEN_AUTO)
	{
		if(osGet3DSliderState())
			is_3d = true;
		else
			is_800px = true;
	}
	else if(var_screen_mode == DEF_SEM_SCREEN_800PX)
		is_800px = true;
	else if(var_screen_mode == DEF_SEM_SCREEN_3D)
		is_3d = true;

	DEF_LOG_RESULT_SMART(result, Draw_init(is_800px, is_3d), (result == DEF_SUCCESS), result);

	//Init screen.
	Draw_frame_ready();
	Draw_screen_ready(DRAW_SCREEN_TOP_LEFT, DEF_DRAW_WHITE);
	Draw_top_ui();
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

	for (int i = 0; i < DEF_EXFONT_NUM_OF_FONT_NAME; i++)
		Exfont_set_external_font_request_state(i, true);

	Exfont_request_load_external_font();

	menu_thread_run = true;
	menu_worker_thread = threadCreate(Menu_worker_thread, (void*)(""), DEF_THREAD_STACKSIZE * 2, DEF_THREAD_PRIORITY_ABOVE_NORMAL, 0, false);

#if (DEF_CURL_API_ENABLE || DEF_HTTPC_API_ENABLE)
	menu_check_connectivity_thread = threadCreate(Menu_check_connectivity_thread, (void*)(""), DEF_THREAD_STACKSIZE, DEF_THREAD_PRIORITY_NORMAL, 1, false);
	menu_update_thread = threadCreate(Menu_update_thread, (void*)(""), DEF_THREAD_STACKSIZE, DEF_THREAD_PRIORITY_REALTIME, 1, true);
#endif

	//Check for core availability.
	if(var_model == CFG_MODEL_N2DSXL || var_model == CFG_MODEL_N3DSXL || var_model == CFG_MODEL_N3DS)
	{
		core_2 = threadCreate(Menu_check_core_thread, (void*)(""), DEF_THREAD_STACKSIZE, DEF_THREAD_PRIORITY_NORMAL, 2, false);
		if(!core_2)
			var_core_2_available = false;
		else
		{
			threadJoin(core_2, U64_MAX);
			var_core_2_available = true;
		}

		core_3 = threadCreate(Menu_check_core_thread, (void*)(""), DEF_THREAD_STACKSIZE, DEF_THREAD_PRIORITY_NORMAL, 3, false);
		if(!core_3)
			var_core_3_available = false;
		else
		{
			threadJoin(core_3, U64_MAX);
			var_core_3_available = true;
		}

		threadFree(core_2);
		threadFree(core_3);
	}

#if (DEF_CURL_API_ENABLE || DEF_HTTPC_API_ENABLE)
	if (var_allow_send_app_info)
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

	for(int i = 0; i < 8; i++)
	{
		menu_sapp_button[i].c2d = var_square_image[0];
		menu_sapp_close_button[i].c2d = var_square_image[0];
	}
	menu_sem_button.c2d = var_square_image[0];

	Util_watch_add(WATCH_HANDLE_MAIN_MENU, &menu_must_exit, sizeof(menu_must_exit));
	Util_watch_add(WATCH_HANDLE_MAIN_MENU, &menu_check_exit_request, sizeof(menu_check_exit_request));
	Util_watch_add(WATCH_HANDLE_MAIN_MENU, &menu_init_request[8], sizeof(menu_init_request[8]));

	Util_watch_add(WATCH_HANDLE_MAIN_MENU, &menu_sem_button.selected, sizeof(menu_sem_button.selected));
	for(int i = 0; i < 8; i++)
	{
		Util_watch_add(WATCH_HANDLE_MAIN_MENU, &menu_init_request[i], sizeof(menu_init_request[i]));
		Util_watch_add(WATCH_HANDLE_MAIN_MENU, &menu_exit_request[i], sizeof(menu_exit_request[i]));

		Util_watch_add(WATCH_HANDLE_MAIN_MENU, &menu_sapp_button[i].selected, sizeof(menu_sapp_button[i].selected));
		Util_watch_add(WATCH_HANDLE_MAIN_MENU, &menu_sapp_close_button[i].selected, sizeof(menu_sapp_close_button[i].selected));
	}

	Menu_get_system_info();

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

	for(int i = 0; i < 8; i++)
		Draw_free_texture(menu_icon_texture_num[i]);

	Util_hid_remove_callback(Menu_hid_callback);
	Util_hid_exit();
	Util_expl_exit();
	Exfont_exit();
	Util_err_exit();
	Util_exit();
	Util_cpu_usage_monitor_exit();

	DEF_LOG_RESULT_SMART(result, threadJoin(menu_worker_thread, DEF_THREAD_WAIT_TIME), (result == DEF_SUCCESS), result);
	threadFree(menu_worker_thread);

#if (DEF_CURL_API_ENABLE || DEF_HTTPC_API_ENABLE)
	DEF_LOG_RESULT_SMART(result, threadJoin(menu_check_connectivity_thread, DEF_THREAD_WAIT_TIME), (result == DEF_SUCCESS), result);
	DEF_LOG_RESULT_SMART(result, threadJoin(menu_send_app_info_thread, DEF_THREAD_WAIT_TIME), (result == DEF_SUCCESS), result);
	DEF_LOG_RESULT_SMART(result, threadJoin(menu_update_thread, DEF_THREAD_WAIT_TIME), (result == DEF_SUCCESS), result);
	threadFree(menu_check_connectivity_thread);
#endif

	Util_watch_remove(WATCH_HANDLE_MAIN_MENU, &menu_must_exit);
	Util_watch_remove(WATCH_HANDLE_MAIN_MENU, &menu_check_exit_request);
	Util_watch_remove(WATCH_HANDLE_MAIN_MENU, &menu_init_request[8]);

	Util_watch_remove(WATCH_HANDLE_MAIN_MENU, &menu_sem_button.selected);
	for(int i = 0; i < 8; i++)
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

	for(int i = 0; i < DEF_MENU_NUM_OF_CALLBACKS; i++)
	{
		if(menu_worker_thread_callbacks[i] == callback)
			goto success;//Already exist.
	}

	for(int i = 0; i < DEF_MENU_NUM_OF_CALLBACKS; i++)
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

	for(int i = 0; i < DEF_MENU_NUM_OF_CALLBACKS; i++)
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
	uint8_t screen_mode = var_screen_mode;
	int color = DEF_DRAW_BLACK;
	int back_color = DEF_DRAW_WHITE;

	is_800px = (screen_mode == DEF_SEM_SCREEN_800PX);
	is_3d = (screen_mode == DEF_SEM_SCREEN_3D);
	if(screen_mode == DEF_SEM_SCREEN_AUTO)
	{
		if(osGet3DSliderState())
		{
			is_3d = true;
			is_800px = false;
		}
		else
		{
			is_3d = false;
			is_800px = true;
		}
	}

	if(var_model == CFG_MODEL_2DS && is_800px)
	{
		is_800px = false;
		var_screen_mode = DEF_SEM_SCREEN_AUTO;
	}
	if((var_model == CFG_MODEL_2DS || var_model == CFG_MODEL_N2DSXL) && is_3d)
	{
		is_3d = false;
		var_screen_mode = DEF_SEM_SCREEN_AUTO;
	}

	//Update screen mode here.
	if(is_3d != Draw_is_3d_mode() || is_800px != Draw_is_800px_mode())
	{
		uint32_t result = DEF_ERR_OTHER;

		DEF_LOG_RESULT_SMART(result, Draw_reinit(is_800px, is_3d), (result == DEF_SUCCESS), result);
		var_need_reflesh = true;
	}

	if(var_debug_mode)
		var_need_reflesh = true;

	if (menu_main_run)
	{
		Watch_handle_bit watch_handle_bit = (DEF_WATCH_HANDLE_BIT_GLOBAL | DEF_WATCH_HANDLE_BIT_MAIN_MENU);

		//Check if we should update the screen.
		if(Util_watch_is_changed(watch_handle_bit) || var_need_reflesh || !var_eco_mode)
		{
			var_need_reflesh = false;
			if (var_night_mode)
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

			Draw_top_ui();

			if(var_monitor_cpu_usage)
				Draw_cpu_usage_info();

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
			Draw_texture(&menu_icon_image[8 + var_night_mode], DEF_DRAW_NO_COLOR, 260, 170, 60, 60);
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
		if(menu_init_request[8])
		{
			Sem_init();
			menu_init_request[8] = false;
		}
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
					{
						if (!Sem_query_init_flag())
						{
							menu_init_request[8] = true;
							while(menu_init_request[8])
								Util_sleep(20000);
						}
						else
							Sem_resume();
					}
				}

				if(!key.p_touch && !key.h_touch)
				{
					for(int i = 0; i < 8; i++)
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

void Menu_get_system_info(void)
{
	uint8_t battery_level = 0;
	uint8_t battery_voltage = 0;
	uint8_t battery_temp = 0;
	uint32_t result = DEF_ERR_OTHER;
	char* ssid = (char*)malloc(512);

	PTMU_GetBatteryChargeState(&var_battery_charge);//Battery charge.
	result = MCUHWC_GetBatteryLevel(&battery_level);//Battery level(%).
	if(result == DEF_SUCCESS)
	{
		MCUHWC_GetBatteryVoltage(&battery_voltage);
		MCUHWC_ReadRegister(0x0A, &battery_temp, 1);
		var_battery_voltage = 5.0 * (battery_voltage / 256.0);
		var_battery_level_raw = battery_level;
		var_battery_temp = battery_temp;
	}
	else
	{
		PTMU_GetBatteryLevel(&battery_level);
		if ((int)battery_level == 0)
			var_battery_level_raw = 0;
		else if ((int)battery_level == 1)
			var_battery_level_raw = 5;
		else if ((int)battery_level == 2)
			var_battery_level_raw = 10;
		else if ((int)battery_level == 3)
			var_battery_level_raw = 30;
		else if ((int)battery_level == 4)
			var_battery_level_raw = 60;
		else if ((int)battery_level == 5)
			var_battery_level_raw = 100;
	}

	//Connected SSID.
	result = ACU_GetSSID(ssid);
	if(result == DEF_SUCCESS)
		var_connected_ssid = ssid;
	else
		var_connected_ssid = "";

	free(ssid);
	ssid = NULL;

	var_wifi_signal = osGetWifiStrength();
	//Get wifi state from shared memory #0x1FF81067.
	var_wifi_state = *(uint8_t*)0x1FF81067;
	if(var_wifi_state == 2)
	{
#if (DEF_CURL_API_ENABLE || DEF_HTTPC_API_ENABLE)
		if (!var_connect_test_succes)
			var_wifi_signal += 4;
#endif
	}
	else
	{
		var_wifi_signal = 8;
		var_connect_test_succes = false;
	}

	//Get time.
	time_t unixTime = time(NULL);
	struct tm* timeStruct = gmtime((const time_t*)&unixTime);
	var_years = timeStruct->tm_year + 1900;
	var_months = timeStruct->tm_mon + 1;
	var_days = timeStruct->tm_mday;
	var_hours = timeStruct->tm_hour;
	var_minutes = timeStruct->tm_min;
	var_seconds = timeStruct->tm_sec;

	if (var_debug_mode)
	{
		//Check for free RAM.
		var_free_ram = Util_check_free_ram();
		var_free_linear_ram = Util_check_free_linear_space();
	}

	sprintf(var_status, "%02dfps %04d/%02d/%02d %02d:%02d:%02d ", (int)Draw_query_fps(), var_years, var_months, var_days, var_hours, var_minutes, var_seconds);
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

	osGetSystemVersionDataString(&os_ver, &os_ver, system_ver_char, 0x50);
	std::string system_ver = system_ver_char;

	APT_CheckNew3DS(&is_new3ds);
	new3ds = is_new3ds ? "yes" : "no";

	std::string send_data = (std::string)"{ \"app_ver\": \"" + DEF_MENU_CURRENT_APP_VER + "\",\"system_ver\" : \"" + system_ver + "\",\"start_num_of_app\" : \"" + std::to_string(var_num_of_app_start) + "\",\"language\" : \"" + var_lang + "\",\"new3ds\" : \"" + new3ds + "\",\"time_to_enter_sleep\" : \"" + std::to_string(var_time_to_turn_off_lcd) + "\",\"scroll_speed\" : \"" + std::to_string(var_scroll_speed) + "\" }";

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

#if (DEF_CURL_API_ENABLE || DEF_HTTPC_API_ENABLE)
void Menu_check_connectivity_thread(void* arg)
{
	DEF_LOG_STRING("Thread started.");
	uint8_t* http_buffer = NULL;
	uint16_t status_code = 0;
	uint16_t count = 100;

	while (menu_thread_run)
	{
		if (count >= 100)
		{
			count = 0;
#if DEF_HTTPC_API_ENABLE//Curl uses more CPU so prefer to use httpc module here.
			Util_httpc_dl_data(DEF_MENU_CHECK_INTERNET_URL, &http_buffer, 0x1000, NULL, &status_code, 0, NULL);
#else
			Util_curl_dl_data(DEF_MENU_CHECK_INTERNET_URL, &http_buffer, 0x1000, NULL, &status_code, 0, NULL);
#endif
			free(http_buffer);
			http_buffer = NULL;

			if (status_code == 204)
				var_connect_test_succes = true;
			else
				var_connect_test_succes = false;
		}
		else
			Util_sleep(DEF_THREAD_ACTIVE_SLEEP_TIME);

		count++;
	}

	DEF_LOG_STRING("Thread exit.");
	threadExit(0);
}
#endif

void Menu_worker_thread(void* arg)
{
	DEF_LOG_STRING("Thread started.");
	uint32_t count = 0;
	uint32_t result = DEF_ERR_OTHER;
	uint64_t previous_ts = 0;

	while (menu_thread_run)
	{
		if(previous_ts + 50 <= osGetTime())
		{
			if(var_flash_mode)
			{
				var_night_mode = !var_night_mode;
				var_need_reflesh = true;
			}
			count++;

			if(previous_ts + 100 >= osGetTime())
				previous_ts += 50;
			else
				previous_ts = osGetTime();
		}

		if(count % 5 == 0)
			Menu_get_system_info();

		if(count >= 20)
		{
			var_need_reflesh = true;
			var_afk_time++;
			count = 0;
		}

		//If var_time_to_turn_off_lcd < 0, it means turn off LCD settings has been disabled.
		if(var_time_to_turn_off_lcd > 0 && var_afk_time > var_time_to_turn_off_lcd)
		{
			result = Util_hw_config_set_screen_state(true, true, false);
			if(result != DEF_SUCCESS)
				DEF_LOG_RESULT(Util_hw_config_set_screen_state, false, result);
		}
		else if(var_time_to_turn_off_lcd > 0 &&var_afk_time > (var_time_to_turn_off_lcd - 10))
		{
			result = Util_hw_config_set_screen_brightness(true, true, 10);
			if(result != DEF_SUCCESS)
				DEF_LOG_RESULT(Util_hw_config_set_screen_brightness, false, result);
		}
		else
		{
			result = Util_hw_config_set_screen_state(true, false, var_turn_on_top_lcd);
			if(result != DEF_SUCCESS)
				DEF_LOG_RESULT(Util_hw_config_set_screen_state, false, result);

			result = Util_hw_config_set_screen_state(false, true, var_turn_on_bottom_lcd);
			if(result != DEF_SUCCESS)
				DEF_LOG_RESULT(Util_hw_config_set_screen_state, false, result);

			if(var_top_lcd_brightness == var_lcd_brightness && var_bottom_lcd_brightness == var_lcd_brightness)
			{
				result = Util_hw_config_set_screen_brightness(true, true, var_lcd_brightness);
				if(result != DEF_SUCCESS)
					DEF_LOG_RESULT(Util_hw_config_set_screen_brightness, false, result);
			}
			else
			{
				result = Util_hw_config_set_screen_brightness(true, false, var_top_lcd_brightness);
				if(result != DEF_SUCCESS)
					DEF_LOG_RESULT(Util_hw_config_set_screen_brightness, false, result);

				result = Util_hw_config_set_screen_brightness(false, true, var_bottom_lcd_brightness);
				if(result != DEF_SUCCESS)
					DEF_LOG_RESULT(Util_hw_config_set_screen_brightness, false, result);
			}
		}

		if(var_time_to_enter_sleep > 0 && var_afk_time > var_time_to_enter_sleep)
		{
			result = Util_hw_config_sleep_system((HW_CONFIG_WAKEUP_BIT_OPEN_SHELL | HW_CONFIG_WAKEUP_BIT_PRESS_HOME_BUTTON));
			if(result == DEF_SUCCESS)
			{
				//We woke up from sleep.
				var_afk_time = 0;
			}
			else
				DEF_LOG_RESULT(Util_hw_config_sleep_system, false, result);
		}

		LightLock_Lock(&menu_callback_mutex);

		//Call callback functions.
		for(int i = 0; i < DEF_MENU_NUM_OF_CALLBACKS; i++)
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
