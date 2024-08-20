#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <string>

#include "3ds.h"

extern "C"
{
#include "system/menu.h"
#include "system/draw/draw.h"
#include "system/draw/exfont.h"
#include "system/util/converter.h"
#include "system/util/cpu_usage.h"
#include "system/util/curl.h"
#include "system/util/encoder.h"
#include "system/util/err.h"
#include "system/util/file.h"
#include "system/util/hid.h"
#include "system/util/httpc.h"
#include "system/util/hw_config.h"
#include "system/util/log.h"
#include "system/util/str.h"
#include "system/util/thread_types.h"
#include "system/util/util.h"
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
extern "C"
{
#include "system/sem.h"
}


typedef struct
{
	bool is_connect_test_succes;
	uint8_t wifi_state;
	Sem_model fake_model;
} Sem_internal_state;


bool sem_main_run = false;
bool sem_already_init = false;
bool sem_thread_run = false;
bool sem_thread_suspend = false;
bool sem_reload_msg_request = false;
bool sem_scroll_mode = false;
bool sem_dump_log_request = false;
bool sem_should_wifi_enabled = false;
int8_t sem_selected_menu_mode = DEF_SEM_MENU_TOP;
double sem_y_offset = 0.0;
double sem_y_max = 0.0;
double sem_touch_x_move_left = 0.0;
double sem_touch_y_move_left = 0.0;
const char* sem_model_name[6] = { "OLD 3DS", "OLD 3DS XL", "OLD 2DS", "NEW 3DS", "NEW 3DS XL", "NEW 2DS XL", };
Str_data sem_msg[DEF_SEM_NUM_OF_MSG] = { 0, };
std::string sem_newest_ver_data[6];//0 newest version number, 1 3dsx available, 2 cia available, 3 3dsx dl url, 4 cia dl url, 5 patch note
LightLock sem_config_state_mutex = 1;//Initially unlocked state.
Thread sem_hw_config_thread = NULL;
Draw_image_data sem_back_button = { 0, }, sem_scroll_bar = { 0, }, sem_menu_button[9] = { 0, }, sem_english_button = { 0, },
sem_japanese_button = { 0, }, sem_hungarian_button = { 0, }, sem_chinese_button = { 0, }, sem_italian_button = { 0, },
sem_spanish_button = { 0, }, sem_romanian_button = { 0, }, sem_polish_button = { 0, }, sem_ryukyuan_button = { 0, },
sem_night_mode_on_button = { 0, }, sem_night_mode_off_button = { 0, }, sem_flash_mode_button = { 0, }, sem_screen_brightness_slider = { 0, },
sem_screen_brightness_bar = { 0, }, sem_screen_off_time_slider = { 0, }, sem_screen_off_time_bar = { 0, }, sem_sleep_time_slider = { 0, },
sem_sleep_time_bar = { 0, }, sem_800px_mode_button = { 0, }, sem_3d_mode_button = { 0, }, sem_400px_mode_button = { 0, },
sem_auto_mode_button = { 0, }, sem_scroll_speed_slider = { 0, }, sem_scroll_speed_bar = { 0, }, sem_load_all_ex_font_button = { 0, },
sem_unload_all_ex_font_button = { 0, }, sem_ex_font_button[DEF_EXFONT_NUM_OF_FONT_NAME] = { 0, }, sem_wifi_on_button = { 0, },
sem_wifi_off_button = { 0, }, sem_allow_send_info_button = { 0, }, sem_deny_send_info_button = { 0, }, sem_debug_mode_on_button = { 0, },
sem_debug_mode_off_button = { 0, }, sem_eco_mode_on_button = { 0, }, sem_eco_mode_off_button = { 0, }, sem_record_both_lcd_button = { 0, },
sem_record_top_lcd_button = { 0, }, sem_record_bottom_lcd_button = { 0, }, sem_use_fake_model_button = { 0, }, sem_dump_log_button = { 0, };
Sem_internal_state sem_internal_state = { 0, };
Sem_state sem_state = { 0, };
Sem_config sem_config = { 0, };

#if DEF_CPU_USAGE_API_ENABLE
bool sem_is_cpu_usage_monitor_running = false;
bool sem_should_cpu_usage_monitor_running = false;
#endif

#if (DEF_CURL_API_ENABLE || DEF_HTTPC_API_ENABLE)

Thread sem_check_connectivity_thread = NULL;

#endif

#if ((DEF_CURL_API_ENABLE || DEF_HTTPC_API_ENABLE) && DEF_SEM_ENABLE_UPDATER)

bool sem_check_update_request = false;
bool sem_new_version_available = false;
bool sem_show_patch_note_request = false;
bool sem_select_ver_request = false;
bool sem_dl_file_request = false;
int8_t sem_update_progress = -1;
int8_t sem_selected_edition_num = DEF_SEM_EDTION_NONE;
uint32_t sem_installed_size = 0;
uint32_t sem_total_cia_size = 0;
uint32_t sem_dled_size = 0;
Thread sem_update_thread = NULL;
Draw_image_data sem_check_update_button = { 0, }, sem_select_edtion_button = { 0, }, sem_close_updater_button = { 0, },
sem_3dsx_button = { 0, }, sem_cia_button = { 0, }, sem_dl_install_button = { 0, }, sem_back_to_patch_note_button = { 0, },
sem_close_app_button = { 0, };

#endif

#if DEF_CPU_USAGE_API_ENABLE

Draw_image_data sem_monitor_cpu_usage_on_button = { 0, }, sem_monitor_cpu_usage_off_button = { 0, };

#endif

#if (DEF_ENCODER_VIDEO_AUDIO_API_ENABLE && DEF_CONVERTER_SW_API_ENABLE && DEF_SEM_ENABLE_SCREEN_RECORDER)

bool sem_record_request = false;
bool sem_encode_request = false;
bool sem_wait_request = false;
bool sem_stop_record_request = false;
uint8_t sem_selected_recording_mode = 0;
uint8_t* sem_yuv420p = NULL;
uint16_t sem_rec_width = 400;
uint16_t sem_rec_height = 480;
Thread sem_record_thread, sem_encode_thread;

#endif


static void Sem_get_system_info(void);

#if (DEF_ENCODER_VIDEO_AUDIO_API_ENABLE && DEF_CONVERTER_SW_API_ENABLE && DEF_SEM_ENABLE_SCREEN_RECORDER)

void Sem_encode_thread(void* arg);
void Sem_record_thread(void* arg);

#endif

void Sem_worker_callback(void);
void Sem_hw_config_thread(void* arg);

#if (DEF_CURL_API_ENABLE || DEF_HTTPC_API_ENABLE)

void Sem_check_connectivity_thread(void* arg);

#endif

#if ((DEF_CURL_API_ENABLE || DEF_HTTPC_API_ENABLE) && DEF_SEM_ENABLE_UPDATER)

void Sem_update_thread(void* arg);

#endif


bool Sem_query_init_flag(void)
{
	return sem_already_init;
}

bool Sem_query_running_flag(void)
{
	return sem_main_run;
}

void Sem_resume(void)
{
	sem_thread_suspend = false;
	sem_main_run = true;
	Draw_set_refresh_needed(true);
	Menu_suspend();
}

void Sem_suspend(void)
{
	Menu_resume();
	sem_thread_suspend = true;
	sem_main_run = false;
}

uint32_t Sem_load_msg(const char* lang)
{
	char file_name[32] = { 0, };

	snprintf(file_name, sizeof(file_name), "sem_%s.txt", (lang ? lang : ""));
	return Util_load_msg(file_name, sem_msg, DEF_SEM_NUM_OF_MSG);
}

void Sem_get_config(Sem_config* config)
{
	if(!sem_already_init)
		return;

	if(!config)
		return;

	LightLock_Lock(&sem_config_state_mutex);
	memcpy(config, &sem_config, sizeof(Sem_config));
	LightLock_Unlock(&sem_config_state_mutex);
}

void Sem_set_config(Sem_config* new_config)
{
	bool reload_msg = false;
	bool set_wifi_state = false;

	if(!sem_already_init)
		return;

	if(!new_config)
		return;

	LightLock_Lock(&sem_config_state_mutex);

	// if(sem_config.is_debug != new_config->is_debug)
		//Do nothing.
	// if(sem_config.is_send_info_allowed != new_config->is_send_info_allowed)
		//Do nothing.
	// if(sem_config.is_night != new_config->is_night)
		//Do nothing.
	// if(sem_config.is_flash != new_config->is_flash)
		//Do nothing, Sem_hw_config_thread() will do the job later.
	// if(sem_config.is_eco != new_config->is_eco)
		//Do nothing.
	if(sem_config.is_wifi_on != new_config->is_wifi_on)
	{
		//Send a request.
		sem_should_wifi_enabled = new_config->is_wifi_on;
		set_wifi_state = true;
		//Don't update config yet, Sem_hw_config_thread() will do the job later.
		new_config->is_wifi_on = sem_config.is_wifi_on;
	}
	// if(sem_config.is_top_lcd_on != new_config->is_top_lcd_on)
		//Do nothing, Sem_hw_config_thread() will do the job later.
	// if(sem_config.is_bottom_lcd_on != new_config->is_bottom_lcd_on)
		//Do nothing, Sem_hw_config_thread() will do the job later.
	if(sem_config.top_lcd_brightness != new_config->top_lcd_brightness)
	{
		//Do nothing (just validate the value), Sem_hw_config_thread() will do the job later.
		if(new_config->top_lcd_brightness > 180)
			new_config->top_lcd_brightness = 180;
	}
	if(sem_config.bottom_lcd_brightness != new_config->bottom_lcd_brightness)
	{
		//Do nothing (just validate the value), Sem_hw_config_thread() will do the job later.
		if(new_config->bottom_lcd_brightness > 180)
			new_config->bottom_lcd_brightness = 180;
	}
	if(sem_config.time_to_turn_off_lcd != new_config->time_to_turn_off_lcd)
	{
		//Do nothing (just validate the value), Sem_hw_config_thread() will do the job later.
		if(new_config->time_to_turn_off_lcd != 0 && (new_config->time_to_turn_off_lcd < 20 || new_config->time_to_turn_off_lcd > 600))
			new_config->time_to_turn_off_lcd = 150;
	}
	if(sem_config.time_to_enter_sleep != new_config->time_to_enter_sleep)
	{
		//Do nothing (just validate the value), Sem_hw_config_thread() will do the job later.
		if(new_config->time_to_enter_sleep != 0 && (new_config->time_to_enter_sleep < 20 || new_config->time_to_enter_sleep > 600))
			new_config->time_to_enter_sleep = 175;
	}
	// if(sem_config.scroll_speed != new_config->scroll_speed)
		//Do nothing.
	if(strncmp(sem_config.lang, new_config->lang, sizeof(sem_config.lang)) != 0)
	{
		//Validate the value and send a request if the value is valid.
		//Sem_worker_callback() will do the job.
		if(strcmp(new_config->lang, "jp") != 0 && strcmp(new_config->lang, "en") != 0
		&& strcmp(new_config->lang, "hu") != 0 && strcmp(new_config->lang, "zh-cn") != 0
		&& strcmp(new_config->lang, "it") != 0 && strcmp(new_config->lang, "es") != 0
		&& strcmp(new_config->lang, "ro") != 0 && strcmp(new_config->lang, "pl") != 0
		&& strcmp(new_config->lang, "ryu") != 0)
			snprintf(new_config->lang, sizeof(new_config->lang), "en");
		else
			reload_msg = true;
	}

	if(new_config->time_to_turn_off_lcd > new_config->time_to_enter_sleep)
		new_config->time_to_enter_sleep = new_config->time_to_turn_off_lcd;

	memcpy(&sem_config, new_config, sizeof(Sem_config));
	if(set_wifi_state)
		sem_should_wifi_enabled = true;
	if(reload_msg)
		sem_reload_msg_request = true;

	LightLock_Unlock(&sem_config_state_mutex);
}

void Sem_get_state(Sem_state* state)
{
	if(!sem_already_init)
		return;

	if(!state)
		return;

	LightLock_Lock(&sem_config_state_mutex);
	memcpy(state, &sem_state, sizeof(Sem_state));
	LightLock_Unlock(&sem_config_state_mutex);
}

void Sem_init(void)
{
	DEF_LOG_STRING("Initializing...");
	bool wifi_state = true;
	uint8_t model = 0;
	uint8_t* read_cache = NULL;
	uint32_t read_size = 0;
	uint32_t result = DEF_ERR_OTHER;
	Str_data data[13] = { 0, };
	Sem_config config = { 0, };
	Sem_state state = { 0, };

	//Default values.
	config.is_debug = false;
	config.is_send_info_allowed = false;
	config.is_night = false;
	config.is_flash = false;
	config.is_eco = true;
	config.is_wifi_on = true;
	config.is_top_lcd_on = true;
	config.is_bottom_lcd_on = true;
	config.top_lcd_brightness = 100;
	config.bottom_lcd_brightness = 100;
	config.time_to_turn_off_lcd = 150;
	config.time_to_enter_sleep = 175;
	config.scroll_speed = 0.5;
	config.screen_mode = DEF_SEM_SCREEN_MODE_AUTO;
	memset(config.lang, 0x00, sizeof(config.lang));

	state.is_charging = false;
	state.battery_level = 0;
	state.battery_temp = 0;
	state.free_ram = 0;
	state.free_linear_ram = 0;
	state.battery_voltage = 0;
	state.num_of_launch = 0;
	state.wifi_signal = DEF_SEM_WIFI_SIGNAL_DISABLED;
	state.console_model = DEF_SEM_MODEL_OLD3DS;
	state.time = { .years = 1990, .months = 1, .days = 1, .hours = 0, .minutes = 0, .seconds = 0, };
	memset(state.connected_wifi, 0x00, sizeof(state.connected_wifi));
	memset(state.msg, 0x00, sizeof(state.msg));

	LightLock_Init(&sem_config_state_mutex);

	if(CFGU_GetSystemModel(&model) == DEF_SUCCESS)
	{
		if(model == CFG_MODEL_3DS)
			state.console_model = DEF_SEM_MODEL_OLD3DS;
		else if(model == CFG_MODEL_3DSXL)
			state.console_model = DEF_SEM_MODEL_OLD3DSXL;
		else if(model == CFG_MODEL_2DS)
			state.console_model = DEF_SEM_MODEL_OLD2DS;
		else if(model == CFG_MODEL_N3DS)
			state.console_model = DEF_SEM_MODEL_NEW3DS;
		else if(model == CFG_MODEL_N3DSXL)
			state.console_model = DEF_SEM_MODEL_NEW3DSXL;
		else if(model == CFG_MODEL_N2DSXL)
			state.console_model = DEF_SEM_MODEL_NEW2DSXL;

		DEF_LOG_FORMAT("Model : %s", sem_model_name[state.console_model]);
	}
	else
		DEF_LOG_STRING("Model : Unknown");

	if(Util_file_load_from_file("fake_model.txt", DEF_MENU_MAIN_DIR, &read_cache, 1, 0, &read_size) == DEF_SUCCESS && *read_cache < DEF_SEM_MODEL_MAX)
	{
		//Fake model config exists.
		state.console_model = *read_cache;
		sem_internal_state.fake_model = state.console_model;

		DEF_LOG_FORMAT("Using fake model : %s", sem_model_name[state.console_model]);

		free(read_cache);
		read_cache = NULL;
	}
	else
		sem_internal_state.fake_model = 255;

	if(!DEF_SEM_MODEL_IS_NEW(state.console_model))
		osSetSpeedupEnable(false);

	DEF_LOG_RESULT_SMART(result, Util_file_load_from_file("settings.txt", DEF_MENU_MAIN_DIR, &read_cache, 0x1000, 0, &read_size), (result == DEF_SUCCESS), result)
	if (result == DEF_SUCCESS)
	{
		DEF_LOG_RESULT_SMART(result, Util_parse_file((char*)read_cache, 13, data), (result == DEF_SUCCESS), result);

		//If this fails, the settings file may come from older version.
		//To keep backward compatibility, try to parse with less elements.
		if(result != DEF_SUCCESS)
		{
			DEF_LOG_RESULT_SMART(result, Util_parse_file((char*)read_cache, 12, data), (result == DEF_SUCCESS), result);
			Util_str_set(&data[12], "175");//Time to turn off LCDs default value.
		}
		if(result != DEF_SUCCESS)
		{
			DEF_LOG_RESULT_SMART(result, Util_parse_file((char*)read_cache, 11, data), (result == DEF_SUCCESS), result);
			Util_str_set(&data[11], "175");//Screen mode default value.
			Util_str_set(&data[12], "175");//Time to turn off LCDs default value.
		}

		if(result == DEF_SUCCESS)
		{
			uint8_t brightness = 0;
			uint8_t length = Util_min(data[0].length, (sizeof(config.lang) - 1));

			memcpy(config.lang, data[0].buffer, length);
			brightness = atoi(data[1].buffer);
			config.time_to_turn_off_lcd = atoi(data[2].buffer);
			config.scroll_speed = strtod(data[3].buffer, NULL);
			config.is_send_info_allowed = (data[4].buffer[0] == '1');
			state.num_of_launch = atoi(data[5].buffer);
			config.is_night = (data[6].buffer[0] == '1');
			config.is_eco = (data[7].buffer[0] == '1');
			wifi_state = (data[8].buffer[0] == '1');
			//9 and 10 is no longer used.
			config.screen_mode = atoi(data[11].buffer);
			config.time_to_enter_sleep = atoi(data[12].buffer);

			if(strcmp(config.lang, "jp") != 0 && strcmp(config.lang, "en") != 0
			&& strcmp(config.lang, "hu") != 0 && strcmp(config.lang, "zh-cn") != 0
			&& strcmp(config.lang, "it") != 0 && strcmp(config.lang, "es") != 0
			&& strcmp(config.lang, "ro") != 0 && strcmp(config.lang, "pl") != 0
			&& strcmp(config.lang, "ryu") != 0)
				snprintf(config.lang, sizeof(config.lang), "en");

			if(brightness > 180)
				brightness = 100;

			config.top_lcd_brightness = brightness;
			config.bottom_lcd_brightness = brightness;

			if(config.time_to_turn_off_lcd != 0 && (config.time_to_turn_off_lcd < 20 || config.time_to_turn_off_lcd > 600))
				config.time_to_turn_off_lcd = 150;

			if(config.scroll_speed < 0.05 || config.scroll_speed > 2)
				config.scroll_speed = 0.5;

			if(config.screen_mode > DEF_SEM_SCREEN_MODE_3D)
				config.screen_mode = DEF_SEM_SCREEN_MODE_AUTO;

			if(config.time_to_enter_sleep != 0 && (config.time_to_enter_sleep < 20 || config.time_to_enter_sleep > 600))
				config.time_to_enter_sleep = 175;

			if(config.time_to_turn_off_lcd > config.time_to_enter_sleep)
				config.time_to_enter_sleep = config.time_to_turn_off_lcd;

			sem_config = config;
		}
		free(read_cache);
		read_cache = NULL;
	}

	config = sem_config;

	if(state.console_model == DEF_SEM_MODEL_OLD2DS && config.screen_mode == DEF_SEM_SCREEN_MODE_800PX)//OLD 2DS doesn't support high resolution mode.
		config.screen_mode = DEF_SEM_SCREEN_MODE_AUTO;

	if((state.console_model == DEF_SEM_MODEL_OLD2DS || state.console_model == DEF_SEM_MODEL_NEW2DSXL)
	&& config.screen_mode == DEF_SEM_SCREEN_MODE_3D)//2DSs don't support 3d mode.
		config.screen_mode = DEF_SEM_SCREEN_MODE_AUTO;

	result = Util_hw_config_set_wifi_state(wifi_state);
	if(result == DEF_SUCCESS || result == 0xC8A06C0D)
		config.is_wifi_on = wifi_state;

	sem_should_wifi_enabled = config.is_wifi_on;
	sem_config = config;
	sem_state = state;

	sem_thread_run = true;
	sem_hw_config_thread = threadCreate(Sem_hw_config_thread, (void*)(""), DEF_THREAD_STACKSIZE, DEF_THREAD_PRIORITY_HIGH, 1, false);
#if (DEF_CURL_API_ENABLE || DEF_HTTPC_API_ENABLE)
	sem_check_connectivity_thread = threadCreate(Sem_check_connectivity_thread, (void*)(""), DEF_THREAD_STACKSIZE, DEF_THREAD_PRIORITY_NORMAL, 1, false);
#endif

#if ((DEF_CURL_API_ENABLE || DEF_HTTPC_API_ENABLE) && DEF_SEM_ENABLE_UPDATER)
	sem_update_thread = threadCreate(Sem_update_thread, (void*)(""), DEF_THREAD_STACKSIZE, DEF_THREAD_PRIORITY_NORMAL, 0, false);
#endif

#if (DEF_ENCODER_VIDEO_AUDIO_API_ENABLE && DEF_CONVERTER_SW_API_ENABLE && DEF_SEM_ENABLE_SCREEN_RECORDER)
	sem_record_thread = threadCreate(Sem_record_thread, (void*)(""), DEF_THREAD_STACKSIZE, DEF_THREAD_PRIORITY_HIGH, 0, false);
	if(DEF_SEM_MODEL_IS_NEW(state.console_model))
		sem_encode_thread = threadCreate(Sem_encode_thread, (void*)(""), DEF_THREAD_STACKSIZE, DEF_THREAD_PRIORITY_HIGH, 2, false);
	else
		sem_encode_thread = threadCreate(Sem_encode_thread, (void*)(""), DEF_THREAD_STACKSIZE, DEF_THREAD_PRIORITY_HIGH, 1, false);
#endif

	sem_reload_msg_request = true;

	//Global.
	Util_watch_add(WATCH_HANDLE_SETTINGS_MENU, &sem_y_offset, sizeof(sem_y_offset));
	Util_watch_add(WATCH_HANDLE_SETTINGS_MENU, &sem_y_max, sizeof(sem_y_max));
	Util_watch_add(WATCH_HANDLE_SETTINGS_MENU, &sem_selected_menu_mode, sizeof(sem_selected_menu_mode));
	Util_watch_add(WATCH_HANDLE_SETTINGS_MENU, &sem_scroll_mode, sizeof(sem_scroll_mode));
	Util_watch_add(WATCH_HANDLE_SETTINGS_MENU, &sem_scroll_bar.selected, sizeof(sem_scroll_bar.selected));

	Util_watch_add(WATCH_HANDLE_SETTINGS_MENU, &sem_back_button.selected, sizeof(sem_back_button.selected));
	for(uint8_t i = 0; i < 9; i++)
		Util_watch_add(WATCH_HANDLE_SETTINGS_MENU, &sem_menu_button[i].selected, sizeof(sem_menu_button[i].selected));


#if ((DEF_CURL_API_ENABLE || DEF_HTTPC_API_ENABLE) && DEF_SEM_ENABLE_UPDATER)
	//Updater.
	Util_watch_add(WATCH_HANDLE_SETTINGS_MENU, &sem_show_patch_note_request, sizeof(sem_show_patch_note_request));
	Util_watch_add(WATCH_HANDLE_SETTINGS_MENU, &sem_select_ver_request, sizeof(sem_select_ver_request));
	Util_watch_add(WATCH_HANDLE_SETTINGS_MENU, &sem_dl_file_request, sizeof(sem_dl_file_request));
	Util_watch_add(WATCH_HANDLE_SETTINGS_MENU, &sem_check_update_request, sizeof(sem_check_update_request));
	Util_watch_add(WATCH_HANDLE_SETTINGS_MENU, &sem_selected_edition_num, sizeof(sem_selected_edition_num));
	Util_watch_add(WATCH_HANDLE_SETTINGS_MENU, &sem_installed_size, sizeof(sem_installed_size));
	Util_watch_add(WATCH_HANDLE_SETTINGS_MENU, &sem_dled_size, sizeof(sem_dled_size));

	Util_watch_add(WATCH_HANDLE_SETTINGS_MENU, &sem_close_updater_button.selected, sizeof(sem_close_updater_button.selected));
	Util_watch_add(WATCH_HANDLE_SETTINGS_MENU, &sem_select_edtion_button.selected, sizeof(sem_select_edtion_button.selected));
	Util_watch_add(WATCH_HANDLE_SETTINGS_MENU, &sem_3dsx_button.selected, sizeof(sem_3dsx_button.selected));
	Util_watch_add(WATCH_HANDLE_SETTINGS_MENU, &sem_cia_button.selected, sizeof(sem_cia_button.selected));
	Util_watch_add(WATCH_HANDLE_SETTINGS_MENU, &sem_back_to_patch_note_button.selected, sizeof(sem_back_to_patch_note_button.selected));
	Util_watch_add(WATCH_HANDLE_SETTINGS_MENU, &sem_dl_install_button.selected, sizeof(sem_dl_install_button.selected));
	Util_watch_add(WATCH_HANDLE_SETTINGS_MENU, &sem_close_app_button.selected, sizeof(sem_close_app_button.selected));
	Util_watch_add(WATCH_HANDLE_SETTINGS_MENU, &sem_check_update_button.selected, sizeof(sem_check_update_button.selected));
#endif

	//Languages.
	Util_watch_add(WATCH_HANDLE_SETTINGS_MENU, &sem_reload_msg_request, sizeof(sem_reload_msg_request));

	Util_watch_add(WATCH_HANDLE_SETTINGS_MENU, &sem_english_button.selected, sizeof(sem_english_button.selected));
	Util_watch_add(WATCH_HANDLE_SETTINGS_MENU, &sem_japanese_button.selected, sizeof(sem_japanese_button.selected));
	Util_watch_add(WATCH_HANDLE_SETTINGS_MENU, &sem_hungarian_button.selected, sizeof(sem_hungarian_button.selected));
	Util_watch_add(WATCH_HANDLE_SETTINGS_MENU, &sem_chinese_button.selected, sizeof(sem_chinese_button.selected));
	Util_watch_add(WATCH_HANDLE_SETTINGS_MENU, &sem_italian_button.selected, sizeof(sem_italian_button.selected));
	Util_watch_add(WATCH_HANDLE_SETTINGS_MENU, &sem_spanish_button.selected, sizeof(sem_spanish_button.selected));
	Util_watch_add(WATCH_HANDLE_SETTINGS_MENU, &sem_romanian_button.selected, sizeof(sem_romanian_button.selected));
	Util_watch_add(WATCH_HANDLE_SETTINGS_MENU, &sem_polish_button.selected, sizeof(sem_polish_button.selected));
	Util_watch_add(WATCH_HANDLE_SETTINGS_MENU, &sem_ryukyuan_button.selected, sizeof(sem_ryukyuan_button.selected));

	//LCD.
	Util_watch_add(WATCH_HANDLE_SETTINGS_MENU, &sem_config.is_night, sizeof(sem_config.is_night));
	Util_watch_add(WATCH_HANDLE_SETTINGS_MENU, &sem_config.is_flash, sizeof(sem_config.is_flash));
	Util_watch_add(WATCH_HANDLE_SETTINGS_MENU, &sem_config.top_lcd_brightness, sizeof(sem_config.top_lcd_brightness));
	Util_watch_add(WATCH_HANDLE_SETTINGS_MENU, &sem_config.bottom_lcd_brightness, sizeof(sem_config.bottom_lcd_brightness));
	Util_watch_add(WATCH_HANDLE_SETTINGS_MENU, &sem_config.time_to_turn_off_lcd, sizeof(sem_config.time_to_turn_off_lcd));
	Util_watch_add(WATCH_HANDLE_SETTINGS_MENU, &sem_config.time_to_enter_sleep, sizeof(sem_config.time_to_enter_sleep));
	Util_watch_add(WATCH_HANDLE_SETTINGS_MENU, &sem_config.screen_mode, sizeof(sem_config.screen_mode));

	Util_watch_add(WATCH_HANDLE_SETTINGS_MENU, &sem_night_mode_on_button.selected, sizeof(sem_night_mode_on_button.selected));
	Util_watch_add(WATCH_HANDLE_SETTINGS_MENU, &sem_night_mode_off_button.selected, sizeof(sem_night_mode_off_button.selected));
	Util_watch_add(WATCH_HANDLE_SETTINGS_MENU, &sem_flash_mode_button.selected, sizeof(sem_flash_mode_button.selected));
	Util_watch_add(WATCH_HANDLE_SETTINGS_MENU, &sem_screen_brightness_bar.selected, sizeof(sem_screen_brightness_bar.selected));
	Util_watch_add(WATCH_HANDLE_SETTINGS_MENU, &sem_screen_off_time_bar.selected, sizeof(sem_screen_off_time_bar.selected));
	Util_watch_add(WATCH_HANDLE_SETTINGS_MENU, &sem_sleep_time_bar.selected, sizeof(sem_sleep_time_bar.selected));
	Util_watch_add(WATCH_HANDLE_SETTINGS_MENU, &sem_800px_mode_button.selected, sizeof(sem_800px_mode_button.selected));
	Util_watch_add(WATCH_HANDLE_SETTINGS_MENU, &sem_3d_mode_button.selected, sizeof(sem_3d_mode_button.selected));
	Util_watch_add(WATCH_HANDLE_SETTINGS_MENU, &sem_400px_mode_button.selected, sizeof(sem_400px_mode_button.selected));
	Util_watch_add(WATCH_HANDLE_SETTINGS_MENU, &sem_auto_mode_button.selected, sizeof(sem_auto_mode_button.selected));

	//Scroll speed.
	Util_watch_add(WATCH_HANDLE_SETTINGS_MENU, &sem_config.scroll_speed, sizeof(sem_config.scroll_speed));

	Util_watch_add(WATCH_HANDLE_SETTINGS_MENU, &sem_scroll_speed_bar.selected, sizeof(sem_scroll_speed_bar.selected));

	//Font
	Util_watch_add(WATCH_HANDLE_SETTINGS_MENU, &sem_load_all_ex_font_button.selected, sizeof(sem_load_all_ex_font_button.selected));
	Util_watch_add(WATCH_HANDLE_SETTINGS_MENU, &sem_unload_all_ex_font_button.selected, sizeof(sem_unload_all_ex_font_button.selected));
	for(uint16_t i = 0; i < DEF_EXFONT_NUM_OF_FONT_NAME; i++)
		Util_watch_add(WATCH_HANDLE_SETTINGS_MENU, &sem_ex_font_button[i].selected, sizeof(sem_ex_font_button[i].selected));

	//Wireless.
	Util_watch_add(WATCH_HANDLE_SETTINGS_MENU, &sem_config.is_wifi_on, sizeof(sem_config.is_wifi_on));

	Util_watch_add(WATCH_HANDLE_SETTINGS_MENU, &sem_wifi_on_button.selected, sizeof(sem_wifi_on_button.selected));
	Util_watch_add(WATCH_HANDLE_SETTINGS_MENU, &sem_wifi_off_button.selected, sizeof(sem_wifi_off_button.selected));

	//Advanced settings.
	Util_watch_add(WATCH_HANDLE_SETTINGS_MENU, &sem_config.is_send_info_allowed, sizeof(sem_config.is_send_info_allowed));
	Util_watch_add(WATCH_HANDLE_SETTINGS_MENU, &sem_config.is_debug, sizeof(sem_config.is_debug));

	Util_watch_add(WATCH_HANDLE_SETTINGS_MENU, &sem_allow_send_info_button.selected, sizeof(sem_allow_send_info_button.selected));
	Util_watch_add(WATCH_HANDLE_SETTINGS_MENU, &sem_deny_send_info_button.selected, sizeof(sem_deny_send_info_button.selected));
	Util_watch_add(WATCH_HANDLE_SETTINGS_MENU, &sem_debug_mode_on_button.selected, sizeof(sem_debug_mode_on_button.selected));
	Util_watch_add(WATCH_HANDLE_SETTINGS_MENU, &sem_debug_mode_off_button.selected, sizeof(sem_debug_mode_off_button.selected));
	Util_watch_add(WATCH_HANDLE_SETTINGS_MENU, &sem_use_fake_model_button.selected, sizeof(sem_use_fake_model_button.selected));
	Util_watch_add(WATCH_HANDLE_SETTINGS_MENU, &sem_dump_log_button.selected, sizeof(sem_dump_log_button.selected));

#if DEF_CPU_USAGE_API_ENABLE
	//CPU usage.
	Util_watch_add(WATCH_HANDLE_SETTINGS_MENU, &sem_is_cpu_usage_monitor_running, sizeof(sem_is_cpu_usage_monitor_running));
	Util_watch_add(WATCH_HANDLE_SETTINGS_MENU, &sem_monitor_cpu_usage_on_button.selected, sizeof(sem_monitor_cpu_usage_on_button.selected));
	Util_watch_add(WATCH_HANDLE_SETTINGS_MENU, &sem_monitor_cpu_usage_off_button.selected, sizeof(sem_monitor_cpu_usage_off_button.selected));
#endif

	//Battery.
	Util_watch_add(WATCH_HANDLE_SETTINGS_MENU, &sem_config.is_eco, sizeof(sem_config.is_eco));
	Util_watch_add(WATCH_HANDLE_SETTINGS_MENU, &sem_eco_mode_on_button.selected, sizeof(sem_eco_mode_on_button.selected));
	Util_watch_add(WATCH_HANDLE_SETTINGS_MENU, &sem_eco_mode_off_button.selected, sizeof(sem_eco_mode_off_button.selected));

#if (DEF_ENCODER_VIDEO_AUDIO_API_ENABLE && DEF_CONVERTER_SW_API_ENABLE && DEF_SEM_ENABLE_SCREEN_RECORDER)
	//Screen recording.
	Util_watch_add(WATCH_HANDLE_SETTINGS_MENU, &sem_record_request, sizeof(sem_record_request));
	Util_watch_add(WATCH_HANDLE_SETTINGS_MENU, &sem_stop_record_request, sizeof(sem_stop_record_request));
#endif

	Util_watch_add(WATCH_HANDLE_SETTINGS_MENU, &sem_record_both_lcd_button.selected, sizeof(sem_record_both_lcd_button.selected));
	Util_watch_add(WATCH_HANDLE_SETTINGS_MENU, &sem_record_top_lcd_button.selected, sizeof(sem_record_top_lcd_button.selected));
	Util_watch_add(WATCH_HANDLE_SETTINGS_MENU, &sem_record_bottom_lcd_button.selected, sizeof(sem_record_bottom_lcd_button.selected));

	DEF_LOG_RESULT_SMART(result, Menu_add_worker_thread_callback(Sem_worker_callback), result, result);

	Sem_resume();
	sem_already_init = true;

	Sem_get_system_info();

	DEF_LOG_STRING("Initialized.");
}

void Sem_draw_init(void)
{
	Util_watch_add(WATCH_HANDLE_SETTINGS_MENU, &Draw_get_bot_ui_button()->selected, sizeof(Draw_get_bot_ui_button()->selected));
	sem_back_button = Draw_get_empty_image();
	sem_scroll_bar = Draw_get_empty_image();
	sem_english_button = Draw_get_empty_image();
	sem_japanese_button = Draw_get_empty_image();
	sem_hungarian_button = Draw_get_empty_image();
	sem_chinese_button = Draw_get_empty_image();
	sem_italian_button = Draw_get_empty_image();
	sem_spanish_button = Draw_get_empty_image();
	sem_romanian_button = Draw_get_empty_image();
	sem_polish_button = Draw_get_empty_image();
	sem_ryukyuan_button = Draw_get_empty_image();
	sem_night_mode_on_button = Draw_get_empty_image();
	sem_night_mode_off_button = Draw_get_empty_image();
	sem_flash_mode_button = Draw_get_empty_image();
	sem_screen_brightness_slider = Draw_get_empty_image();
	sem_screen_brightness_bar = Draw_get_empty_image();
	sem_screen_off_time_slider = Draw_get_empty_image();
	sem_screen_off_time_bar = Draw_get_empty_image();
	sem_sleep_time_slider = Draw_get_empty_image();
	sem_sleep_time_bar = Draw_get_empty_image();
	sem_800px_mode_button = Draw_get_empty_image();
	sem_3d_mode_button = Draw_get_empty_image();
	sem_400px_mode_button = Draw_get_empty_image();
	sem_auto_mode_button = Draw_get_empty_image();
	sem_scroll_speed_slider = Draw_get_empty_image();
	sem_scroll_speed_bar = Draw_get_empty_image();
	sem_load_all_ex_font_button = Draw_get_empty_image();
	sem_unload_all_ex_font_button = Draw_get_empty_image();
	sem_wifi_on_button = Draw_get_empty_image();
	sem_wifi_off_button = Draw_get_empty_image();
	sem_allow_send_info_button = Draw_get_empty_image();
	sem_deny_send_info_button = Draw_get_empty_image();
	sem_debug_mode_on_button = Draw_get_empty_image();
	sem_debug_mode_off_button = Draw_get_empty_image();
	sem_eco_mode_on_button = Draw_get_empty_image();
	sem_eco_mode_off_button = Draw_get_empty_image();
	sem_record_both_lcd_button = Draw_get_empty_image();
	sem_record_top_lcd_button = Draw_get_empty_image();
	sem_record_bottom_lcd_button = Draw_get_empty_image();
	sem_use_fake_model_button = Draw_get_empty_image();
	sem_dump_log_button = Draw_get_empty_image();

#if ((DEF_CURL_API_ENABLE || DEF_HTTPC_API_ENABLE) && DEF_SEM_ENABLE_UPDATER)
	sem_check_update_button = Draw_get_empty_image();
	sem_select_edtion_button = Draw_get_empty_image();
	sem_close_updater_button = Draw_get_empty_image();
	sem_3dsx_button = Draw_get_empty_image();
	sem_cia_button = Draw_get_empty_image();
	sem_dl_install_button = Draw_get_empty_image();
	sem_back_to_patch_note_button = Draw_get_empty_image();
	sem_close_app_button = Draw_get_empty_image();
#endif

#if DEF_CPU_USAGE_API_ENABLE
	sem_monitor_cpu_usage_on_button = Draw_get_empty_image();
	sem_monitor_cpu_usage_off_button = Draw_get_empty_image();
#endif

	for(uint8_t i = 0; i < 9; i++)
		sem_menu_button[i] = Draw_get_empty_image();
	for(uint16_t i = 0; i < DEF_EXFONT_NUM_OF_FONT_NAME; i++)
		sem_ex_font_button[i] = Draw_get_empty_image();
}

void Sem_exit(void)
{
	DEF_LOG_STRING("Exiting...");
	uint8_t brightness = 0;
	uint32_t result = DEF_ERR_OTHER;
	std::string data = "";
	Sem_config config = { 0, };
	Sem_state state = { 0, };

	Sem_get_config(&config);
	Sem_get_state(&state);

	brightness = Util_max(config.top_lcd_brightness, config.bottom_lcd_brightness);
	state.num_of_launch++;

	data = (std::string)"<0>" + config.lang + "</0><1>" + std::to_string(brightness) + "</1><2>" + std::to_string(config.time_to_turn_off_lcd)
	+ "</2><3>" + std::to_string(config.scroll_speed) + "</3><4>" + std::to_string(config.is_send_info_allowed) + "</4><5>" + std::to_string(state.num_of_launch)
	+ "</5><6>" + std::to_string(config.is_night) + "</6><7>" + std::to_string(config.is_eco) + "</7><8>" + std::to_string(config.is_wifi_on) + "</8>"
	+ "<9>0</9><10>0</10><11>" + std::to_string(config.screen_mode) + "</11><12>" + std::to_string(config.time_to_enter_sleep) + "</12>";
	//9 and 10 are no longer used.

#if (DEF_ENCODER_VIDEO_AUDIO_API_ENABLE && DEF_CONVERTER_SW_API_ENABLE && DEF_SEM_ENABLE_SCREEN_RECORDER)
	sem_stop_record_request = true;
#endif

	sem_already_init = false;
	sem_thread_suspend = false;
	sem_thread_run = false;
	Menu_remove_worker_thread_callback(Sem_worker_callback);

	//Save settings.
	DEF_LOG_RESULT_SMART(result, Util_file_save_to_file("settings.txt", DEF_MENU_MAIN_DIR, (uint8_t*)data.c_str(), data.length(), true), (result == DEF_SUCCESS), result);

	if(sem_internal_state.fake_model < DEF_SEM_MODEL_MAX)
	{
		//Fake mode has been configured, save it.
		DEF_LOG_RESULT_SMART(result, Util_file_save_to_file("fake_model.txt", DEF_MENU_MAIN_DIR, &sem_internal_state.fake_model, 1, true), (result == DEF_SUCCESS), result);
	}
	else
	{
		//Fake model has been turned off, delete the config file.
		DEF_LOG_RESULT_SMART(result, Util_file_delete_file("fake_model.txt", DEF_MENU_MAIN_DIR), (result == DEF_SUCCESS), result);
	}

	//Exit threads.
#if ((DEF_CURL_API_ENABLE || DEF_HTTPC_API_ENABLE) && DEF_SEM_ENABLE_UPDATER)
	DEF_LOG_RESULT_SMART(result, threadJoin(sem_update_thread, DEF_THREAD_WAIT_TIME), (result == DEF_SUCCESS), result);
	threadFree(sem_update_thread);
#endif

#if (DEF_ENCODER_VIDEO_AUDIO_API_ENABLE && DEF_CONVERTER_SW_API_ENABLE && DEF_SEM_ENABLE_SCREEN_RECORDER)
	DEF_LOG_RESULT_SMART(result, threadJoin(sem_encode_thread, DEF_THREAD_WAIT_TIME), (result == DEF_SUCCESS), result);
	DEF_LOG_RESULT_SMART(result, threadJoin(sem_record_thread, DEF_THREAD_WAIT_TIME), (result == DEF_SUCCESS), result);
	threadFree(sem_encode_thread);
	threadFree(sem_record_thread);
#endif

	DEF_LOG_RESULT_SMART(result, threadJoin(sem_hw_config_thread, DEF_THREAD_WAIT_TIME), (result == DEF_SUCCESS), result);
	threadFree(sem_hw_config_thread);

#if (DEF_CURL_API_ENABLE || DEF_HTTPC_API_ENABLE)
	DEF_LOG_RESULT_SMART(result, threadJoin(sem_check_connectivity_thread, DEF_THREAD_WAIT_TIME), (result == DEF_SUCCESS), result);
	threadFree(sem_check_connectivity_thread);
#endif

	//Global.
	Util_watch_remove(WATCH_HANDLE_SETTINGS_MENU, &Draw_get_bot_ui_button()->selected);
	Util_watch_remove(WATCH_HANDLE_SETTINGS_MENU, &sem_y_offset);
	Util_watch_remove(WATCH_HANDLE_SETTINGS_MENU, &sem_y_max);
	Util_watch_remove(WATCH_HANDLE_SETTINGS_MENU, &sem_selected_menu_mode);
	Util_watch_remove(WATCH_HANDLE_SETTINGS_MENU, &sem_scroll_mode);
	Util_watch_remove(WATCH_HANDLE_SETTINGS_MENU, &sem_scroll_bar.selected);

	Util_watch_remove(WATCH_HANDLE_SETTINGS_MENU, &sem_back_button.selected);
	for(uint8_t i = 0; i < 9; i++)
		Util_watch_remove(WATCH_HANDLE_SETTINGS_MENU, &sem_menu_button[i].selected);

#if ((DEF_CURL_API_ENABLE || DEF_HTTPC_API_ENABLE) && DEF_SEM_ENABLE_UPDATER)
	//Updater.
	Util_watch_remove(WATCH_HANDLE_SETTINGS_MENU, &sem_show_patch_note_request);
	Util_watch_remove(WATCH_HANDLE_SETTINGS_MENU, &sem_select_ver_request);
	Util_watch_remove(WATCH_HANDLE_SETTINGS_MENU, &sem_dl_file_request);
	Util_watch_remove(WATCH_HANDLE_SETTINGS_MENU, &sem_check_update_request);
	Util_watch_remove(WATCH_HANDLE_SETTINGS_MENU, &sem_selected_edition_num);
	Util_watch_remove(WATCH_HANDLE_SETTINGS_MENU, &sem_installed_size);
	Util_watch_remove(WATCH_HANDLE_SETTINGS_MENU, &sem_dled_size);

	Util_watch_remove(WATCH_HANDLE_SETTINGS_MENU, &sem_close_updater_button.selected);
	Util_watch_remove(WATCH_HANDLE_SETTINGS_MENU, &sem_select_edtion_button.selected);
	Util_watch_remove(WATCH_HANDLE_SETTINGS_MENU, &sem_3dsx_button.selected);
	Util_watch_remove(WATCH_HANDLE_SETTINGS_MENU, &sem_cia_button.selected);
	Util_watch_remove(WATCH_HANDLE_SETTINGS_MENU, &sem_back_to_patch_note_button.selected);
	Util_watch_remove(WATCH_HANDLE_SETTINGS_MENU, &sem_dl_install_button.selected);
	Util_watch_remove(WATCH_HANDLE_SETTINGS_MENU, &sem_close_app_button.selected);
	Util_watch_remove(WATCH_HANDLE_SETTINGS_MENU, &sem_check_update_button.selected);
#endif

	//Languages.
	Util_watch_remove(WATCH_HANDLE_SETTINGS_MENU, &sem_reload_msg_request);

	Util_watch_remove(WATCH_HANDLE_SETTINGS_MENU, &sem_english_button.selected);
	Util_watch_remove(WATCH_HANDLE_SETTINGS_MENU, &sem_japanese_button.selected);
	Util_watch_remove(WATCH_HANDLE_SETTINGS_MENU, &sem_hungarian_button.selected);
	Util_watch_remove(WATCH_HANDLE_SETTINGS_MENU, &sem_chinese_button.selected);
	Util_watch_remove(WATCH_HANDLE_SETTINGS_MENU, &sem_italian_button.selected);
	Util_watch_remove(WATCH_HANDLE_SETTINGS_MENU, &sem_spanish_button.selected);
	Util_watch_remove(WATCH_HANDLE_SETTINGS_MENU, &sem_romanian_button.selected);
	Util_watch_remove(WATCH_HANDLE_SETTINGS_MENU, &sem_polish_button.selected);
	Util_watch_remove(WATCH_HANDLE_SETTINGS_MENU, &sem_ryukyuan_button.selected);

	//LCD.
	Util_watch_remove(WATCH_HANDLE_SETTINGS_MENU, &sem_config.is_night);
	Util_watch_remove(WATCH_HANDLE_SETTINGS_MENU, &sem_config.is_flash);
	Util_watch_remove(WATCH_HANDLE_SETTINGS_MENU, &sem_config.top_lcd_brightness);
	Util_watch_remove(WATCH_HANDLE_SETTINGS_MENU, &sem_config.bottom_lcd_brightness);
	Util_watch_remove(WATCH_HANDLE_SETTINGS_MENU, &sem_config.time_to_turn_off_lcd);
	Util_watch_remove(WATCH_HANDLE_SETTINGS_MENU, &sem_config.time_to_enter_sleep);
	Util_watch_remove(WATCH_HANDLE_SETTINGS_MENU, &sem_config.screen_mode);

	Util_watch_remove(WATCH_HANDLE_SETTINGS_MENU, &sem_night_mode_on_button.selected);
	Util_watch_remove(WATCH_HANDLE_SETTINGS_MENU, &sem_night_mode_off_button.selected);
	Util_watch_remove(WATCH_HANDLE_SETTINGS_MENU, &sem_flash_mode_button.selected);
	Util_watch_remove(WATCH_HANDLE_SETTINGS_MENU, &sem_screen_brightness_bar.selected);
	Util_watch_remove(WATCH_HANDLE_SETTINGS_MENU, &sem_screen_off_time_bar.selected);
	Util_watch_remove(WATCH_HANDLE_SETTINGS_MENU, &sem_sleep_time_bar.selected);
	Util_watch_remove(WATCH_HANDLE_SETTINGS_MENU, &sem_800px_mode_button.selected);
	Util_watch_remove(WATCH_HANDLE_SETTINGS_MENU, &sem_3d_mode_button.selected);
	Util_watch_remove(WATCH_HANDLE_SETTINGS_MENU, &sem_400px_mode_button.selected);
	Util_watch_remove(WATCH_HANDLE_SETTINGS_MENU, &sem_auto_mode_button.selected);

	//Scroll speed.
	Util_watch_remove(WATCH_HANDLE_SETTINGS_MENU, &sem_config.scroll_speed);

	Util_watch_remove(WATCH_HANDLE_SETTINGS_MENU, &sem_scroll_speed_bar.selected);

	//Font.
	Util_watch_remove(WATCH_HANDLE_SETTINGS_MENU, &sem_load_all_ex_font_button.selected);
	Util_watch_remove(WATCH_HANDLE_SETTINGS_MENU, &sem_unload_all_ex_font_button.selected);
	for(uint16_t i = 0; i < DEF_EXFONT_NUM_OF_FONT_NAME; i++)
		Util_watch_remove(WATCH_HANDLE_SETTINGS_MENU, &sem_ex_font_button[i].selected);

	//Wireless.
	Util_watch_remove(WATCH_HANDLE_SETTINGS_MENU, &sem_config.is_wifi_on);

	Util_watch_remove(WATCH_HANDLE_SETTINGS_MENU, &sem_wifi_on_button.selected);
	Util_watch_remove(WATCH_HANDLE_SETTINGS_MENU, &sem_wifi_off_button.selected);

	//Advanced settings.
	Util_watch_remove(WATCH_HANDLE_SETTINGS_MENU, &sem_config.is_send_info_allowed);
	Util_watch_remove(WATCH_HANDLE_SETTINGS_MENU, &sem_config.is_debug);

	Util_watch_remove(WATCH_HANDLE_SETTINGS_MENU, &sem_allow_send_info_button.selected);
	Util_watch_remove(WATCH_HANDLE_SETTINGS_MENU, &sem_deny_send_info_button.selected);
	Util_watch_remove(WATCH_HANDLE_SETTINGS_MENU, &sem_debug_mode_on_button.selected);
	Util_watch_remove(WATCH_HANDLE_SETTINGS_MENU, &sem_debug_mode_off_button.selected);
	Util_watch_remove(WATCH_HANDLE_SETTINGS_MENU, &sem_use_fake_model_button.selected);
	Util_watch_remove(WATCH_HANDLE_SETTINGS_MENU, &sem_dump_log_button.selected);

#if DEF_CPU_USAGE_API_ENABLE
	//CPU usage.
	Util_watch_remove(WATCH_HANDLE_SETTINGS_MENU, &sem_is_cpu_usage_monitor_running);
	Util_watch_remove(WATCH_HANDLE_SETTINGS_MENU, &sem_monitor_cpu_usage_on_button.selected);
	Util_watch_remove(WATCH_HANDLE_SETTINGS_MENU, &sem_monitor_cpu_usage_off_button.selected);
#endif

	//Battery.
	Util_watch_remove(WATCH_HANDLE_SETTINGS_MENU, &sem_config.is_eco);
	Util_watch_remove(WATCH_HANDLE_SETTINGS_MENU, &sem_eco_mode_on_button.selected);
	Util_watch_remove(WATCH_HANDLE_SETTINGS_MENU, &sem_eco_mode_off_button.selected);

#if (DEF_ENCODER_VIDEO_AUDIO_API_ENABLE && DEF_CONVERTER_SW_API_ENABLE && DEF_SEM_ENABLE_SCREEN_RECORDER)
	//Screen recording.
	Util_watch_remove(WATCH_HANDLE_SETTINGS_MENU, &sem_record_request);
	Util_watch_remove(WATCH_HANDLE_SETTINGS_MENU, &sem_stop_record_request);
#endif

	Util_watch_remove(WATCH_HANDLE_SETTINGS_MENU, &sem_record_both_lcd_button.selected);
	Util_watch_remove(WATCH_HANDLE_SETTINGS_MENU, &sem_record_top_lcd_button.selected);
	Util_watch_remove(WATCH_HANDLE_SETTINGS_MENU, &sem_record_bottom_lcd_button.selected);

	DEF_LOG_STRING("Exited.");
}

void Sem_main(void)
{
	uint32_t color = DEF_DRAW_BLACK;
	uint32_t back_color = DEF_DRAW_WHITE;
	uint32_t cache_color[DEF_EXFONT_NUM_OF_FONT_NAME];
	Watch_handle_bit watch_handle_bit = (DEF_WATCH_HANDLE_BIT_GLOBAL | DEF_WATCH_HANDLE_BIT_SETTINGS_MENU);
	Sem_config config = { 0, };
	Sem_state state = { 0, };

	Sem_get_config(&config);
	Sem_get_state(&state);

	if (config.is_night)
	{
		color = DEF_DRAW_WHITE;
		back_color = DEF_DRAW_BLACK;
	}

	for(uint16_t i = 0; i < DEF_EXFONT_NUM_OF_FONT_NAME; i++)
		cache_color[i] = color;

	//Check if we should update the screen.
	if(Util_watch_is_changed(watch_handle_bit) || Draw_is_refresh_needed() || !config.is_eco)
	{
		double draw_x = 0;
		double draw_y = 0;
		Str_data format_str = { 0, };
		Draw_image_data background = Draw_get_empty_image();

		Draw_set_refresh_needed(false);
		Util_str_init(&format_str);
		Draw_frame_ready();
		Draw_screen_ready(DRAW_SCREEN_TOP_LEFT, back_color);

		if(Util_log_query_log_show_flag())
			Util_log_draw();

		Draw_top_ui(config.is_eco, state.is_charging, state.wifi_signal, state.battery_level, state.msg);

		if(config.is_debug)
			Draw_debug_info(config.is_night, state.free_ram, state.free_linear_ram);

		if(Util_cpu_usage_query_show_flag())
			Util_cpu_usage_draw();

		Draw_screen_ready(DRAW_SCREEN_BOTTOM, back_color);

		if (sem_selected_menu_mode >= DEF_SEM_MENU_UPDATE && sem_selected_menu_mode <= DEF_SEM_MENU_RECORDING)
		{
			draw_y = 0.0;
			if (draw_y + sem_y_offset >= -30 && draw_y + sem_y_offset <= 240)
			{
				//Back.
				Draw_with_background(&sem_msg[DEF_SEM_BACK_MSG], 0.0, draw_y + sem_y_offset, 0.55, 0.55, color, DRAW_X_ALIGN_CENTER, DRAW_Y_ALIGN_CENTER,
				55, 25, DRAW_BACKGROUND_ENTIRE_BOX, &sem_back_button, sem_back_button.selected ? DEF_DRAW_RED : DEF_DRAW_WEAK_RED);
			}
		}

		//Scroll bar.
		if (sem_selected_menu_mode == DEF_SEM_MENU_LANGAGES || sem_selected_menu_mode == DEF_SEM_MENU_LCD
		|| sem_selected_menu_mode == DEF_SEM_MENU_FONT)
		{
			Draw_texture(&background, color, 312.5, 0.0, 7.5, 15.0);
			Draw_texture(&background, color, 312.5, 215.0, 7.5, 10.0);
			Draw_texture(&sem_scroll_bar, sem_scroll_bar.selected ? DEF_DRAW_BLUE : DEF_DRAW_WEAK_BLUE, 312.5, 15.0 + (195 * (sem_y_offset / sem_y_max)), 7.5, 5.0);
		}

		if (sem_selected_menu_mode == DEF_SEM_MENU_TOP)
		{
			//Update.
			Draw_with_background(&sem_msg[DEF_SEM_UPDATE_MSG], 0, 0, 0.75, 0.75, color, DRAW_X_ALIGN_LEFT, DRAW_Y_ALIGN_CENTER, 240, 20,
			DRAW_BACKGROUND_ENTIRE_BOX, &sem_menu_button[DEF_SEM_MENU_UPDATE], sem_menu_button[DEF_SEM_MENU_UPDATE].selected ? DEF_DRAW_AQUA : DEF_DRAW_WEAK_AQUA);

			//Languages.
			Draw_with_background(&sem_msg[DEF_SEM_LANGAGES_MSG], 0, 25, 0.75, 0.75, color, DRAW_X_ALIGN_LEFT, DRAW_Y_ALIGN_CENTER, 240, 20,
			DRAW_BACKGROUND_ENTIRE_BOX, &sem_menu_button[DEF_SEM_MENU_LANGAGES], sem_menu_button[DEF_SEM_MENU_LANGAGES].selected ? DEF_DRAW_AQUA : DEF_DRAW_WEAK_AQUA);

			//LCD.
			Draw_with_background(&sem_msg[DEF_SEM_LCD_MSG], 0, 50, 0.75, 0.75, color, DRAW_X_ALIGN_LEFT, DRAW_Y_ALIGN_CENTER, 240, 20,
			DRAW_BACKGROUND_ENTIRE_BOX, &sem_menu_button[DEF_SEM_MENU_LCD], sem_menu_button[DEF_SEM_MENU_LCD].selected ? DEF_DRAW_AQUA : DEF_DRAW_WEAK_AQUA);

			//Control.
			Draw_with_background(&sem_msg[DEF_SEM_CONTROL_MSG], 0, 75, 0.75, 0.75, color, DRAW_X_ALIGN_LEFT, DRAW_Y_ALIGN_CENTER, 240, 20,
			DRAW_BACKGROUND_ENTIRE_BOX, &sem_menu_button[DEF_SEM_MENU_CONTROL], sem_menu_button[DEF_SEM_MENU_CONTROL].selected ? DEF_DRAW_AQUA : DEF_DRAW_WEAK_AQUA);

			//Font.
			Draw_with_background(&sem_msg[DEF_SEM_FONT_MSG], 0, 100, 0.75, 0.75, color, DRAW_X_ALIGN_LEFT, DRAW_Y_ALIGN_CENTER, 240, 20,
			DRAW_BACKGROUND_ENTIRE_BOX, &sem_menu_button[DEF_SEM_MENU_FONT], sem_menu_button[DEF_SEM_MENU_FONT].selected ? DEF_DRAW_AQUA : DEF_DRAW_WEAK_AQUA);

			//Wireless.
			Draw_with_background(&sem_msg[DEF_SEM_WIFI_MSG], 0, 125, 0.75, 0.75, color, DRAW_X_ALIGN_LEFT, DRAW_Y_ALIGN_CENTER, 240, 20,
			DRAW_BACKGROUND_ENTIRE_BOX, &sem_menu_button[DEF_SEM_MENU_WIFI], sem_menu_button[DEF_SEM_MENU_WIFI].selected ? DEF_DRAW_AQUA : DEF_DRAW_WEAK_AQUA);

			//Advanced.
			Draw_with_background(&sem_msg[DEF_SEM_ADVANCED_MSG], 0, 150, 0.75, 0.75, color, DRAW_X_ALIGN_LEFT, DRAW_Y_ALIGN_CENTER, 240, 20,
			DRAW_BACKGROUND_ENTIRE_BOX, &sem_menu_button[DEF_SEM_MENU_ADVANCED], sem_menu_button[DEF_SEM_MENU_ADVANCED].selected ? DEF_DRAW_AQUA : DEF_DRAW_WEAK_AQUA);

			//Battery.
			Draw_with_background(&sem_msg[DEF_SEM_BATTERY_MSG], 0, 175, 0.75, 0.75, color, DRAW_X_ALIGN_LEFT, DRAW_Y_ALIGN_CENTER, 240, 20,
			DRAW_BACKGROUND_ENTIRE_BOX, &sem_menu_button[DEF_SEM_MENU_BATTERY], sem_menu_button[DEF_SEM_MENU_BATTERY].selected ? DEF_DRAW_AQUA : DEF_DRAW_WEAK_AQUA);

			//Screen recording.
			Draw_with_background(&sem_msg[DEF_SEM_RECORDING_MSG], 0, 200, 0.75, 0.75, color, DRAW_X_ALIGN_LEFT, DRAW_Y_ALIGN_CENTER, 240, 20,
			DRAW_BACKGROUND_ENTIRE_BOX, &sem_menu_button[DEF_SEM_MENU_RECORDING], sem_menu_button[DEF_SEM_MENU_RECORDING].selected ? DEF_DRAW_AQUA : DEF_DRAW_WEAK_AQUA);
		}
		else if (sem_selected_menu_mode == DEF_SEM_MENU_UPDATE)
		{
#if ((DEF_CURL_API_ENABLE || DEF_HTTPC_API_ENABLE) && DEF_SEM_ENABLE_UPDATER)
			//Check for updates.
			Draw_with_background(&sem_msg[DEF_SEM_CHECK_UPDATE_MSG], 10, 25, 0.75, 0.75, color, DRAW_X_ALIGN_LEFT, DRAW_Y_ALIGN_CENTER, 240, 20,
			DRAW_BACKGROUND_ENTIRE_BOX, &sem_check_update_button, sem_check_update_button.selected ? DEF_DRAW_AQUA : DEF_DRAW_WEAK_AQUA);

			if (sem_show_patch_note_request)
			{
				Draw_texture(&background, DEF_DRAW_AQUA, 15, 15, 290, 200);
				Draw_texture(&sem_select_edtion_button, sem_select_edtion_button.selected ? DEF_DRAW_GREEN : DEF_DRAW_WEAK_GREEN, 15, 200, 145, 15);
				Draw_texture(&sem_close_updater_button, sem_close_updater_button.selected ? DEF_DRAW_WHITE : DEF_DRAW_WEAK_WHITE, 160, 200, 145, 15);

				if(sem_update_progress == 0)//Checking.
					Draw(&sem_msg[DEF_SEM_CHECKING_UPDATE_MSG], 17.5, 15, 0.5, 0.5, DEF_DRAW_BLACK);
				else if(sem_update_progress == -1)//Failed.
					Draw(&sem_msg[DEF_SEM_CHECKING_UPDATE_FAILED_MSG], 17.5, 15, 0.5, 0.5, DEF_DRAW_BLACK);
				else if (sem_update_progress == 1)//Success.
				{
					Draw(&sem_msg[sem_new_version_available ? DEF_SEM_NEW_VERSION_AVAILABLE_MSG : DEF_SEM_UP_TO_DATE_MSG], 17.5, 15, 0.5, 0.5, DEF_DRAW_BLACK);
					Draw_c(sem_newest_ver_data[5].c_str(), 17.5, 35, 0.425, 0.425, DEF_DRAW_BLACK);
				}
				if(strcmp(config.lang, "ro") == 0)
					Draw(&sem_msg[DEF_SEM_SELECT_EDITION_MSG], 17.5, 200, 0.35, 0.35, DEF_DRAW_BLACK);
				else
					Draw(&sem_msg[DEF_SEM_SELECT_EDITION_MSG], 17.5, 200, 0.425, 0.425, DEF_DRAW_BLACK);

				Draw(&sem_msg[DEF_SEM_CLOSE_UPDATER_MSG], 162.5, 200, 0.425, 0.425, DEF_DRAW_BLACK);
			}
			else if (sem_select_ver_request)
			{
				Draw_texture(&background, DEF_DRAW_AQUA, 15, 15, 290, 200);
				Draw_texture(&sem_back_to_patch_note_button, sem_back_to_patch_note_button.selected ? DEF_DRAW_WHITE : DEF_DRAW_WEAK_WHITE, 15, 200, 145, 15);
				Draw_texture(&sem_dl_install_button, sem_dl_install_button.selected ? DEF_DRAW_GREEN : DEF_DRAW_WEAK_GREEN, 160, 200, 145, 15);
				Draw_texture(&sem_3dsx_button, sem_3dsx_button.selected ? DEF_DRAW_RED : DEF_DRAW_WEAK_RED, 15, 15, 100, 25);
				Draw_texture(&sem_cia_button, sem_cia_button.selected ? DEF_DRAW_RED : DEF_DRAW_WEAK_RED, 15, 45, 100, 25);

				//3dsx.
				if(sem_selected_edition_num == DEF_SEM_EDTION_3DSX)
					Draw(&sem_msg[DEF_SEM_3DSX_MSG], 17.5, 15, 0.8, 0.8, DEF_DRAW_RED);
				else if(sem_newest_ver_data[1] == "1")
					Draw(&sem_msg[DEF_SEM_3DSX_MSG], 17.5, 15, 0.8, 0.8, DEF_DRAW_BLACK);
				else
					Draw(&sem_msg[DEF_SEM_3DSX_MSG], 17.5, 15, 0.8, 0.8, DEF_DRAW_WEAK_BLACK);

				//Cia.
				if(sem_selected_edition_num == DEF_SEM_EDTION_CIA)
					Draw(&sem_msg[DEF_SEM_CIA_MSG], 17.5, 45, 0.8, 0.8, DEF_DRAW_RED);
				else if(sem_newest_ver_data[2] == "1")
					Draw(&sem_msg[DEF_SEM_CIA_MSG], 17.5, 45, 0.8, 0.8, DEF_DRAW_BLACK);
				else
					Draw(&sem_msg[DEF_SEM_CIA_MSG], 17.5, 45, 0.8, 0.8, DEF_DRAW_WEAK_BLACK);

				if (sem_selected_edition_num == DEF_SEM_EDTION_3DSX)
				{
					Util_str_format(&format_str, "sdmc:%s%s/%s.3dsx", DEF_SEM_UPDATE_DIR_PREFIX, sem_newest_ver_data[0].c_str(), DEF_SEM_UPDATE_FILE_PREFIX);
					Draw(&sem_msg[DEF_SEM_FILE_PATH_MSG], 17.5, 140, 0.5, 0.5, DEF_DRAW_BLACK);
					Draw(&format_str, 17.5, 150, 0.425, 0.425, DEF_DRAW_RED);
				}

				if(sem_update_progress == 2)
				{
					//Downloading.
					Draw_c((std::to_string(sem_dled_size / 1000.0 / 1000.0).substr(0, 4) + "MB(" + std::to_string(sem_dled_size / 1000) + "KB)").c_str(), 17.5, 180, 0.425, 0.425, DEF_DRAW_BLACK);
					Draw(&sem_msg[DEF_SEM_DOWNLOADING_MSG], 17.5, 160, 0.75, 0.75, DEF_DRAW_BLACK);
				}
				else if(sem_update_progress == 3)
				{
					//Installing.
					Draw_c((std::to_string(sem_installed_size / 1000.0 / 1000.0).substr(0, 4) + "MB/" + std::to_string(sem_total_cia_size / 1000.0 / 1000.0).substr(0, 4) + "MB").c_str(), 17.5, 180, 0.425, 0.425, DEF_DRAW_BLACK);
					Draw(&sem_msg[DEF_SEM_INSTALLING_MSG], 17.5, 160, 0.75, 0.75, DEF_DRAW_BLACK);
				}
				else if (sem_update_progress == 4)
				{
					//Success.
					Draw(&sem_msg[DEF_SEM_SUCCESS_MSG], 17.5, 160, 0.75, 0.75, DEF_DRAW_BLACK);
					Draw(&sem_msg[DEF_SEM_RESTART_MSG], 17.5, 180, 0.45, 0.45, DEF_DRAW_BLACK);
					Draw_texture(&sem_close_app_button, sem_close_app_button.selected ? DEF_DRAW_YELLOW : DEF_DRAW_WEAK_YELLOW, 250, 180, 55.0, 20.0);
					Draw(&sem_msg[DEF_SEM_CLOSE_APP_MSG], 250, 180, 0.375, 0.375, DEF_DRAW_BLACK);
				}
				else if (sem_update_progress == -2)
					Draw(&sem_msg[DEF_SEM_FAILURE_MSG], 17.5, 160, 0.75, 0.75, DEF_DRAW_BLACK);

				Draw(&sem_msg[DEF_SEM_DL_INSTALL_MSG], 162.5, 200, 0.425, 0.425, (sem_selected_edition_num != DEF_SEM_EDTION_NONE && sem_newest_ver_data[1 + sem_selected_edition_num] == "1") ? DEF_DRAW_BLACK : DEF_DRAW_WEAK_BLACK);
				Draw(&sem_msg[DEF_SEM_BACK_TO_PATCH_NOTE_MSG], 17.5, 200, 0.45, 0.45, DEF_DRAW_BLACK);
			}
#else
			Draw_c("☢Updater is disabled\non this app.☢", 10, 25, 0.75, 0.75, DEF_DRAW_RED);
#endif
		}
		else if (sem_selected_menu_mode == DEF_SEM_MENU_LANGAGES)
		{
			//Languages.

			//English.
			Draw_with_background(&sem_msg[DEF_SEM_ENGLISH_MSG], 10, 25 + sem_y_offset, 0.75, 0.75, (strcmp(config.lang, "en") == 0) ? DEF_DRAW_RED : color,
			DRAW_X_ALIGN_LEFT, DRAW_Y_ALIGN_CENTER, 240, 20, DRAW_BACKGROUND_ENTIRE_BOX, &sem_english_button, sem_english_button.selected ? DEF_DRAW_AQUA : DEF_DRAW_WEAK_AQUA);

			//Japanese.
			Draw_with_background(&sem_msg[DEF_SEM_JAPANESE_MSG], 10, 50 + sem_y_offset, 0.75, 0.75, (strcmp(config.lang, "jp") == 0) ? DEF_DRAW_RED : color,
			DRAW_X_ALIGN_LEFT, DRAW_Y_ALIGN_CENTER, 240, 20, DRAW_BACKGROUND_ENTIRE_BOX, &sem_japanese_button, sem_japanese_button.selected ? DEF_DRAW_AQUA : DEF_DRAW_WEAK_AQUA);

			//Hungarian.
			Draw_with_background(&sem_msg[DEF_SEM_HUNGARIAN_MSG], 10, 75 + sem_y_offset, 0.75, 0.75, (strcmp(config.lang, "hu") == 0) ? DEF_DRAW_RED : color,
			DRAW_X_ALIGN_LEFT, DRAW_Y_ALIGN_CENTER, 240, 20, DRAW_BACKGROUND_ENTIRE_BOX, &sem_hungarian_button, sem_hungarian_button.selected ? DEF_DRAW_AQUA : DEF_DRAW_WEAK_AQUA);

			//Chinese.
			Draw_with_background(&sem_msg[DEF_SEM_CHINESE_MSG], 10, 100 + sem_y_offset, 0.75, 0.75, (strcmp(config.lang, "zh-cn") == 0) ? DEF_DRAW_RED : color,
			DRAW_X_ALIGN_LEFT, DRAW_Y_ALIGN_CENTER, 240, 20, DRAW_BACKGROUND_ENTIRE_BOX, &sem_chinese_button, sem_chinese_button.selected ? DEF_DRAW_AQUA : DEF_DRAW_WEAK_AQUA);

			//Italian.
			Draw_with_background(&sem_msg[DEF_SEM_ITALIAN_MSG], 10, 125 + sem_y_offset, 0.75, 0.75, (strcmp(config.lang, "it") == 0) ? DEF_DRAW_RED : color,
			DRAW_X_ALIGN_LEFT, DRAW_Y_ALIGN_CENTER, 240, 20, DRAW_BACKGROUND_ENTIRE_BOX, &sem_italian_button, sem_italian_button.selected ? DEF_DRAW_AQUA : DEF_DRAW_WEAK_AQUA);

			//Spanish.
			Draw_with_background(&sem_msg[DEF_SEM_SPANISH_MSG], 10, 150 + sem_y_offset, 0.75, 0.75, (strcmp(config.lang, "es") == 0) ? DEF_DRAW_RED : color,
			DRAW_X_ALIGN_LEFT, DRAW_Y_ALIGN_CENTER, 240, 20, DRAW_BACKGROUND_ENTIRE_BOX, &sem_spanish_button, sem_spanish_button.selected ? DEF_DRAW_AQUA : DEF_DRAW_WEAK_AQUA);

			//Romanian.
			Draw_with_background(&sem_msg[DEF_SEM_ROMANIAN_MSG], 10, 175 + sem_y_offset, 0.75, 0.75, (strcmp(config.lang, "ro") == 0) ? DEF_DRAW_RED : color,
			DRAW_X_ALIGN_LEFT, DRAW_Y_ALIGN_CENTER, 240, 20, DRAW_BACKGROUND_ENTIRE_BOX, &sem_romanian_button, sem_romanian_button.selected ? DEF_DRAW_AQUA : DEF_DRAW_WEAK_AQUA);

			//Polish.
			Draw_with_background(&sem_msg[DEF_SEM_POLISH_MSG], 10, 200 + sem_y_offset, 0.75, 0.75, (strcmp(config.lang, "pl") == 0) ? DEF_DRAW_RED : color,
			DRAW_X_ALIGN_LEFT, DRAW_Y_ALIGN_CENTER, 240, 20, DRAW_BACKGROUND_ENTIRE_BOX, &sem_polish_button, sem_polish_button.selected ? DEF_DRAW_AQUA : DEF_DRAW_WEAK_AQUA);

			//Ryukyuan.
			Draw_with_background(&sem_msg[DEF_SEM_RYUKYUAN_MSG], 10, 225 + sem_y_offset, 0.75, 0.75, (strcmp(config.lang, "ryu") == 0) ? DEF_DRAW_RED : color,
			DRAW_X_ALIGN_LEFT, DRAW_Y_ALIGN_CENTER, 240, 20, DRAW_BACKGROUND_ENTIRE_BOX, &sem_ryukyuan_button, sem_ryukyuan_button.selected ? DEF_DRAW_AQUA : DEF_DRAW_WEAK_AQUA);
		}
		else if (sem_selected_menu_mode == DEF_SEM_MENU_LCD)
		{
			uint8_t brightness = Util_max(config.top_lcd_brightness, config.bottom_lcd_brightness);
			double bar_pos = 0;

#if (DEF_ENCODER_VIDEO_AUDIO_API_ENABLE && DEF_CONVERTER_SW_API_ENABLE && DEF_SEM_ENABLE_SCREEN_RECORDER)
			if(sem_record_request && config.is_night)
			{
				cache_color[0] = DEF_DRAW_WEAK_WHITE;
				cache_color[1] = DEF_DRAW_WEAK_WHITE;
				cache_color[2] = DEF_DRAW_WEAK_WHITE;
			}
			else if(sem_record_request)
			{
				cache_color[0] = DEF_DRAW_WEAK_BLACK;
				cache_color[1] = DEF_DRAW_WEAK_BLACK;
				cache_color[2] = DEF_DRAW_WEAK_BLACK;
			}
#endif

			if(state.console_model == DEF_SEM_MODEL_OLD2DS && config.is_night)
			{
				cache_color[0] = DEF_DRAW_WEAK_WHITE;
				cache_color[1] = DEF_DRAW_WEAK_WHITE;
			}
			else if(state.console_model == DEF_SEM_MODEL_OLD2DS)
			{
				cache_color[0] = DEF_DRAW_WEAK_BLACK;
				cache_color[1] = DEF_DRAW_WEAK_BLACK;
			}

			if(state.console_model == DEF_SEM_MODEL_NEW2DSXL && config.is_night)
				cache_color[1] = DEF_DRAW_WEAK_WHITE;
			else if(state.console_model == DEF_SEM_MODEL_NEW2DSXL)
				cache_color[1] = DEF_DRAW_WEAK_BLACK;

			//Night mode.
			draw_y = 25;
			Draw(&sem_msg[DEF_SEM_NIGHT_MODE_MSG], 0, draw_y + sem_y_offset, 0.5, 0.5, color);

			//ON.
			draw_y += 20;
			Draw_with_background(&sem_msg[DEF_SEM_ON_MSG], 10, draw_y + sem_y_offset, 0.55, 0.55, config.is_night ? DEF_DRAW_RED : color, DRAW_X_ALIGN_CENTER,
			DRAW_Y_ALIGN_CENTER, 140, 20, DRAW_BACKGROUND_ENTIRE_BOX, &sem_night_mode_on_button, sem_night_mode_on_button.selected ? DEF_DRAW_AQUA : DEF_DRAW_WEAK_AQUA);
			//OFF.
			Draw_with_background(&sem_msg[DEF_SEM_OFF_MSG], 170, draw_y + sem_y_offset, 0.55, 0.55, config.is_night ? color : DEF_DRAW_RED, DRAW_X_ALIGN_CENTER,
			DRAW_Y_ALIGN_CENTER, 140, 20, DRAW_BACKGROUND_ENTIRE_BOX, &sem_night_mode_off_button, sem_night_mode_off_button.selected ? DEF_DRAW_AQUA : DEF_DRAW_WEAK_AQUA);

			//Flash.
			draw_y += 25;
			Draw_with_background(&sem_msg[DEF_SEM_FLASH_MSG], 10, draw_y + sem_y_offset, 0.8, 0.8, sem_config.is_flash ? DEF_DRAW_RED : color, DRAW_X_ALIGN_CENTER,
			DRAW_Y_ALIGN_CENTER, 300, 20, DRAW_BACKGROUND_ENTIRE_BOX, &sem_flash_mode_button, sem_flash_mode_button.selected ? DEF_DRAW_RED : DEF_DRAW_WEAK_RED);

			//Screen brightness.
			draw_y += 30;
			bar_pos = 10 + (290 * (brightness / 180.0));
			Util_str_format(&format_str, "%s%" PRIu8, DEF_STR_NEVER_NULL(&sem_msg[DEF_SEM_BRIGHTNESS_MSG]), brightness);
			Draw(&format_str, 0, draw_y + sem_y_offset, 0.5, 0.5, color);
			//Bar.
			draw_y += 15;
			Draw_texture(&sem_screen_brightness_slider, DEF_DRAW_WEAK_RED, 10, draw_y + sem_y_offset + 6.5, 300, 7);
			Draw_texture(&sem_screen_brightness_bar, sem_screen_brightness_bar.selected ? DEF_DRAW_GREEN : DEF_DRAW_WEAK_GREEN, bar_pos, draw_y + sem_y_offset, 10, 20);

			//Time to turn off LCDs.
			draw_y += 25;
			if(config.time_to_turn_off_lcd > 0)
			{
				bar_pos = 10 + (290 * ((config.time_to_turn_off_lcd - 20) / 580.0));
				Util_str_format(&format_str, "%s%" PRIu16, DEF_STR_NEVER_NULL(&sem_msg[DEF_SEM_LCD_OFF_TIME_0_MSG]), config.time_to_turn_off_lcd);
				Util_str_format_append(&format_str, "%s", DEF_STR_NEVER_NULL(&sem_msg[DEF_SEM_LCD_OFF_TIME_1_MSG]));
				Draw(&format_str, 0, draw_y + sem_y_offset, 0.5, 0.5, color);
			}
			else
			{
				//Never turn off LCD automatically.
				bar_pos = 300;
				Util_str_format(&format_str, "%s", DEF_STR_NEVER_NULL(&sem_msg[DEF_SEM_LCD_OFF_TIME_0_MSG]));
				Util_str_format_append(&format_str, "%s", DEF_STR_NEVER_NULL(&sem_msg[DEF_SEM_OFF_MSG]));
				Draw(&format_str, 0, draw_y + sem_y_offset, 0.5, 0.5, color);
			}

			//Bar.
			draw_y += 15;
			Draw_texture(&sem_screen_off_time_slider, DEF_DRAW_WEAK_RED, 10, draw_y + sem_y_offset + 6.5, 300, 7);
			Draw_texture(&sem_screen_off_time_bar, sem_screen_off_time_bar.selected ? DEF_DRAW_GREEN : DEF_DRAW_WEAK_GREEN, bar_pos, draw_y + sem_y_offset, 10, 20);

			//Time to enter sleep.
			draw_y += 25;
			if(config.time_to_enter_sleep > 0)
			{
				bar_pos = 10 + (290 * ((config.time_to_enter_sleep - 20) / 580.0));
				Util_str_format(&format_str, "%s%" PRIu16, DEF_STR_NEVER_NULL(&sem_msg[DEF_SEM_SLEEP_TIME_MSG]), config.time_to_enter_sleep);
				Util_str_format_append(&format_str, "%s", DEF_STR_NEVER_NULL(&sem_msg[DEF_SEM_LCD_OFF_TIME_1_MSG]));//DEF_SEM_LCD_OFF_TIME_1_MSG is intentional.
				Draw(&format_str, 0, draw_y + sem_y_offset, 0.5, 0.5, color);
			}
			else
			{
				//Never enter sleep automatically.
				bar_pos = 300;
				Util_str_format(&format_str, "%s", DEF_STR_NEVER_NULL(&sem_msg[DEF_SEM_SLEEP_TIME_MSG]));
				Util_str_format_append(&format_str, "%s", DEF_STR_NEVER_NULL(&sem_msg[DEF_SEM_OFF_MSG]));
				Draw(&format_str, 0, draw_y + sem_y_offset, 0.5, 0.5, color);
			}

			//Bar.
			draw_y += 15;
			Draw_texture(&sem_sleep_time_slider, DEF_DRAW_WEAK_RED, 10, draw_y + sem_y_offset + 6.5, 300, 7);
			Draw_texture(&sem_sleep_time_bar, sem_sleep_time_bar.selected ? DEF_DRAW_GREEN : DEF_DRAW_WEAK_GREEN, bar_pos, draw_y + sem_y_offset, 10, 20);

			//Screen mode.
			draw_y += 25;
			Draw(&sem_msg[DEF_SEM_LCD_MODE_MSG], 0, draw_y + sem_y_offset, 0.5, 0.5, color);

			//800px.
			draw_y += 15;
			Draw_with_background(&sem_msg[DEF_SEM_800PX_MSG], 10, draw_y + sem_y_offset, 0.65, 0.65, (config.screen_mode == DEF_SEM_SCREEN_MODE_800PX) ? DEF_DRAW_RED : cache_color[0],
			DRAW_X_ALIGN_CENTER, DRAW_Y_ALIGN_CENTER, 65, 20, DRAW_BACKGROUND_ENTIRE_BOX, &sem_800px_mode_button, sem_800px_mode_button.selected ? DEF_DRAW_AQUA : DEF_DRAW_WEAK_AQUA);
			//3D.
			Draw_with_background(&sem_msg[DEF_SEM_3D_MSG], 85, draw_y + sem_y_offset, 0.65, 0.65, (config.screen_mode == DEF_SEM_SCREEN_MODE_3D) ? DEF_DRAW_RED : cache_color[1],
			DRAW_X_ALIGN_CENTER, DRAW_Y_ALIGN_CENTER, 65, 20, DRAW_BACKGROUND_ENTIRE_BOX, &sem_3d_mode_button, sem_3d_mode_button.selected ? DEF_DRAW_AQUA : DEF_DRAW_WEAK_AQUA);
			//Nothing.
			Draw_with_background(&sem_msg[DEF_SEM_400PX_MSG], 160, draw_y + sem_y_offset, 0.65, 0.65, (config.screen_mode == DEF_SEM_SCREEN_MODE_400PX) ? DEF_DRAW_RED : cache_color[2],
			DRAW_X_ALIGN_CENTER, DRAW_Y_ALIGN_CENTER, 65, 20, DRAW_BACKGROUND_ENTIRE_BOX, &sem_400px_mode_button, sem_400px_mode_button.selected ? DEF_DRAW_AQUA : DEF_DRAW_WEAK_AQUA);
			//Auto.
			Draw_with_background(&sem_msg[DEF_SEM_AUTO_MSG], 235, draw_y + sem_y_offset, 0.65, 0.65, (config.screen_mode == DEF_SEM_SCREEN_MODE_AUTO) ? DEF_DRAW_RED : cache_color[2],
			DRAW_X_ALIGN_CENTER, DRAW_Y_ALIGN_CENTER, 65, 20, DRAW_BACKGROUND_ENTIRE_BOX, &sem_auto_mode_button, sem_auto_mode_button.selected ? DEF_DRAW_AQUA : DEF_DRAW_WEAK_AQUA);
		}
		else if (sem_selected_menu_mode == DEF_SEM_MENU_CONTROL)
		{
			//Scroll speed.
			double bar_pos = 10 + (290 * ((config.scroll_speed - 0.05) / 1.95));

			Util_str_format(&format_str, "%s%f", DEF_STR_NEVER_NULL(&sem_msg[DEF_SEM_SCROLL_SPEED_MSG]), config.scroll_speed);
			Draw(&format_str, 0, 25, 0.5, 0.5, color);
			//Bar.
			Draw_texture(&sem_scroll_speed_slider, DEF_DRAW_WEAK_RED, 10, 46.5, 300, 7);
			Draw_texture(&sem_scroll_speed_bar, sem_scroll_speed_bar.selected ? DEF_DRAW_GREEN : DEF_DRAW_WEAK_GREEN, bar_pos, 40, 10, 20);
		}
		else if (sem_selected_menu_mode == DEF_SEM_MENU_FONT)
		{
			//Font.
			if (30 + sem_y_offset >= -30 && 30 + sem_y_offset <= 240)
			{
				cache_color[0] = color;
				if ((Exfont_is_unloading_external_font() || Exfont_is_loading_external_font()) && config.is_night)
					cache_color[0] = DEF_DRAW_WEAK_WHITE;
				else if (Exfont_is_unloading_external_font() || Exfont_is_loading_external_font())
					cache_color[0] = DEF_DRAW_WEAK_BLACK;

				//Load all.
				Draw_with_background(&sem_msg[DEF_SEM_LOAD_ALL_FONT_MSG], 10, 30 + sem_y_offset, 0.65, 0.65, cache_color[0], DRAW_X_ALIGN_CENTER, DRAW_Y_ALIGN_CENTER,
				150, 20, DRAW_BACKGROUND_ENTIRE_BOX, &sem_load_all_ex_font_button, sem_load_all_ex_font_button.selected ? DEF_DRAW_RED : DEF_DRAW_WEAK_RED);

				//Unload all.
				Draw_with_background(&sem_msg[DEF_SEM_UNLOAD_ALL_FONT_MSG], 160, 30 + sem_y_offset, 0.65, 0.65, cache_color[0], DRAW_X_ALIGN_CENTER, DRAW_Y_ALIGN_CENTER,
				150, 20, DRAW_BACKGROUND_ENTIRE_BOX, &sem_unload_all_ex_font_button, sem_unload_all_ex_font_button.selected ? DEF_DRAW_YELLOW : DEF_DRAW_WEAK_YELLOW);
			}

			draw_x = 10.0;
			draw_y = 50.0;
			for(uint16_t i = 0; i < DEF_EXFONT_NUM_OF_FONT_NAME; i++)
				cache_color[i] = color;

			for (uint16_t i = 0; i < DEF_EXFONT_NUM_OF_FONT_NAME; i++)
			{
				if (Exfont_is_loaded_external_font(i))
				{
					if(Exfont_is_unloading_external_font() || Exfont_is_loading_external_font())
						cache_color[i] = DEF_DRAW_WEAK_RED;
					else
						cache_color[i] = DEF_DRAW_RED;
				}
				else if ((Exfont_is_unloading_external_font() || Exfont_is_loading_external_font()) && config.is_night)
					cache_color[i] = DEF_DRAW_WEAK_WHITE;
				else if (Exfont_is_unloading_external_font() || Exfont_is_loading_external_font())
					cache_color[i] = DEF_DRAW_WEAK_BLACK;

				if (draw_y + sem_y_offset >= -30 && draw_y + sem_y_offset <= 240)
				{
					Draw_with_background_c(Exfont_query_external_font_name(i), draw_x, draw_y + sem_y_offset, 0.45, 0.45, cache_color[i], DRAW_X_ALIGN_LEFT,
					DRAW_Y_ALIGN_CENTER, 300, 20, DRAW_BACKGROUND_ENTIRE_BOX, &sem_ex_font_button[i], sem_ex_font_button[i].selected ? DEF_DRAW_AQUA : DEF_DRAW_WEAK_AQUA);
				}
				draw_y += 20.0;
			}
		}
		else if (sem_selected_menu_mode == DEF_SEM_MENU_WIFI)
		{
			//Wifi.
			Draw(&sem_msg[DEF_SEM_WIFI_MODE_MSG], 0, 25, 0.5, 0.5, color);

			//ON.
			Draw_with_background(&sem_msg[DEF_SEM_ON_MSG], 10, 40, 0.55, 0.55, config.is_wifi_on ? DEF_DRAW_RED : color, DRAW_X_ALIGN_CENTER, DRAW_Y_ALIGN_CENTER,
			90, 20, DRAW_BACKGROUND_ENTIRE_BOX, &sem_wifi_on_button, sem_wifi_on_button.selected ? DEF_DRAW_AQUA : DEF_DRAW_WEAK_AQUA);
			//OFF.
			Draw_with_background(&sem_msg[DEF_SEM_OFF_MSG], 110, 40, 0.55, 0.55, config.is_wifi_on ? color : DEF_DRAW_RED, DRAW_X_ALIGN_CENTER, DRAW_Y_ALIGN_CENTER,
			90, 20, DRAW_BACKGROUND_ENTIRE_BOX, &sem_wifi_off_button, sem_wifi_off_button.selected ? DEF_DRAW_AQUA : DEF_DRAW_WEAK_AQUA);

			//Connected SSID.
			Util_str_format(&format_str, "%s%s", DEF_STR_NEVER_NULL(&sem_msg[DEF_SEM_CONNECTED_SSID_MSG]), state.connected_wifi);
			Draw(&format_str, 0, 65, 0.425, 0.425, color);
		}
		else if (sem_selected_menu_mode == DEF_SEM_MENU_ADVANCED)
		{
			//Allow send app info.
			Draw(&sem_msg[DEF_SEM_SEND_INFO_MODE_MSG], 0, 25, 0.5, 0.5, color);

			//Allow.
			Draw_with_background(&sem_msg[DEF_SEM_ALLOW_MSG], 10, 40, 0.65, 0.65, config.is_send_info_allowed ? DEF_DRAW_RED : color, DRAW_X_ALIGN_CENTER, DRAW_Y_ALIGN_CENTER,
			90, 20, DRAW_BACKGROUND_ENTIRE_BOX, &sem_allow_send_info_button, sem_allow_send_info_button.selected ? DEF_DRAW_AQUA : DEF_DRAW_WEAK_AQUA);
			//Deny.
			Draw_with_background(&sem_msg[DEF_SEM_DENY_MSG], 110, 40, 0.65, 0.65, config.is_send_info_allowed ? color : DEF_DRAW_RED, DRAW_X_ALIGN_CENTER, DRAW_Y_ALIGN_CENTER,
			90, 20, DRAW_BACKGROUND_ENTIRE_BOX, &sem_deny_send_info_button, sem_deny_send_info_button.selected ? DEF_DRAW_AQUA : DEF_DRAW_WEAK_AQUA);

			//Debug mode.
			Draw(&sem_msg[DEF_SEM_DEBUG_MODE_MSG], 0, 65, 0.5, 0.5, color);

			//ON.
			Draw_with_background(&sem_msg[DEF_SEM_ON_MSG], 10, 80, 0.55, 0.55, config.is_debug ? DEF_DRAW_RED : color, DRAW_X_ALIGN_CENTER, DRAW_Y_ALIGN_CENTER,
			90, 20, DRAW_BACKGROUND_ENTIRE_BOX, &sem_debug_mode_on_button, sem_debug_mode_on_button.selected ? DEF_DRAW_AQUA : DEF_DRAW_WEAK_AQUA);
			//OFF.
			Draw_with_background(&sem_msg[DEF_SEM_OFF_MSG], 110, 80, 0.55, 0.55, config.is_debug ? color : DEF_DRAW_RED, DRAW_X_ALIGN_CENTER, DRAW_Y_ALIGN_CENTER,
			90, 20, DRAW_BACKGROUND_ENTIRE_BOX, &sem_debug_mode_off_button, sem_debug_mode_off_button.selected ? DEF_DRAW_AQUA : DEF_DRAW_WEAK_AQUA);

			//Fake model.
			Draw(&sem_msg[DEF_SEM_FAKE_MODEL_MSG], 0, 105, 0.5, 0.5, color);
			if(sem_internal_state.fake_model < DEF_SEM_MODEL_MAX)
			{
				Util_str_format(&format_str, "%s (%s)", DEF_STR_NEVER_NULL(&sem_msg[DEF_SEM_ON_MSG]), sem_model_name[sem_internal_state.fake_model]);
				Draw_with_background(&format_str, 10, 135, 0.65, 0.65, color, DRAW_X_ALIGN_CENTER,
				DRAW_Y_ALIGN_CENTER, 190, 20, DRAW_BACKGROUND_ENTIRE_BOX, &sem_use_fake_model_button, sem_use_fake_model_button.selected ? DEF_DRAW_AQUA : DEF_DRAW_WEAK_AQUA);
			}
			else
			{
				Draw_with_background(&sem_msg[DEF_SEM_OFF_MSG], 10, 135, 0.65, 0.65, color, DRAW_X_ALIGN_CENTER, DRAW_Y_ALIGN_CENTER, 190, 20,
				DRAW_BACKGROUND_ENTIRE_BOX, &sem_use_fake_model_button, sem_use_fake_model_button.selected ? DEF_DRAW_AQUA : DEF_DRAW_WEAK_AQUA);
			}

			Draw_with_background(&sem_msg[DEF_SEM_DUMP_LOGS_MSG], 10, 165, 0.5, 0.5, color, DRAW_X_ALIGN_CENTER, DRAW_Y_ALIGN_CENTER, 190, 20,
			DRAW_BACKGROUND_ENTIRE_BOX, &sem_dump_log_button, sem_dump_log_button.selected ? DEF_DRAW_AQUA : DEF_DRAW_WEAK_AQUA);

#if DEF_CPU_USAGE_API_ENABLE
			//CPU usage monitor.
			Draw(&sem_msg[DEF_SEM_CPU_USAGE_MONITOR_MSG], 0, 185, 0.5, 0.5, color);

			//ON.
			Draw_with_background(&sem_msg[DEF_SEM_ON_MSG], 10, 200, 0.55, 0.55, sem_is_cpu_usage_monitor_running ? DEF_DRAW_RED : color, DRAW_X_ALIGN_CENTER, DRAW_Y_ALIGN_CENTER,
			90, 20, DRAW_BACKGROUND_ENTIRE_BOX, &sem_monitor_cpu_usage_on_button, sem_monitor_cpu_usage_on_button.selected ? DEF_DRAW_AQUA : DEF_DRAW_WEAK_AQUA);
			//OFF.
			Draw_with_background(&sem_msg[DEF_SEM_OFF_MSG], 110, 200, 0.55, 0.55, sem_is_cpu_usage_monitor_running ? color : DEF_DRAW_RED, DRAW_X_ALIGN_CENTER, DRAW_Y_ALIGN_CENTER,
			90, 20, DRAW_BACKGROUND_ENTIRE_BOX, &sem_monitor_cpu_usage_off_button, sem_monitor_cpu_usage_off_button.selected ? DEF_DRAW_AQUA : DEF_DRAW_WEAK_AQUA);
#endif
		}
		else if (sem_selected_menu_mode == DEF_SEM_MENU_BATTERY)
		{
			//Eco mode.
			Draw(&sem_msg[DEF_SEM_ECO_MODE_MSG], 0, 25, 0.5, 0.5, color);

			//ON.
			Draw_with_background(&sem_msg[DEF_SEM_ON_MSG], 10, 40, 0.55, 0.55, config.is_eco ? DEF_DRAW_RED : color, DRAW_X_ALIGN_CENTER, DRAW_Y_ALIGN_CENTER,
			90, 20, DRAW_BACKGROUND_ENTIRE_BOX, &sem_eco_mode_on_button, sem_eco_mode_on_button.selected ? DEF_DRAW_AQUA : DEF_DRAW_WEAK_AQUA);
			//OFF.
			Draw_with_background(&sem_msg[DEF_SEM_OFF_MSG], 110, 40, 0.55, 0.55, config.is_eco ? color : DEF_DRAW_RED, DRAW_X_ALIGN_CENTER, DRAW_Y_ALIGN_CENTER,
			90, 20, DRAW_BACKGROUND_ENTIRE_BOX, &sem_eco_mode_off_button, sem_eco_mode_off_button.selected ? DEF_DRAW_AQUA : DEF_DRAW_WEAK_AQUA);
		}
		else if (sem_selected_menu_mode == DEF_SEM_MENU_RECORDING)
		{
#if (DEF_ENCODER_VIDEO_AUDIO_API_ENABLE && DEF_CONVERTER_SW_API_ENABLE && DEF_SEM_ENABLE_SCREEN_RECORDER)
			bool can_record = (config.screen_mode == DEF_SEM_SCREEN_MODE_400PX || config.screen_mode == DEF_SEM_SCREEN_MODE_3D);

			if(!can_record)
				cache_color[0] = (config.is_night ? DEF_DRAW_WEAK_WHITE : DEF_DRAW_WEAK_BLACK);

			//Record both screen.
			Draw_with_background(&sem_msg[sem_record_request ? DEF_SEM_STOP_RECORDING_MSG : DEF_SEM_RECORD_BOTH_LCD_MSG], 10, 25, 0.475, 0.475, cache_color[0], DRAW_X_ALIGN_CENTER,
			DRAW_Y_ALIGN_CENTER, 240, 20, DRAW_BACKGROUND_ENTIRE_BOX, &sem_record_both_lcd_button, sem_record_both_lcd_button.selected ? DEF_DRAW_AQUA : DEF_DRAW_WEAK_AQUA);

			//Record top screen.
			Draw_with_background(&sem_msg[sem_record_request ? DEF_SEM_STOP_RECORDING_MSG : DEF_SEM_RECORD_TOP_LCD_MSG], 10, 60, 0.475, 0.475, cache_color[0], DRAW_X_ALIGN_CENTER,
			DRAW_Y_ALIGN_CENTER, 240, 20, DRAW_BACKGROUND_ENTIRE_BOX, &sem_record_top_lcd_button, sem_record_top_lcd_button.selected ? DEF_DRAW_AQUA : DEF_DRAW_WEAK_AQUA);

			//Record bottom screen.
			Draw_with_background(&sem_msg[sem_record_request ? DEF_SEM_STOP_RECORDING_MSG : DEF_SEM_RECORD_BOTTOM_LCD_MSG], 10, 95, 0.475, 0.475, cache_color[0], DRAW_X_ALIGN_CENTER,
			DRAW_Y_ALIGN_CENTER, 240, 20, DRAW_BACKGROUND_ENTIRE_BOX, &sem_record_bottom_lcd_button, sem_record_bottom_lcd_button.selected ? DEF_DRAW_AQUA : DEF_DRAW_WEAK_AQUA);

			if(!can_record)
				Draw(&sem_msg[DEF_SEM_CANNOT_RECORD_MSG], 10, 120, 0.5, 0.5, DEF_DRAW_RED);
#else
			Draw_c("☢Screen recorder is disabled\non this app.☢", 10, 25, 0.75, 0.75, DEF_DRAW_RED);
#endif
		}

		if(Util_err_query_error_show_flag())
			Util_err_draw();

		Draw_bot_ui();

		Draw_apply_draw();
		Util_str_free(&format_str);
	}
	else
		gspWaitForVBlank();
}

void Sem_hid(Hid_info key)
{
	int8_t menu_button_list[9] = { DEF_SEM_MENU_UPDATE, DEF_SEM_MENU_LANGAGES, DEF_SEM_MENU_LCD, DEF_SEM_MENU_CONTROL,
	DEF_SEM_MENU_FONT, DEF_SEM_MENU_WIFI, DEF_SEM_MENU_ADVANCED, DEF_SEM_MENU_BATTERY, DEF_SEM_MENU_RECORDING };
	Sem_config config = { 0, };
	Sem_state state = { 0, };

	if(aptShouldJumpToHome())
		return;

	Sem_get_config(&config);
	Sem_get_state(&state);

	if (Util_err_query_error_show_flag())
		Util_err_main(key);
	else
	{
		if(Util_hid_is_pressed(key, *Draw_get_bot_ui_button()))
			Draw_get_bot_ui_button()->selected = true;
		else if (key.p_start || (Util_hid_is_released(key, *Draw_get_bot_ui_button()) && Draw_get_bot_ui_button()->selected))
			Sem_suspend();
		else if (sem_selected_menu_mode == DEF_SEM_MENU_TOP)
		{
			for(uint8_t i = 0; i < 9; i++)
			{
				if(Util_hid_is_pressed(key, sem_menu_button[menu_button_list[i]]))
					sem_menu_button[menu_button_list[i]].selected = true;
				else if(Util_hid_is_released(key, sem_menu_button[menu_button_list[i]]) && sem_menu_button[menu_button_list[i]].selected)
				{
					sem_y_offset = 0.0;
					sem_selected_menu_mode = menu_button_list[i];
					if (sem_selected_menu_mode == DEF_SEM_MENU_LANGAGES)
						sem_y_max = -50.0;
					else if (sem_selected_menu_mode == DEF_SEM_MENU_LCD)
						sem_y_max = -60.0;
					else if (sem_selected_menu_mode == DEF_SEM_MENU_FONT)
						sem_y_max = -950.0;
				}
			}
		}
		else if(sem_selected_menu_mode >= DEF_SEM_MENU_UPDATE && sem_selected_menu_mode <= DEF_SEM_MENU_RECORDING)
		{
			bool enable_back_button = true;

#if ((DEF_CURL_API_ENABLE || DEF_HTTPC_API_ENABLE) && DEF_SEM_ENABLE_UPDATER)
			enable_back_button = (!sem_show_patch_note_request && !sem_select_ver_request);
#endif

			if (Util_hid_is_pressed(key, sem_back_button) && enable_back_button)
				sem_back_button.selected = true;
			else if (Util_hid_is_released(key, sem_back_button) && sem_back_button.selected && enable_back_button)
			{
				sem_y_offset = 0.0;
				sem_y_max = 0.0;
				sem_selected_menu_mode = DEF_SEM_MENU_TOP;
			}
			else if (sem_back_button.selected && !Util_hid_is_held(key, sem_back_button))
				sem_back_button.selected = false;
			else if (sem_selected_menu_mode == DEF_SEM_MENU_UPDATE)//Check for updates
			{
#if ((DEF_CURL_API_ENABLE || DEF_HTTPC_API_ENABLE) && DEF_SEM_ENABLE_UPDATER)
				if (sem_show_patch_note_request)
				{
					if (Util_hid_is_pressed(key, sem_close_updater_button))
						sem_close_updater_button.selected = true;
					else if (key.p_b || (Util_hid_is_released(key, sem_close_updater_button) && sem_close_updater_button.selected))
						sem_show_patch_note_request = false;
					else if (Util_hid_is_pressed(key, sem_select_edtion_button))
						sem_select_edtion_button.selected = true;
					else if (key.p_a || (Util_hid_is_released(key, sem_select_edtion_button) && sem_select_edtion_button.selected))
					{
						sem_show_patch_note_request = false;
						sem_select_ver_request = true;
					}
				}
				else if (sem_select_ver_request && !sem_dl_file_request)
				{
					if (Util_hid_is_pressed(key, sem_3dsx_button) && sem_newest_ver_data[1] == "1")
						sem_3dsx_button.selected = true;
					else if (Util_hid_is_released(key, sem_3dsx_button) && sem_newest_ver_data[1] == "1" && sem_3dsx_button.selected)
						sem_selected_edition_num = DEF_SEM_EDTION_3DSX;
					else if (Util_hid_is_pressed(key, sem_cia_button) && sem_newest_ver_data[2] == "1")
						sem_cia_button.selected = true;
					else if (Util_hid_is_released(key, sem_cia_button) && sem_newest_ver_data[2] == "1" && sem_cia_button.selected)
						sem_selected_edition_num = DEF_SEM_EDTION_CIA;
					else if (Util_hid_is_pressed(key, sem_back_to_patch_note_button))
						sem_back_to_patch_note_button.selected = true;
					else if (key.p_b || (Util_hid_is_released(key, sem_back_to_patch_note_button) && sem_back_to_patch_note_button.selected))
					{
						sem_show_patch_note_request = true;
						sem_select_ver_request = false;
					}
					else if (Util_hid_is_pressed(key, sem_dl_install_button) && sem_selected_edition_num != DEF_SEM_EDTION_NONE && sem_newest_ver_data[1 + sem_selected_edition_num] == "1")
						sem_dl_install_button.selected = true;
					else if ((key.p_x || (Util_hid_is_released(key, sem_dl_install_button) && sem_dl_install_button.selected)) && sem_selected_edition_num != DEF_SEM_EDTION_NONE && sem_newest_ver_data[1 + sem_selected_edition_num] == "1")
						sem_dl_file_request = true;
					else if(Util_hid_is_pressed(key, sem_close_app_button) && sem_update_progress == 4)
						sem_close_app_button.selected = true;
					else if(Util_hid_is_released(key, sem_close_app_button) && sem_update_progress == 4 && sem_close_app_button.selected)
						Menu_set_must_exit_flag(true);
				}
				else
				{
					if(Util_hid_is_pressed(key, sem_check_update_button))
						sem_check_update_button.selected = true;
					if(Util_hid_is_released(key, sem_check_update_button) && sem_check_update_button.selected)
					{
						sem_check_update_request = true;
						sem_show_patch_note_request = true;
					}
				}
#endif
			}
			else if (sem_selected_menu_mode == DEF_SEM_MENU_LANGAGES)//Language
			{
				if(key.p_touch || key.h_touch || key.r_touch)
				{
					if(!sem_reload_msg_request)
					{
						if(Util_hid_is_pressed(key, sem_english_button))
							sem_english_button.selected = true;
						else if(Util_hid_is_released(key, sem_english_button) && sem_english_button.selected)
						{
							snprintf(config.lang, sizeof(config.lang), "en");
							Sem_set_config(&config);
							sem_reload_msg_request = true;
						}
						else if(sem_english_button.selected && !Util_hid_is_held(key, sem_english_button))
							sem_english_button.selected = false;
						else if(Util_hid_is_pressed(key, sem_japanese_button))
							sem_japanese_button.selected = true;
						else if(Util_hid_is_released(key, sem_japanese_button) && sem_japanese_button.selected)
						{
							snprintf(config.lang, sizeof(config.lang), "jp");
							Sem_set_config(&config);
							sem_reload_msg_request = true;
						}
						else if(sem_japanese_button.selected && !Util_hid_is_held(key, sem_japanese_button))
							sem_japanese_button.selected = false;
						else if(Util_hid_is_pressed(key, sem_hungarian_button))
							sem_hungarian_button.selected = true;
						else if(Util_hid_is_released(key, sem_hungarian_button) && sem_hungarian_button.selected)
						{
							snprintf(config.lang, sizeof(config.lang), "hu");
							Sem_set_config(&config);
							sem_reload_msg_request = true;
						}
						else if(sem_hungarian_button.selected && !Util_hid_is_held(key, sem_hungarian_button))
							sem_hungarian_button.selected = false;
						else if(Util_hid_is_pressed(key, sem_chinese_button))
							sem_chinese_button.selected = true;
						else if(Util_hid_is_released(key, sem_chinese_button) && sem_chinese_button.selected)
						{
							snprintf(config.lang, sizeof(config.lang), "zh-cn");
							Sem_set_config(&config);
							sem_reload_msg_request = true;
						}
						else if(sem_chinese_button.selected && !Util_hid_is_held(key, sem_chinese_button))
							sem_chinese_button.selected = false;
						else if(Util_hid_is_pressed(key, sem_italian_button))
							sem_italian_button.selected = true;
						else if(Util_hid_is_released(key, sem_italian_button) && sem_italian_button.selected)
						{
							snprintf(config.lang, sizeof(config.lang), "it");
							Sem_set_config(&config);
							sem_reload_msg_request = true;
						}
						else if(sem_italian_button.selected && !Util_hid_is_held(key, sem_italian_button))
							sem_italian_button.selected = false;
						else if(Util_hid_is_pressed(key, sem_spanish_button))
							sem_spanish_button.selected = true;
						else if(Util_hid_is_released(key, sem_spanish_button) && sem_spanish_button.selected)
						{
							snprintf(config.lang, sizeof(config.lang), "es");
							Sem_set_config(&config);
							sem_reload_msg_request = true;
						}
						else if(sem_spanish_button.selected && !Util_hid_is_held(key, sem_spanish_button))
							sem_spanish_button.selected = false;
						else if(Util_hid_is_pressed(key, sem_romanian_button))
							sem_romanian_button.selected = true;
						else if(Util_hid_is_released(key, sem_romanian_button) && sem_romanian_button.selected)
						{
							snprintf(config.lang, sizeof(config.lang), "ro");
							Sem_set_config(&config);
							sem_reload_msg_request = true;
						}
						else if(sem_romanian_button.selected && !Util_hid_is_held(key, sem_romanian_button))
							sem_romanian_button.selected = false;
						else if(Util_hid_is_pressed(key, sem_polish_button))
							sem_polish_button.selected = true;
						else if(Util_hid_is_released(key, sem_polish_button) && sem_polish_button.selected)
						{
							snprintf(config.lang, sizeof(config.lang), "pl");
							Sem_set_config(&config);
							sem_reload_msg_request = true;
						}
						else if(sem_polish_button.selected && !Util_hid_is_held(key, sem_polish_button))
							sem_polish_button.selected = false;
						else if(Util_hid_is_pressed(key, sem_ryukyuan_button))
							sem_ryukyuan_button.selected = true;
						else if(Util_hid_is_released(key, sem_ryukyuan_button) && sem_ryukyuan_button.selected)
						{
							snprintf(config.lang, sizeof(config.lang), "ryu");
							Sem_set_config(&config);
							sem_reload_msg_request = true;
						}
						else if(sem_ryukyuan_button.selected && !Util_hid_is_held(key, sem_ryukyuan_button))
							sem_ryukyuan_button.selected = false;
					}

					sem_scroll_mode = true;
					if(sem_english_button.selected || sem_japanese_button.selected || sem_hungarian_button.selected || sem_chinese_button.selected
					|| sem_italian_button.selected || sem_spanish_button.selected || sem_romanian_button.selected || sem_polish_button.selected
					|| sem_ryukyuan_button.selected || Draw_get_bot_ui_button()->selected)
						sem_scroll_mode = false;
				}
			}
			else if (sem_selected_menu_mode == DEF_SEM_MENU_LCD)//LCD
			{
				bool record_request = false;

#if (DEF_ENCODER_VIDEO_AUDIO_API_ENABLE && DEF_CONVERTER_SW_API_ENABLE && DEF_SEM_ENABLE_SCREEN_RECORDER)
				record_request = sem_record_request;
#endif

				if(Util_hid_is_pressed(key, sem_night_mode_on_button))
					sem_night_mode_on_button.selected = true;
				else if(Util_hid_is_released(key, sem_night_mode_on_button) && sem_night_mode_on_button.selected)
				{
					config.is_night = true;
					Sem_set_config(&config);
				}
				else if(sem_night_mode_on_button.selected && !Util_hid_is_held(key, sem_night_mode_on_button))
					sem_night_mode_on_button.selected = false;
				else if(Util_hid_is_pressed(key, sem_night_mode_off_button))
					sem_night_mode_off_button.selected = true;
				else if(Util_hid_is_released(key, sem_night_mode_off_button) && sem_night_mode_off_button.selected)
				{
					config.is_night = false;
					Sem_set_config(&config);
				}
				else if(sem_night_mode_off_button.selected && !Util_hid_is_held(key, sem_night_mode_off_button))
					sem_night_mode_off_button.selected = false;
				else if(Util_hid_is_pressed(key, sem_flash_mode_button))
					sem_flash_mode_button.selected = true;
				else if(Util_hid_is_released(key, sem_flash_mode_button) && sem_flash_mode_button.selected)
				{
					config.is_flash = !config.is_flash;
					Sem_set_config(&config);
				}
				else if(sem_flash_mode_button.selected && !Util_hid_is_held(key, sem_flash_mode_button))
					sem_flash_mode_button.selected = false;
				else if(Util_hid_is_pressed(key, sem_screen_brightness_bar) || Util_hid_is_pressed(key, sem_screen_brightness_slider)
				|| (key.h_touch && sem_screen_brightness_bar.selected))
				{
					//Update screen brightness.
					int32_t new_brightness = 180 * ((key.touch_x - 15) / 290.0);

					if(new_brightness < 0)
						new_brightness = 0;
					else if(new_brightness > 180)
						new_brightness = 180;

					config.top_lcd_brightness = new_brightness;
					config.bottom_lcd_brightness = new_brightness;
					Sem_set_config(&config);

					sem_screen_brightness_bar.selected = true;
				}
				else if(Util_hid_is_pressed(key, sem_screen_off_time_bar) || Util_hid_is_pressed(key, sem_screen_off_time_slider)
				|| (key.h_touch && sem_screen_off_time_bar.selected))
				{
					//Update time to turn off LCD.
					uint16_t new_time = (580 * ((key.touch_x - 15) / 290.0)) + 20;

					if(new_time < 20)
						new_time = 20;
					else if(new_time > 600)
						new_time = 0;//Never turn off.

					config.time_to_turn_off_lcd = new_time;
					if(config.time_to_turn_off_lcd > 0 && config.time_to_enter_sleep > 0 && (config.time_to_turn_off_lcd > config.time_to_enter_sleep))
						config.time_to_enter_sleep = config.time_to_turn_off_lcd;
					else if(config.time_to_turn_off_lcd < 0)
						config.time_to_enter_sleep = 0;

					Sem_set_config(&config);

					sem_screen_off_time_bar.selected = true;
				}
				else if(Util_hid_is_pressed(key, sem_sleep_time_bar) || Util_hid_is_pressed(key, sem_sleep_time_slider)
				|| (key.h_touch && sem_sleep_time_bar.selected))
				{
					//Update time enter sleep.
					int32_t new_time = (580 * ((key.touch_x - 15) / 290.0)) + 20;

					if(new_time < 20)
						new_time = 20;
					else if(new_time > 600)
						new_time = 0;//Never enter sleep.

					config.time_to_enter_sleep = new_time;
					if(config.time_to_enter_sleep > 0 && config.time_to_turn_off_lcd > 0 && (config.time_to_enter_sleep < config.time_to_turn_off_lcd))
						config.time_to_turn_off_lcd = config.time_to_enter_sleep;

					Sem_set_config(&config);

					sem_sleep_time_bar.selected = true;
				}
				else if (Util_hid_is_pressed(key, sem_800px_mode_button) && !record_request && state.console_model != DEF_SEM_MODEL_OLD2DS)
					sem_800px_mode_button.selected = true;
				else if (Util_hid_is_released(key, sem_800px_mode_button) && !record_request && state.console_model != DEF_SEM_MODEL_OLD2DS && sem_800px_mode_button.selected)
				{
					config.screen_mode = DEF_SEM_SCREEN_MODE_800PX;
					Sem_set_config(&config);
				}
				else if(sem_800px_mode_button.selected && !Util_hid_is_held(key, sem_800px_mode_button))
					sem_800px_mode_button.selected = false;
				else if (Util_hid_is_pressed(key, sem_3d_mode_button) && !record_request
				&& state.console_model != DEF_SEM_MODEL_OLD2DS && state.console_model != DEF_SEM_MODEL_NEW2DSXL)
					sem_3d_mode_button.selected = true;
				else if (Util_hid_is_released(key, sem_3d_mode_button) && !record_request
				&& state.console_model != DEF_SEM_MODEL_OLD2DS && state.console_model != DEF_SEM_MODEL_NEW2DSXL&& sem_3d_mode_button.selected)
				{
					config.screen_mode = DEF_SEM_SCREEN_MODE_3D;
					Sem_set_config(&config);
				}
				else if(sem_3d_mode_button.selected && !Util_hid_is_held(key, sem_3d_mode_button))
					sem_3d_mode_button.selected = false;
				else if (Util_hid_is_pressed(key, sem_400px_mode_button) && !record_request)
					sem_400px_mode_button.selected = true;
				else if (Util_hid_is_released(key, sem_400px_mode_button) && !record_request && sem_400px_mode_button.selected)
				{
					config.screen_mode = DEF_SEM_SCREEN_MODE_400PX;
					Sem_set_config(&config);
				}
				else if(sem_400px_mode_button.selected && !Util_hid_is_held(key, sem_400px_mode_button))
					sem_400px_mode_button.selected = false;
				else if (Util_hid_is_pressed(key, sem_auto_mode_button) && !record_request)
					sem_auto_mode_button.selected = true;
				else if (Util_hid_is_released(key, sem_auto_mode_button) && !record_request && sem_auto_mode_button.selected)
				{
					config.screen_mode = DEF_SEM_SCREEN_MODE_AUTO;
					Sem_set_config(&config);
				}
				else if(sem_auto_mode_button.selected && !Util_hid_is_held(key, sem_auto_mode_button))
					sem_auto_mode_button.selected = false;

				sem_scroll_mode = true;
				if(sem_night_mode_on_button.selected || sem_night_mode_off_button.selected || sem_flash_mode_button.selected
				|| sem_screen_brightness_bar.selected || sem_screen_brightness_slider.selected || sem_screen_off_time_bar.selected
				|| sem_screen_off_time_slider.selected || sem_sleep_time_bar.selected || sem_sleep_time_slider.selected
				|| sem_800px_mode_button.selected || sem_3d_mode_button.selected || sem_400px_mode_button.selected
				|| sem_auto_mode_button.selected || Draw_get_bot_ui_button()->selected)
					sem_scroll_mode = false;
			}
			else if (sem_selected_menu_mode == DEF_SEM_MENU_CONTROL)//Scroll speed
			{
				if (Util_hid_is_pressed(key, sem_scroll_speed_slider) || Util_hid_is_pressed(key, sem_scroll_speed_bar)
				|| (key.h_touch && sem_scroll_speed_bar.selected))
				{
					//Update time to turn off LCD.
					double new_speed = (1.95 * ((key.touch_x - 15) / 290.0)) + 0.05;

					if(new_speed < 0.05)
						new_speed = 0.05;
					else if(new_speed > 2)
						new_speed = 2;

					config.scroll_speed = new_speed;
					Sem_set_config(&config);

					sem_scroll_speed_bar.selected = true;
				}
			}
			else if (sem_selected_menu_mode == DEF_SEM_MENU_FONT)//Font
			{
				if (Util_hid_is_pressed(key, sem_load_all_ex_font_button) && !Exfont_is_loading_external_font() && !Exfont_is_unloading_external_font())
					sem_load_all_ex_font_button.selected = true;
				else if (Util_hid_is_released(key, sem_load_all_ex_font_button) && !Exfont_is_loading_external_font() && !Exfont_is_unloading_external_font() && sem_load_all_ex_font_button.selected)
				{
					for (uint16_t i = 0; i < DEF_EXFONT_NUM_OF_FONT_NAME; i++)
						Exfont_set_external_font_request_state(i ,true);

					Exfont_request_load_external_font();
				}
				else if (Util_hid_is_pressed(key, sem_unload_all_ex_font_button) && !Exfont_is_loading_external_font() && !Exfont_is_unloading_external_font())
					sem_unload_all_ex_font_button.selected = true;
				else if (Util_hid_is_released(key, sem_unload_all_ex_font_button) && !Exfont_is_loading_external_font() && !Exfont_is_unloading_external_font() && sem_unload_all_ex_font_button.selected)
				{
					for (uint16_t i = 1; i < DEF_EXFONT_NUM_OF_FONT_NAME; i++)
						Exfont_set_external_font_request_state(i ,false);

					Exfont_request_unload_external_font();
				}
				else
				{
					for (uint16_t i = 0; i < DEF_EXFONT_NUM_OF_FONT_NAME; i++)
					{
						if (Util_hid_is_pressed(key, sem_ex_font_button[i]) && !Exfont_is_loading_external_font() && !Exfont_is_unloading_external_font())
						{
							sem_ex_font_button[i].selected = true;
							break;
						}
						else if (Util_hid_is_released(key, sem_ex_font_button[i]) && !Exfont_is_loading_external_font() && !Exfont_is_unloading_external_font() && sem_ex_font_button[i].selected)
						{
							if (Exfont_is_loaded_external_font(i))
							{
								if(i != 0)
								{
									Exfont_set_external_font_request_state(i ,false);
									Exfont_request_unload_external_font();
								}
							}
							else
							{
								Exfont_set_external_font_request_state(i ,true);
								Exfont_request_load_external_font();
							}
							break;
						}
						else if(sem_ex_font_button[i].selected && !Util_hid_is_held(key, sem_ex_font_button[i]))
							sem_ex_font_button[i].selected = false;
					}
				}

				sem_scroll_mode = true;
				if(sem_load_all_ex_font_button.selected || sem_unload_all_ex_font_button.selected || Draw_get_bot_ui_button()->selected)
					sem_scroll_mode = false;

				for (uint16_t i = 0; i < DEF_EXFONT_NUM_OF_FONT_NAME; i++)
				{
					if(sem_ex_font_button[i].selected)
						sem_scroll_mode = false;
				}
			}
			else if (sem_selected_menu_mode == DEF_SEM_MENU_WIFI)//Wireless
			{
				if (Util_hid_is_pressed(key, sem_wifi_on_button))
					sem_wifi_on_button.selected = true;
				else if (Util_hid_is_released(key, sem_wifi_on_button) && sem_wifi_on_button.selected)
					sem_should_wifi_enabled = true;
				else if (Util_hid_is_pressed(key, sem_wifi_off_button))
					sem_wifi_off_button.selected = true;
				else if (Util_hid_is_released(key, sem_wifi_off_button) && sem_wifi_off_button.selected)
					sem_should_wifi_enabled = false;
			}
			else if (sem_selected_menu_mode == DEF_SEM_MENU_ADVANCED)//Advanced settings
			{
				if (Util_hid_is_pressed(key, sem_allow_send_info_button))
					sem_allow_send_info_button.selected = true;
				else if (Util_hid_is_released(key, sem_allow_send_info_button) && sem_allow_send_info_button.selected)
				{
					config.is_send_info_allowed = true;
					Sem_set_config(&config);
				}
				else if (Util_hid_is_pressed(key, sem_deny_send_info_button))
					sem_deny_send_info_button.selected = true;
				else if (Util_hid_is_released(key, sem_deny_send_info_button) && sem_deny_send_info_button.selected)
				{
					config.is_send_info_allowed = false;
					Sem_set_config(&config);
				}
				else if (Util_hid_is_pressed(key, sem_debug_mode_on_button))
					sem_debug_mode_on_button.selected = true;
				else if (Util_hid_is_released(key, sem_debug_mode_on_button) && sem_debug_mode_on_button.selected)
				{
					config.is_debug = true;
					Sem_set_config(&config);
				}
				else if (Util_hid_is_pressed(key, sem_debug_mode_off_button))
					sem_debug_mode_off_button.selected = true;
				else if (Util_hid_is_released(key, sem_debug_mode_off_button) && sem_debug_mode_off_button.selected)
				{
					config.is_debug = false;
					Sem_set_config(&config);
				}
				else if (Util_hid_is_pressed(key, sem_use_fake_model_button))
					sem_use_fake_model_button.selected = true;
				else if (Util_hid_is_released(key, sem_use_fake_model_button) && sem_use_fake_model_button.selected)
				{
					if(sem_internal_state.fake_model == 255)
						sem_internal_state.fake_model = 0;
					else if((sem_internal_state.fake_model + 1) >= DEF_SEM_MODEL_MAX)
						sem_internal_state.fake_model = 255;//OFF.
					else
						sem_internal_state.fake_model++;

					Draw_set_refresh_needed(true);
				}
				else if(Util_hid_is_pressed(key, sem_dump_log_button) && !sem_dump_log_request)
					sem_dump_log_button.selected = true;
				else if(Util_hid_is_released(key, sem_dump_log_button) && sem_dump_log_button.selected && !sem_dump_log_request)
					sem_dump_log_request = true;
#if DEF_CPU_USAGE_API_ENABLE
				else if (Util_hid_is_pressed(key, sem_monitor_cpu_usage_on_button))
					sem_monitor_cpu_usage_on_button.selected = true;
				else if (Util_hid_is_released(key, sem_monitor_cpu_usage_on_button) && sem_monitor_cpu_usage_on_button.selected)
					sem_should_cpu_usage_monitor_running = true;
				else if (Util_hid_is_pressed(key, sem_monitor_cpu_usage_off_button))
					sem_monitor_cpu_usage_off_button.selected = true;
				else if (Util_hid_is_released(key, sem_monitor_cpu_usage_off_button) && sem_monitor_cpu_usage_off_button.selected)
					sem_should_cpu_usage_monitor_running = false;
#endif
			}
			else if (sem_selected_menu_mode == DEF_SEM_MENU_BATTERY)//Battery
			{
				if (Util_hid_is_pressed(key, sem_eco_mode_on_button))
					sem_eco_mode_on_button.selected = true;
				else if (Util_hid_is_released(key, sem_eco_mode_on_button) && sem_eco_mode_on_button.selected)
				{
					config.is_eco = true;
					Sem_set_config(&config);
				}
				else if (Util_hid_is_pressed(key, sem_eco_mode_off_button))
					sem_eco_mode_off_button.selected = true;
				else if (Util_hid_is_released(key, sem_eco_mode_off_button) && sem_eco_mode_off_button.selected)
				{
					config.is_eco = false;
					Sem_set_config(&config);
				}
			}
#if (DEF_ENCODER_VIDEO_AUDIO_API_ENABLE && DEF_CONVERTER_SW_API_ENABLE && DEF_SEM_ENABLE_SCREEN_RECORDER)
			else if (sem_selected_menu_mode == DEF_SEM_MENU_RECORDING)//Screen recording
			{
				bool can_record = (config.screen_mode == DEF_SEM_SCREEN_MODE_400PX || config.screen_mode == DEF_SEM_SCREEN_MODE_3D);

				if (Util_hid_is_pressed(key, sem_record_both_lcd_button) && can_record)
					sem_record_both_lcd_button.selected = true;
				else if (Util_hid_is_released(key, sem_record_both_lcd_button) && can_record && sem_record_both_lcd_button.selected)
				{
					if(sem_record_request)
						sem_stop_record_request = true;
					else
					{
						sem_selected_recording_mode = DEF_SEM_RECORD_BOTH;
						sem_record_request = true;
					}
				}
				else if (Util_hid_is_pressed(key, sem_record_top_lcd_button) && can_record)
					sem_record_top_lcd_button.selected = true;
				else if (Util_hid_is_released(key, sem_record_top_lcd_button) && can_record && sem_record_top_lcd_button.selected)
				{
					if(sem_record_request)
						sem_stop_record_request = true;
					else
					{
						sem_selected_recording_mode = DEF_SEM_RECORD_TOP;
						sem_record_request = true;
					}
				}
				else if (Util_hid_is_pressed(key, sem_record_bottom_lcd_button) && can_record)
					sem_record_bottom_lcd_button.selected = true;
				else if (Util_hid_is_released(key, sem_record_bottom_lcd_button) && can_record && sem_record_bottom_lcd_button.selected)
				{
					if(sem_record_request)
						sem_stop_record_request = true;
					else
					{
						sem_selected_recording_mode = DEF_SEM_RECORD_BOTTOM;
						sem_record_request = true;
					}
				}
			}
#endif
		}

		//Scroll bar
		if(sem_scroll_mode)
		{
			if (key.h_c_down || key.h_c_up)
				sem_y_offset += (double)key.cpad_y * config.scroll_speed * 0.0625;

			if (key.h_touch && sem_scroll_bar.selected)
				sem_y_offset = ((key.touch_y - 15.0) / 195.0) * sem_y_max;

			if (Util_hid_is_pressed(key, sem_scroll_bar))
				sem_scroll_bar.selected = true;

			if(sem_touch_y_move_left * config.scroll_speed != 0)
				sem_y_offset -= sem_touch_y_move_left * config.scroll_speed;
		}

		if (sem_y_offset >= 0)
			sem_y_offset = 0.0;
		else if (sem_y_offset <= sem_y_max)
			sem_y_offset = sem_y_max;

		if (key.p_touch || key.h_touch)
		{
			sem_touch_x_move_left = 0;
			sem_touch_y_move_left = 0;

			if(sem_scroll_mode)
			{
				sem_touch_x_move_left = key.touch_x_move;
				sem_touch_y_move_left = key.touch_y_move;
			}
		}
		else
		{
			sem_scroll_mode = false;

			sem_back_button.selected = sem_scroll_bar.selected = sem_english_button.selected = sem_japanese_button.selected
			= sem_hungarian_button.selected = sem_chinese_button.selected = sem_italian_button.selected = sem_spanish_button.selected
			= sem_romanian_button.selected = sem_polish_button.selected = sem_ryukyuan_button.selected = sem_night_mode_on_button.selected
			= sem_night_mode_off_button.selected = sem_flash_mode_button.selected = sem_screen_brightness_bar.selected = sem_screen_off_time_bar.selected
			= sem_sleep_time_bar.selected = sem_800px_mode_button.selected = sem_3d_mode_button.selected = sem_400px_mode_button.selected
			= sem_auto_mode_button.selected = sem_scroll_speed_bar.selected = sem_wifi_on_button.selected = sem_wifi_off_button.selected
			= sem_allow_send_info_button.selected = sem_deny_send_info_button.selected = sem_debug_mode_on_button.selected = sem_debug_mode_off_button.selected
			= sem_eco_mode_on_button.selected = sem_eco_mode_off_button.selected = sem_record_both_lcd_button.selected = sem_record_top_lcd_button.selected
			= sem_record_bottom_lcd_button.selected = sem_load_all_ex_font_button.selected = sem_unload_all_ex_font_button.selected
			= sem_use_fake_model_button.selected = sem_dump_log_button.selected = false;

#if ((DEF_CURL_API_ENABLE || DEF_HTTPC_API_ENABLE) && DEF_SEM_ENABLE_UPDATER)
			sem_check_update_button.selected = sem_close_updater_button.selected = sem_select_edtion_button.selected
			= sem_3dsx_button.selected = sem_cia_button.selected = sem_back_to_patch_note_button.selected
			= sem_dl_install_button.selected = sem_close_app_button.selected = false;
#endif

#if DEF_CPU_USAGE_API_ENABLE
			sem_monitor_cpu_usage_on_button.selected = sem_monitor_cpu_usage_off_button.selected = false;
#endif

			Draw_get_bot_ui_button()->selected = false;

			for (uint8_t i = 0; i < 9; i++)
				sem_menu_button[i].selected = false;

			for (uint16_t i = 0; i < DEF_EXFONT_NUM_OF_FONT_NAME; i++)
				sem_ex_font_button[i].selected = false;

			sem_touch_x_move_left -= (sem_touch_x_move_left * 0.025);
			sem_touch_y_move_left -= (sem_touch_y_move_left * 0.025);
			if (sem_touch_x_move_left < 0.5 && sem_touch_x_move_left > -0.5)
				sem_touch_x_move_left = 0;
			if (sem_touch_y_move_left < 0.5 && sem_touch_y_move_left > -0.5)
				sem_touch_y_move_left = 0;
		}
	}

	if(Util_log_query_log_show_flag())
		Util_log_main(key);
}

static void Sem_get_system_info(void)
{
	uint8_t is_charging = 0;
	uint32_t result = DEF_ERR_OTHER;
	char* ssid = (char*)malloc(512);
	time_t unix_time = time(NULL);
	struct tm* time = gmtime((const time_t*)&unix_time);
	Sem_config config = { 0, };
	Sem_state state = { 0, };

	Sem_get_config(&config);
	Sem_get_state(&state);

	PTMU_GetBatteryChargeState(&is_charging);//Battery charge.
	state.is_charging = (bool)is_charging;

	result = MCUHWC_GetBatteryLevel(&state.battery_level);//Battery level(%).
	if(result == DEF_SUCCESS)
	{
		uint8_t battery_voltage = 0;

		MCUHWC_GetBatteryVoltage(&battery_voltage);
		MCUHWC_ReadRegister(0x0A, &state.battery_temp, 1);
		state.battery_voltage = 5.0 * (battery_voltage / 256.0);
	}
	else
	{
		uint8_t ptmu_battery_level = 0;

		PTMU_GetBatteryLevel(&ptmu_battery_level);
		if (ptmu_battery_level == 0)
			state.battery_level = 0;
		else if (ptmu_battery_level == 1)
			state.battery_level = 5;
		else if (ptmu_battery_level == 2)
			state.battery_level = 10;
		else if (ptmu_battery_level == 3)
			state.battery_level = 30;
		else if (ptmu_battery_level == 4)
			state.battery_level = 60;
		else if (ptmu_battery_level == 5)
			state.battery_level = 100;
	}

	//Connected SSID.
	memset(state.connected_wifi, 0x00, sizeof(state.connected_wifi));
	result = ACU_GetSSID(ssid);
	if(result == DEF_SUCCESS)
	{
		uint8_t length = Util_min(strlen(ssid), (sizeof(state.connected_wifi) - 1));
		memcpy(state.connected_wifi, ssid, length);
	}

	free(ssid);
	ssid = NULL;

	state.wifi_signal = osGetWifiStrength();
	//Get wifi state from shared memory #0x1FF81067.
	sem_internal_state.wifi_state = *(uint8_t*)0x1FF81067;
	if(sem_internal_state.wifi_state == 2)
	{
#if (DEF_CURL_API_ENABLE || DEF_HTTPC_API_ENABLE)
		if (!sem_internal_state.is_connect_test_succes)
			state.wifi_signal += 4;//Without Internet access.
#endif
	}
	else
	{
		state.wifi_signal = DEF_SEM_WIFI_SIGNAL_DISABLED;
		sem_internal_state.is_connect_test_succes = false;
	}

	//Get time.
	state.time.years = time->tm_year + 1900;
	state.time.months = time->tm_mon + 1;
	state.time.days = time->tm_mday;
	state.time.hours = time->tm_hour;
	state.time.minutes = time->tm_min;
	state.time.seconds = time->tm_sec;

	if (config.is_debug)
	{
		//Check for free RAM.
		state.free_ram = Util_check_free_ram();
		state.free_linear_ram = Util_check_free_linear_space();
	}

	snprintf(state.msg, sizeof(state.msg), "%02" PRIu32 "fps %04" PRIu16 "/%02" PRIu8 "/%02" PRIu8 " %02" PRIu8 ":%02" PRIu8 ":%02" PRIu8 " ",
	(uint32_t)Draw_query_fps(), state.time.years, state.time.months, state.time.days, state.time.hours, state.time.minutes, state.time.seconds);

	LightLock_Lock(&sem_config_state_mutex);
	sem_state = state;
	LightLock_Unlock(&sem_config_state_mutex);
}

#if (DEF_ENCODER_VIDEO_AUDIO_API_ENABLE && DEF_CONVERTER_SW_API_ENABLE && DEF_SEM_ENABLE_SCREEN_RECORDER)

void Sem_encode_thread(void* arg)
{
	DEF_LOG_STRING("Thread started.");

	while (sem_thread_run)
	{
		while(sem_record_request)
		{
			if(sem_encode_request)
			{
				uint8_t* yuv420p = NULL;
				uint32_t result = DEF_ERR_OTHER;

				sem_wait_request = true;
				sem_encode_request = false;
				yuv420p = (uint8_t*)linearAlloc(sem_rec_width * sem_rec_height * 1.5);
				if(yuv420p == NULL)
					sem_stop_record_request = true;
				else
				{
					memcpy(yuv420p, sem_yuv420p, sem_rec_width * sem_rec_height * 1.5);

					result = Util_encoder_video_encode(yuv420p, 0);
					if(result != DEF_SUCCESS)
					{
						DEF_LOG_RESULT(Util_encoder_video_encode, false, result);
						break;
					}
				}

				free(yuv420p);
				yuv420p = NULL;
				sem_wait_request = false;
			}
			else
				Util_sleep(1000);
		}

		Util_sleep(DEF_THREAD_ACTIVE_SLEEP_TIME);
	}

	DEF_LOG_STRING("Thread exit.");
	threadExit(0);
}

void Sem_record_thread(void* arg)
{
	DEF_LOG_STRING("Thread started.");
	bool new_3ds = false;
	uint8_t mode = 0;
	uint8_t rec_framerate = 10;
	uint8_t* top_framebuffer = NULL;
	uint8_t* bot_framebuffer = NULL;
	uint8_t* top_bgr = NULL;
	uint8_t* bot_bgr = NULL;
	uint8_t* both_bgr = NULL;
	uint16_t rec_width = 400;
	uint16_t rec_height = 480;
	uint16_t width = 0;
	uint16_t height = 0;
	uint32_t offset = 0;
	uint32_t bot_bgr_offset = 0;
	uint32_t new_width = 0;
	uint32_t new_height = 0;
	double time = 0;
	TickCounter counter;
	APT_CheckNew3DS(&new_3ds);
	osTickCounterStart(&counter);

	while (sem_thread_run)
	{
		if (sem_record_request)
		{
			uint32_t result = DEF_ERR_OTHER;
			std::string file_path = "";
			Sem_state state = { 0, };

			Sem_get_state(&state);

			if(DEF_SEM_MODEL_IS_NEW(state.console_model))
				APT_SetAppCpuTimeLimit(80);
			else
				APT_SetAppCpuTimeLimit(70);

			mode = sem_selected_recording_mode;
			if(mode == DEF_SEM_RECORD_BOTH)
			{
				rec_width = 400;
				rec_height = 480;
				if(new_3ds)
					rec_framerate = 15;
				else
					rec_framerate = 5;
			}
			else if(mode == DEF_SEM_RECORD_TOP)
			{
				rec_width = 400;
				rec_height = 240;
				if(new_3ds)
					rec_framerate = 30;
				else
					rec_framerate = 10;
			}
			else if(mode == DEF_SEM_RECORD_BOTTOM)
			{
				rec_width = 320;
				rec_height = 240;
				if(new_3ds)
					rec_framerate = 30;
				else
					rec_framerate = 10;
			}
			sem_rec_width = rec_width;
			sem_rec_height = rec_height;
			file_path = (std::string)DEF_MENU_MAIN_DIR + "screen_recording/" + std::to_string(state.time.years) + "_"
			+ std::to_string(state.time.months) + "_" + std::to_string(state.time.days) + "_" + std::to_string(state.time.hours)
			+ "_" + std::to_string(state.time.minutes) + "_" + std::to_string(state.time.seconds) + ".mp4";

			DEF_LOG_RESULT_SMART(result, Util_encoder_create_output_file(file_path.c_str(), 0), (result == DEF_SUCCESS), result);
			if(result != DEF_SUCCESS)
				sem_record_request = false;

			DEF_LOG_RESULT_SMART(result, Util_encoder_video_init(MEDIA_V_CODEC_MJPEG, rec_width, rec_height, 1500000, rec_framerate, 0), (result == DEF_SUCCESS), result);
			if(result != DEF_SUCCESS)
				sem_record_request = false;

			DEF_LOG_RESULT_SMART(result, Util_encoder_write_header(0), (result == DEF_SUCCESS), result);
			if(result != DEF_SUCCESS)
				sem_record_request = false;

			sem_yuv420p = (uint8_t*)linearAlloc(rec_width * rec_height * 1.5);
			if(sem_yuv420p == NULL)
				sem_stop_record_request = true;

			while(sem_record_request)
			{
				Converter_color_parameters parameters = { 0, };

				if(sem_stop_record_request)
					break;

				osTickCounterUpdate(&counter);

				if(mode == DEF_SEM_RECORD_BOTH)
				{
					top_framebuffer = gfxGetFramebuffer(GFX_TOP, GFX_LEFT, &width, &height);
					result = Util_converter_rgb888_rotate_90_degree(top_framebuffer, &top_bgr, width, height, &new_width, &new_height);
					if(result != DEF_SUCCESS)
					{
						DEF_LOG_RESULT(Util_converter_rgb888_rotate_90_degree, false, result);
						break;
					}

					bot_framebuffer = gfxGetFramebuffer(GFX_BOTTOM, GFX_LEFT, &width, &height);
					result = Util_converter_rgb888_rotate_90_degree(bot_framebuffer, &bot_bgr, width, height, &new_width, &new_height);
					if(result != DEF_SUCCESS)
					{
						DEF_LOG_RESULT(Util_converter_rgb888_rotate_90_degree, false, result);
						break;
					}

					both_bgr = (uint8_t*)linearAlloc(rec_width * rec_height * 3);
					if(both_bgr == NULL)
						break;

					memcpy(both_bgr, top_bgr, 400 * 240 * 3);
					free(top_bgr);
					top_bgr = NULL;

					offset = 400 * 240 * 3;
					bot_bgr_offset = 0;

					for(uint16_t i = 0; i < 240; i++)
					{
						memset(both_bgr + offset, 0x0, 40 * 3);
						offset += 40 * 3;
						memcpy(both_bgr + offset, bot_bgr + bot_bgr_offset, 320 * 3);
						offset += 320 * 3;
						bot_bgr_offset += 320 * 3;
						memset(both_bgr + offset, 0x0, 40 * 3);
						offset += 40 * 3;
					}
					free(bot_bgr);
					bot_bgr = NULL;
				}
				else if(mode == DEF_SEM_RECORD_TOP)
				{
					top_framebuffer = gfxGetFramebuffer(GFX_TOP, GFX_LEFT, &width, &height);
					result = Util_converter_rgb888_rotate_90_degree(top_framebuffer, &both_bgr, width, height, &new_width, &new_height);
					if(result != DEF_SUCCESS)
					{
						DEF_LOG_RESULT(Util_converter_rgb888_rotate_90_degree, false, result);
						break;
					}
				}
				else if(mode == DEF_SEM_RECORD_BOTTOM)
				{
					bot_framebuffer = gfxGetFramebuffer(GFX_BOTTOM, GFX_LEFT, &width, &height);
					result = Util_converter_rgb888_rotate_90_degree(bot_framebuffer, &both_bgr, width, height, &new_width, &new_height);
					if(result != DEF_SUCCESS)
					{
						DEF_LOG_RESULT(Util_converter_rgb888_rotate_90_degree, false, result);
						break;
					}
				}

				parameters.converted = NULL;
				parameters.in_color_format = RAW_PIXEL_BGR888;
				parameters.in_height = rec_height;
				parameters.in_width = rec_width;
				parameters.out_color_format = RAW_PIXEL_YUV420P;
				parameters.out_height = rec_height;
				parameters.out_width = rec_width;
				parameters.source = both_bgr;

				result = Util_converter_convert_color(&parameters);
				free(both_bgr);
				both_bgr = NULL;
				if(result != DEF_SUCCESS)
				{
					DEF_LOG_RESULT(Util_converter_convert_color, false, result);
					break;
				}
				memcpy(sem_yuv420p, parameters.converted, rec_width * rec_height * 1.5);
				free(parameters.converted);
				parameters.converted = NULL;

				sem_encode_request = true;
				osTickCounterUpdate(&counter);
				time = osTickCounterRead(&counter);
				if(1000.0 / rec_framerate > time)
					Util_sleep(((1000.0 / rec_framerate) - time) * 1000);
			}

			while(sem_wait_request)
				Util_sleep(100000);

			Util_encoder_close_output_file(0);
			free(both_bgr);
			free(bot_bgr);
			free(top_bgr);
			free(sem_yuv420p);
			both_bgr = NULL;
			bot_bgr = NULL;
			top_bgr = NULL;
			sem_yuv420p = NULL;
			sem_record_request = false;
			sem_stop_record_request = false;
			Draw_set_refresh_needed(true);
			APT_SetAppCpuTimeLimit(30);
		}
		else
			Util_sleep(DEF_THREAD_ACTIVE_SLEEP_TIME);
	}

	DEF_LOG_STRING("Thread exit.");
	threadExit(0);
}

#endif

void Sem_worker_callback(void)
{
	uint32_t result = DEF_ERR_OTHER;

	if (sem_already_init)
	{
		if (sem_reload_msg_request)
		{
			Sem_config config = { 0, };

			Sem_get_config(&config);

			//Try to load specified language messages, if it fails
			//(i.e. no translation available), load English messags.
			DEF_LOG_RESULT_SMART(result, Sem_load_msg(config.lang), (result == DEF_SUCCESS), result);
			if (result != DEF_SUCCESS)
				DEF_LOG_RESULT_SMART(result, Sem_load_msg("en"), (result == DEF_SUCCESS), result);

			DEF_LOG_RESULT_SMART(result, Menu_load_msg(config.lang), (result == DEF_SUCCESS), result);
			if (result != DEF_SUCCESS)
				DEF_LOG_RESULT_SMART(result, Menu_load_msg("en"), (result == DEF_SUCCESS), result);

			#ifdef DEF_SAPP0_ENABLE
			DEF_LOG_RESULT_SMART(result, Sapp0_load_msg(config.lang), (result == DEF_SUCCESS), result);
			if (result != DEF_SUCCESS)
				DEF_LOG_RESULT_SMART(result, Sapp0_load_msg("en"), (result == DEF_SUCCESS), result);
			#endif

			#ifdef DEF_SAPP1_ENABLE
			DEF_LOG_RESULT_SMART(result, Sapp1_load_msg(config.lang), (result == DEF_SUCCESS), result);
			if (result != DEF_SUCCESS)
				DEF_LOG_RESULT_SMART(result, Sapp1_load_msg("en"), (result == DEF_SUCCESS), result);
			#endif

			#ifdef DEF_SAPP2_ENABLE
			DEF_LOG_RESULT_SMART(result, Sapp2_load_msg(config.lang), (result == DEF_SUCCESS), result);
			if (result != DEF_SUCCESS)
				DEF_LOG_RESULT_SMART(result, Sapp2_load_msg("en"), (result == DEF_SUCCESS), result);
			#endif

			#ifdef DEF_SAPP3_ENABLE
			DEF_LOG_RESULT_SMART(result, Sapp3_load_msg(config.lang), (result == DEF_SUCCESS), result);
			if (result != DEF_SUCCESS)
				DEF_LOG_RESULT_SMART(result, Sapp3_load_msg("en"), (result == DEF_SUCCESS), result);
			#endif

			#ifdef DEF_SAPP4_ENABLE
			DEF_LOG_RESULT_SMART(result, Sapp4_load_msg(config.lang), (result == DEF_SUCCESS), result);
			if (result != DEF_SUCCESS)
				DEF_LOG_RESULT_SMART(result, Sapp4_load_msg("en"), (result == DEF_SUCCESS), result);
			#endif

			#ifdef DEF_SAPP5_ENABLE
			DEF_LOG_RESULT_SMART(result, Sapp5_load_msg(config.lang), (result == DEF_SUCCESS), result);
			if (result != DEF_SUCCESS)
				DEF_LOG_RESULT_SMART(result, Sapp5_load_msg("en"), (result == DEF_SUCCESS), result);
			#endif

			#ifdef DEF_SAPP6_ENABLE
			DEF_LOG_RESULT_SMART(result, Sapp6_load_msg(config.lang), (result == DEF_SUCCESS), result);
			if (result != DEF_SUCCESS)
				DEF_LOG_RESULT_SMART(result, Sapp6_load_msg("en"), (result == DEF_SUCCESS), result);
			#endif

			#ifdef DEF_SAPP7_ENABLE
			DEF_LOG_RESULT_SMART(result, Sapp7_load_msg(config.lang), (result == DEF_SUCCESS), result);
			if (result != DEF_SUCCESS)
				DEF_LOG_RESULT_SMART(result, Sapp7_load_msg("en"), (result == DEF_SUCCESS), result);
			#endif

			sem_reload_msg_request = false;
			Draw_set_refresh_needed(true);
		}
#if DEF_CPU_USAGE_API_ENABLE
		else if(sem_is_cpu_usage_monitor_running != sem_should_cpu_usage_monitor_running)
		{
			if(sem_should_cpu_usage_monitor_running)
			{
				DEF_LOG_RESULT_SMART(result, Util_cpu_usage_init(), (result == DEF_SUCCESS), result);
				if(result == DEF_SUCCESS)
				{
					Util_cpu_usage_set_show_flag(true);
					sem_is_cpu_usage_monitor_running = true;
				}
				else
				{
					Util_err_set_error_message(Util_err_get_error_msg(result), "", DEF_LOG_GET_FUNCTION_NAME(), result);
					Util_err_set_error_show_flag(true);
					sem_should_cpu_usage_monitor_running = false;
				}
			}
			else
			{
				Util_cpu_usage_set_show_flag(false);
				Util_cpu_usage_exit();
				sem_is_cpu_usage_monitor_running = false;
			}
		}
#endif
		else if(sem_dump_log_request)
		{
			char file_name[64] = { 0, };
			char dir_name[64] = { 0, };
			Sem_state state = { 0, };

			Sem_get_state(&state);

			snprintf(file_name, sizeof(file_name), "%04" PRIu16 "_%02" PRIu8 "_%02" PRIu8 "_%02" PRIu8 "_%02" PRIu8 "_%02" PRIu8 ".txt",
			state.time.years, state.time.months, state.time.days, state.time.hours, state.time.minutes, state.time.seconds);
			snprintf(dir_name, sizeof(dir_name), "%slogs/", DEF_MENU_MAIN_DIR);

			DEF_LOG_RESULT_SMART(result, Util_log_dump(file_name, dir_name), (result == DEF_SUCCESS), result);
			if(result == DEF_SUCCESS)
				DEF_LOG_FORMAT("Log file was dumped at : %s%s", dir_name, file_name);

			sem_dump_log_request = false;
		}
	}
}

void Sem_hw_config_thread(void* arg)
{
	DEF_LOG_STRING("Thread started.");
	uint32_t count = 0;
	uint64_t previous_ts = 0;

	while (sem_thread_run)
	{
		uint32_t result = DEF_ERR_OTHER;
		Sem_config config = { 0, };
		Hid_info hid_info = { 0, };

		Sem_get_config(&config);
		Util_hid_query_key_state(&hid_info);

		if(previous_ts + 50 <= osGetTime())
		{
			if(config.is_flash)
			{
				config.is_night = !config.is_night;
				Sem_set_config(&config);

				Draw_set_refresh_needed(true);
			}
			count++;

			if(previous_ts + 100 >= osGetTime())
				previous_ts += 50;
			else
				previous_ts = osGetTime();
		}

		if(count % 5 == 0)
			Sem_get_system_info();

		if(count >= 20)
		{
			Draw_set_refresh_needed(true);
			count = 0;
		}

		if(sem_should_wifi_enabled != config.is_wifi_on)
		{
			result = Util_hw_config_set_wifi_state(sem_should_wifi_enabled);
			if(result == DEF_SUCCESS || result == 0xC8A06C0D)//0xC8A06C0D means "already requested state".
			{
				//We need to directly update wifi state here, otherwise
				//(i.e. with Sem_set_config()) it'll try to send a request again.
				LightLock_Lock(&sem_config_state_mutex);
				sem_config.is_wifi_on = sem_should_wifi_enabled;
				LightLock_Unlock(&sem_config_state_mutex);
			}
			else
				sem_should_wifi_enabled = !sem_should_wifi_enabled;
		}

		//If config.time_to_turn_off_lcd == 0, it means automatic turn off LCD feature has been disabled.
		if(config.time_to_turn_off_lcd > 0 && (hid_info.afk_time_ms / 1000) > config.time_to_turn_off_lcd)
		{
			result = Util_hw_config_set_screen_state(true, true, false);
			if(result != DEF_SUCCESS)
				DEF_LOG_RESULT(Util_hw_config_set_screen_state, false, result);
		}
		else if(config.time_to_turn_off_lcd > 0 && (hid_info.afk_time_ms / 1000) > (uint16_t)(config.time_to_turn_off_lcd - 10))
		{
			result = Util_hw_config_set_screen_brightness(true, true, 10);
			if(result != DEF_SUCCESS)
				DEF_LOG_RESULT(Util_hw_config_set_screen_brightness, false, result);
		}
		else
		{
			result = Util_hw_config_set_screen_state(true, false, config.is_top_lcd_on);
			if(result != DEF_SUCCESS)
				DEF_LOG_RESULT(Util_hw_config_set_screen_state, false, result);

			result = Util_hw_config_set_screen_state(false, true, config.is_bottom_lcd_on);
			if(result != DEF_SUCCESS)
				DEF_LOG_RESULT(Util_hw_config_set_screen_state, false, result);

			if(config.top_lcd_brightness == config.bottom_lcd_brightness)
			{
				result = Util_hw_config_set_screen_brightness(true, true, config.top_lcd_brightness);
				if(result != DEF_SUCCESS)
					DEF_LOG_RESULT(Util_hw_config_set_screen_brightness, false, result);
			}
			else
			{
				result = Util_hw_config_set_screen_brightness(true, false, config.top_lcd_brightness);
				if(result != DEF_SUCCESS)
					DEF_LOG_RESULT(Util_hw_config_set_screen_brightness, false, result);

				result = Util_hw_config_set_screen_brightness(false, true, config.bottom_lcd_brightness);
				if(result != DEF_SUCCESS)
					DEF_LOG_RESULT(Util_hw_config_set_screen_brightness, false, result);
			}
		}

		//If config.time_to_enter_sleep == 0, it means automatic sleep feature has been disabled.
		if(config.time_to_enter_sleep > 0 && (hid_info.afk_time_ms / 1000) > config.time_to_enter_sleep)
		{
			result = Util_hw_config_sleep_system((HW_CONFIG_WAKEUP_BIT_OPEN_SHELL | HW_CONFIG_WAKEUP_BIT_PRESS_HOME_BUTTON));
			if(result == DEF_SUCCESS)
			{
				//We woke up from sleep.
				Util_hid_reset_afk_time();
			}
			else
				DEF_LOG_RESULT(Util_hw_config_sleep_system, false, result);
		}

		Util_sleep(DEF_THREAD_ACTIVE_SLEEP_TIME);
	}

	DEF_LOG_STRING("Thread exit.");
	threadExit(0);
}

#if (DEF_CURL_API_ENABLE || DEF_HTTPC_API_ENABLE)
void Sem_check_connectivity_thread(void* arg)
{
	DEF_LOG_STRING("Thread started.");
	uint8_t* http_buffer = NULL;
	uint16_t status_code = 0;
	uint16_t count = 100;

	while (sem_thread_run)
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

			sem_internal_state.is_connect_test_succes = (status_code == 204);
		}
		else
			Util_sleep(DEF_THREAD_ACTIVE_SLEEP_TIME);

		count++;
	}

	DEF_LOG_STRING("Thread exit.");
	threadExit(0);
}
#endif

