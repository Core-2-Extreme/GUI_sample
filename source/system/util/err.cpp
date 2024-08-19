extern "C"
{
#include "system/util/err.h"
}

#include <stdbool.h>
#include <stdint.h>
#include <time.h>

extern "C"
{
	#include "system/menu.h"
	#include "system/draw/draw.h"
	#include "system/util/file.h"
	#include "system/util/hid.h"
	#include "system/util/log.h"
	#include "system/util/str.h"
}


extern "C"
{
static void Util_err_save_callback(void);


bool util_err_show_flag = false;
bool util_err_save_request = false;
bool util_err_init = false;
Str_data util_err_summary = { 0, };
Str_data util_err_description = { 0, };
Str_data util_err_location = { 0, };
Str_data util_err_code = { 0, };
Draw_image_data util_err_ok_button = { 0, }, util_err_save_button = { 0, };


uint32_t Util_err_init(void)
{
	uint32_t result = DEF_ERR_OTHER;
	Str_data* str_list[] = { &util_err_summary, &util_err_description, &util_err_location, &util_err_code, };

	if(util_err_init)
		goto already_inited;

	util_err_show_flag = false;
	util_err_save_request = false;
	util_err_ok_button = Draw_get_empty_image();
	util_err_save_button = Draw_get_empty_image();

	for(uint8_t i = 0; i < (sizeof(str_list) / sizeof(str_list[0])); i++)
	{
		result = Util_str_init(str_list[i]);
		if(result != DEF_SUCCESS)
		{
			DEF_LOG_RESULT(Util_str_init, false, result);
			goto error_other;
		}
		Util_str_set(str_list[i], "N/A");
	}

	if(!Menu_add_worker_thread_callback(Util_err_save_callback))
	{
		result = DEF_ERR_OTHER;
		DEF_LOG_RESULT(Menu_add_worker_thread_callback, false, result);
		goto error_other;
	}

	util_err_init = true;
	return result;

	already_inited:
	return DEF_ERR_ALREADY_INITIALIZED;

	error_other:
	for(uint8_t i = 0; i < (sizeof(str_list) / sizeof(str_list[0])); i++)
		Util_str_free(str_list[i]);

	Menu_remove_worker_thread_callback(Util_err_save_callback);
	return result;
}

void Util_err_exit(void)
{
	Str_data* str_list[] = { &util_err_summary, &util_err_description, &util_err_location, &util_err_code, };

	if(!util_err_init)
		return;

	util_err_init = false;
	for(uint8_t i = 0; i < (sizeof(str_list) / sizeof(str_list[0])); i++)
		Util_str_free(str_list[i]);

	Menu_remove_worker_thread_callback(Util_err_save_callback);
}

bool Util_err_query_error_show_flag(void)
{
	if(!util_err_init)
		return false;

	return util_err_show_flag;
}

void Util_err_set_error_message(const char* summary, const char* description, const char* location, uint32_t error_code)
{
	if(!util_err_init)
		return;

	if(!summary || !description || !location)
		return;

	Util_err_clear_error_message();
	Util_str_set(&util_err_summary, summary);
	Util_str_set(&util_err_description, description);
	Util_str_set(&util_err_location, location);

	if(error_code == DEF_ERR_NO_RESULT_CODE)
		Util_str_set(&util_err_code, "N/A");
	else
		Util_str_format(&util_err_code, "0x%X", error_code);
}

void Util_err_set_error_show_flag(bool flag)
{
	if(!util_err_init)
		return;

	util_err_show_flag = flag;
}

void Util_err_clear_error_message(void)
{
	Str_data* str_list[] = { &util_err_summary, &util_err_description, &util_err_location, &util_err_code, };

	if(!util_err_init)
		return;

	for(uint8_t i = 0; i < (sizeof(str_list) / sizeof(str_list[0])); i++)
		Util_str_set(str_list[i], "N/A");
}

void Util_err_save_error(void)
{
	if(!util_err_init)
		return;

	util_err_save_request = true;
}

const char* Util_err_get_error_msg(uint32_t result)
{
	if(!util_err_init)
		return "";

	switch (result)
	{
		case DEF_SUCCESS:
			return "";
		case DEF_ERR_OTHER:
			return DEF_ERR_OTHER_STR;
		case DEF_ERR_OUT_OF_MEMORY:
			return DEF_ERR_OUT_OF_MEMORY_STR;
		case DEF_ERR_OUT_OF_LINEAR_MEMORY:
			return DEF_ERR_OUT_OF_LINEAR_MEMORY_STR;
		case DEF_ERR_GAS_RETURNED_NOT_SUCCESS:
			return DEF_ERR_GAS_RETURNED_NOT_SUCCESS_STR;
		case DEF_ERR_STB_IMG_RETURNED_NOT_SUCCESS:
			return DEF_ERR_STB_IMG_RETURNED_NOT_SUCCESS_STR;
		case DEF_ERR_FFMPEG_RETURNED_NOT_SUCCESS:
			return DEF_ERR_FFMPEG_RETURNED_NOT_SUCCESS_STR;
		case DEF_ERR_INVALID_ARG:
			return DEF_ERR_INVALID_ARG_STR;
		case DEF_ERR_JSMN_RETURNED_NOT_SUCCESS:
			return DEF_ERR_JSMN_RETURNED_NOT_SUCCESS_STR;
		case DEF_ERR_TRY_AGAIN:
			return DEF_ERR_TRY_AGAIN_STR;
		case DEF_ERR_ALREADY_INITIALIZED:
			return DEF_ERR_ALREADY_INITIALIZED_STR;
		case DEF_ERR_NOT_INITIALIZED:
			return DEF_ERR_NOT_INITIALIZED_STR;
		case DEF_ERR_CURL_RETURNED_NOT_SUCCESS:
			return DEF_ERR_CURL_RETURNED_NOT_SUCCESS_STR;
		case DEF_ERR_NEED_MORE_INPUT:
			return DEF_ERR_NEED_MORE_INPUT_STR;
		case DEF_ERR_DECODER_TRY_AGAIN_NO_OUTPUT:
			return DEF_ERR_DECODER_TRY_AGAIN_NO_OUTPUT_STR;
		case DEF_ERR_DECODER_TRY_AGAIN:
			return DEF_ERR_DECODER_TRY_AGAIN_STR;
		case DEF_ERR_MBEDTLS_RETURNED_NOT_SUCCESS:
			return DEF_ERR_MBEDTLS_RETURNED_NOT_SUCCESS_STR;
		case DEF_ERR_DISABLED:
			return DEF_ERR_DISABLED_STR;
		default:
			return "Something went wrong (likely Nintendo API error).";
	}
}

void Util_err_main(Hid_info key)
{
	if(!util_err_init)
	{
		if (key.p_a)
		{
			util_err_show_flag = false;
			Draw_set_refresh_needed(true);
		}
		return;
	}

	if(Util_hid_is_pressed(key, util_err_ok_button) && !util_err_save_request)
	{
		util_err_ok_button.selected = true;
		Draw_set_refresh_needed(true);
	}
	else if ((key.p_a || (Util_hid_is_released(key, util_err_ok_button) && util_err_ok_button.selected)) && !util_err_save_request)
	{
		util_err_show_flag = false;
		Draw_set_refresh_needed(true);
	}
	else if(Util_hid_is_pressed(key, util_err_save_button) && !util_err_save_request)
	{
		util_err_save_button.selected = true;
		Draw_set_refresh_needed(true);
	}
	else if ((key.p_x || (Util_hid_is_released(key, util_err_save_button) && util_err_save_button.selected)) && !util_err_save_request)
	{
		Util_err_save_error();
		Draw_set_refresh_needed(true);
	}

	if(!key.p_touch && !key.h_touch)
	{
		if(util_err_ok_button.selected || util_err_save_button.selected)
			Draw_set_refresh_needed(true);

		util_err_ok_button.selected = util_err_save_button.selected = false;
	}
}

void Util_err_draw(void)
{
	Draw_image_data background = Draw_get_empty_image();

	if(!util_err_init)
	{
		Draw_texture(&background, DEF_DRAW_AQUA, 20.0, 30.0, 280.0, 150.0);
		Draw_c("Error api is not initialized.\nPress A to close.", 22.5, 40.0, 0.45, 0.45, DEF_DRAW_RED);
		return;
	}

	Draw_texture(&background, DEF_DRAW_AQUA, 20.0, 30.0, 280.0, 150.0);
	Draw_texture(&util_err_ok_button, util_err_ok_button.selected ? DEF_DRAW_YELLOW : DEF_DRAW_WEAK_YELLOW, 150.0, 150.0, 30.0, 20.0);
	Draw_texture(&util_err_save_button, util_err_save_button.selected ? DEF_DRAW_YELLOW : DEF_DRAW_WEAK_YELLOW, 210.0, 150.0, 40.0, 20.0);

	Draw_c("Summary : ", 22.5, 40.0, 0.45, 0.45, DEF_DRAW_RED);
	Draw(&util_err_summary, 22.5, 50.0, 0.45, 0.45, DEF_DRAW_BLACK);
	Draw_c("Description : ", 22.5, 60.0, 0.45, 0.45, DEF_DRAW_RED);
	Draw(&util_err_description, 22.5, 70.0, 0.4, 0.4, DEF_DRAW_BLACK);
	Draw_c("Location : ", 22.5, 90.0, 0.45, 0.45, DEF_DRAW_RED);
	Draw(&util_err_location, 22.5, 100.0, 0.45, 0.45, DEF_DRAW_BLACK);
	Draw_c("Error code : ", 22.5, 110.0, 0.45, 0.45, DEF_DRAW_RED);
	Draw(&util_err_code, 22.5, 120.0, 0.45, 0.45, DEF_DRAW_BLACK);
	Draw_c("OK(A)", 152.5, 152.5, 0.375, 0.375, util_err_save_request ? DEF_DRAW_WEAK_BLACK : DEF_DRAW_BLACK);
	Draw_c("SAVE(X)", 212.5, 152.5, 0.375, 0.375, util_err_save_request ? DEF_DRAW_WEAK_BLACK : DEF_DRAW_BLACK);
}

static void Util_err_save_callback(void)
{
	if (util_err_init && util_err_save_request)
	{
		uint32_t result = DEF_ERR_OTHER;
		Str_data file_name = { 0, };
		Str_data save_data = { 0, };
		time_t unix_time = time(NULL);
		struct tm* time = gmtime((const time_t*)&unix_time);

		time->tm_year += 1900;
		time->tm_mon += 1;

		result = Util_str_init(&file_name);
		if(result != DEF_SUCCESS)
			DEF_LOG_RESULT(Util_str_init, false, result);

		result = Util_str_init(&save_data);
		if(result != DEF_SUCCESS)
			DEF_LOG_RESULT(Util_str_init, false, result);

		if(Util_str_is_valid(&file_name) && Util_str_is_valid(&save_data))
		{
			Util_str_format(&file_name, "%04" PRIu16 "_%02" PRIu8 "_%02" PRIu8 "_%02" PRIu8 "_%02" PRIu8 "_%02" PRIu8 ".txt",
			time->tm_year, time->tm_mon, time->tm_mday, time->tm_hour, time->tm_min, time->tm_sec);

			Util_str_format(&save_data, "\n\n##ERROR MESSAGE##\n%s\n%s\n%s\n%s\n",
			util_err_summary.buffer, util_err_description.buffer, util_err_location.buffer, util_err_code.buffer);

			result = Util_log_dump(file_name.buffer, (DEF_MENU_MAIN_DIR "error/"));
			if(result != DEF_SUCCESS)
				DEF_LOG_RESULT(Util_log_dump, false, result);

			result = Util_file_save_to_file(file_name.buffer, (DEF_MENU_MAIN_DIR "error/"), (uint8_t*)save_data.buffer , save_data.length, false);
			if(result != DEF_SUCCESS)
				DEF_LOG_RESULT(Util_file_save_to_file, false, result);

			Util_err_set_error_show_flag(false);
			util_err_save_request = false;
		}

		Util_str_free(&file_name);
		Util_str_free(&save_data);
	}
}
}