#if ((DEF_CURL_API_ENABLE || DEF_HTTPC_API_ENABLE) && DEF_SEM_ENABLE_UPDATER)
void Sem_update_thread(void* arg)
{
	DEF_LOG_STRING("Thread started.");

	uint8_t* buffer = NULL;
	uint32_t write_size = 0;
	uint32_t read_size = 0;
	uint32_t result = DEF_ERR_OTHER;
	uint64_t offset = 0;
	size_t parse_start_pos = std::string::npos;
	size_t parse_end_pos = std::string::npos;
	std::string dir_path = "";
	std::string file_name = "";
	std::string url = "";
	std::string parse_cache = "";
	std::string parse_start[6] = {"<newest>", "<3dsx_available>", "<cia_available>", "<3dsx_url>", "<cia_url>", "<patch_note>", };
	std::string parse_end[6] = { "</newest>", "</3dsx_available>", "</cia_available>", "</3dsx_url>", "</cia_url>", "</patch_note>", };
	Handle am_handle = 0;

	while (sem_thread_run)
	{
		if (sem_check_update_request || sem_dl_file_request)
		{
			if (sem_check_update_request)
			{
				sem_update_progress = 0;
				sem_selected_edition_num = DEF_SEM_EDTION_NONE;
				url = DEF_SEM_CHECK_UPDATE_URL;
				sem_new_version_available = false;
				for (uint8_t i = 0; i < 6; i++)
					sem_newest_ver_data[i] = "";
			}
			else if (sem_dl_file_request)
			{
				sem_update_progress = 2;
				url = sem_newest_ver_data[3 + sem_selected_edition_num];
			}
			Draw_set_refresh_needed(true);

			sem_dled_size = 0;
			offset = 0;
			sem_installed_size = 0;
			sem_total_cia_size = 0;

			if(sem_dl_file_request)
			{
				dir_path = DEF_SEM_UPDATE_DIR_PREFIX + sem_newest_ver_data[0] + "/";
				file_name = DEF_SEM_UPDATE_FILE_PREFIX;
				if(sem_selected_edition_num == DEF_SEM_EDTION_3DSX)
					file_name += ".3dsx";
				else if(sem_selected_edition_num == DEF_SEM_EDTION_CIA)
					file_name += ".cia";

				Util_file_delete_file(file_name.c_str(), dir_path.c_str());//Delete old file if exist.
			}

			if(sem_dl_file_request)
			{
#if DEF_CURL_API_ENABLE
				DEF_LOG_RESULT_SMART(result, Util_curl_save_data(url.c_str(), 0x20000, &sem_dled_size, NULL, 5, NULL, dir_path.c_str(), file_name.c_str()), (result == DEF_SUCCESS), result);
#else
				DEF_LOG_RESULT_SMART(result, Util_httpc_save_data(url.c_str(), 0x20000, &sem_dled_size, NULL, 5, NULL, dir_path.c_str(), file_name.c_str()), (result == DEF_SUCCESS), result);
#endif
			}
			else
			{
#if DEF_CURL_API_ENABLE
				DEF_LOG_RESULT_SMART(result, Util_curl_dl_data(url.c_str(), &buffer, 0x20000, &sem_dled_size, NULL, 5, NULL), (result == DEF_SUCCESS), result);
#else
				DEF_LOG_RESULT_SMART(result, Util_httpc_dl_data(url.c_str(), &buffer, 0x20000, &sem_dled_size, NULL, 5, NULL), (result == DEF_SUCCESS), result);
#endif
			}

			if (result != DEF_SUCCESS)
			{
				Util_err_set_error_message(Util_err_get_error_msg(result), "", DEF_LOG_GET_FUNCTION_NAME(), result);
				Util_err_set_error_show_flag(true);
				if (sem_check_update_request)
					sem_update_progress = -1;
				else if (sem_dl_file_request)
					sem_update_progress = -2;
				Draw_set_refresh_needed(true);
			}
			else
			{
				if (sem_check_update_request && buffer)
				{
					parse_cache = (char*)buffer;

					for (uint8_t i = 0; i < 6; i++)
					{
						parse_start_pos = parse_cache.find(parse_start[i]);
						parse_end_pos = parse_cache.find(parse_end[i]);

						parse_start_pos += parse_start[i].length();
						parse_end_pos -= parse_start_pos;
						if (parse_start_pos != std::string::npos && parse_end_pos != std::string::npos)
							sem_newest_ver_data[i] = parse_cache.substr(parse_start_pos, parse_end_pos);
						else
						{
							sem_update_progress = -1;
							break;
						}
					}

					if(sem_update_progress != -1)
					{
						if (DEF_MENU_CURRENT_APP_VER_INT < (uint32_t)atoi(sem_newest_ver_data[0].c_str()))
							sem_new_version_available = true;
						else
							sem_new_version_available = false;

						if(envIsHomebrew() && sem_newest_ver_data[1] == "1")
							sem_selected_edition_num = DEF_SEM_EDTION_3DSX;
						else if(sem_newest_ver_data[2] == "1")
							sem_selected_edition_num = DEF_SEM_EDTION_CIA;
					}

					sem_update_progress = 1;
					Draw_set_refresh_needed(true);
				}
				else if (sem_dl_file_request)
				{
					sem_update_progress = 3;
					if (sem_selected_edition_num == DEF_SEM_EDTION_3DSX)
						sem_update_progress = 4;

					Draw_set_refresh_needed(true);
					if (sem_selected_edition_num == DEF_SEM_EDTION_CIA)
					{
						sem_total_cia_size = sem_dled_size;
						DEF_LOG_RESULT_SMART(result, AM_StartCiaInstall(MEDIATYPE_SD, &am_handle), (result == DEF_SUCCESS), result);

						while (true)
						{
							free(buffer);
							buffer = NULL;

							DEF_LOG_RESULT_SMART(result, Util_file_load_from_file(file_name.c_str(), dir_path.c_str(), &buffer, 0x20000, offset, &read_size), (result == DEF_SUCCESS), result);
							if(result != DEF_SUCCESS || read_size <= 0)
								break;

							DEF_LOG_RESULT_SMART(result, FSFILE_Write(am_handle, &write_size, offset, buffer, read_size, FS_WRITE_FLUSH), (result == DEF_SUCCESS), result);
							if(result != DEF_SUCCESS)
								break;

							offset += write_size;
							sem_installed_size += write_size;
						}

						DEF_LOG_RESULT_SMART(result, AM_FinishCiaInstall(am_handle), (result == DEF_SUCCESS), result);
						if (result == DEF_SUCCESS)
							sem_update_progress = 4;
						else
							sem_update_progress = -2;

						Draw_set_refresh_needed(true);
					}
				}
			}

			free(buffer);
			buffer = NULL;
			if(sem_check_update_request)
				sem_check_update_request = false;
			else if(sem_dl_file_request)
				sem_dl_file_request = false;
		}
		else
			Util_sleep(DEF_THREAD_ACTIVE_SLEEP_TIME);

		while (sem_thread_suspend)
			Util_sleep(DEF_THREAD_INACTIVE_SLEEP_TIME);
	}
	DEF_LOG_STRING("Thread exit.");
	threadExit(0);
}
#endif
