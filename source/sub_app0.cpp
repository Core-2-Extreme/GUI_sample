#include "definitions.hpp"
#include "system/types.hpp"

#include "system/menu.hpp"
#include "system/variables.hpp"

#include "system/draw/draw.hpp"

#include "system/util/converter.hpp"
#include "system/util/curl.hpp"
#include "system/util/decoder.hpp"
#include "system/util/error.hpp"
#include "system/util/hid.hpp"
#include "system/util/log.hpp"
#include "system/util/util.hpp"

extern "C"
{
	#include "system/util/str.h"
}

//Include myself.
#include "sub_app0.hpp"


bool sapp0_main_run = false;
bool sapp0_thread_run = false;
bool sapp0_already_init = false;
bool sapp0_thread_suspend = true;
std::string sapp0_msg[DEF_SAPP0_NUM_OF_MSG];
Thread sapp0_init_thread, sapp0_exit_thread, sapp0_worker_thread;
Util_str sapp0_status = { 0, };
Image_data sapp0_image[3];


static void Sapp0_draw_init_exit_message(void);
static void Sapp0_init_thread(void* arg);
static void Sapp0_exit_thread(void* arg);
static void Sapp0_worker_thread(void* arg);


bool Sapp0_query_init_flag(void)
{
	return sapp0_already_init;
}

bool Sapp0_query_running_flag(void)
{
	return sapp0_main_run;
}

void Sapp0_hid(Hid_info key)
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
			Sapp0_suspend();
	}

	if(!key.p_touch && !key.h_touch)
		Draw_get_bot_ui_button()->selected = false;

	if(Util_log_query_log_show_flag())
		Util_log_main(key);
}

void Sapp0_resume(void)
{
	sapp0_thread_suspend = false;
	sapp0_main_run = true;
	var_need_reflesh = true;
	Menu_suspend();
}

void Sapp0_suspend(void)
{
	sapp0_thread_suspend = true;
	sapp0_main_run = false;
	var_need_reflesh = true;
	Menu_resume();
}

Result_with_string Sapp0_load_msg(std::string lang)
{
	return Util_load_msg("sapp0_" + lang + ".txt", sapp0_msg, DEF_SAPP0_NUM_OF_MSG);
}

void Sapp0_init(bool draw)
{
	DEF_LOG_STRING("Initializing...");
	uint32_t result = DEF_ERR_OTHER;

	DEF_LOG_RESULT_SMART(result, Util_str_init(&sapp0_status), (result == DEF_SUCCESS), result);

	Util_add_watch(WATCH_HANDLE_SUB_APP0, &sapp0_status.sequencial_id, sizeof(sapp0_status.sequencial_id));

	if((var_model == CFG_MODEL_N2DSXL || var_model == CFG_MODEL_N3DSXL || var_model == CFG_MODEL_N3DS) && var_core_2_available)
		sapp0_init_thread = threadCreate(Sapp0_init_thread, (void*)(""), DEF_STACKSIZE, DEF_THREAD_PRIORITY_NORMAL, 2, false);
	else
	{
		APT_SetAppCpuTimeLimit(80);
		sapp0_init_thread = threadCreate(Sapp0_init_thread, (void*)(""), DEF_STACKSIZE, DEF_THREAD_PRIORITY_NORMAL, 1, false);
	}

	while(!sapp0_already_init)
	{
		if(draw)
			Sapp0_draw_init_exit_message();
		else
			Util_sleep(20000);
	}

	if(!(var_model == CFG_MODEL_N2DSXL || var_model == CFG_MODEL_N3DSXL || var_model == CFG_MODEL_N3DS) || !var_core_2_available)
		APT_SetAppCpuTimeLimit(10);

	DEF_LOG_RESULT_SMART(result, threadJoin(sapp0_init_thread, DEF_THREAD_WAIT_TIME), result, result);
	threadFree(sapp0_init_thread);

	Util_str_clear(&sapp0_status);
	Sapp0_resume();

	DEF_LOG_STRING("Initialized.");
}

void Sapp0_exit(bool draw)
{
	DEF_LOG_STRING("Exiting...");
	uint32_t result = DEF_ERR_OTHER;

	sapp0_exit_thread = threadCreate(Sapp0_exit_thread, (void*)(""), DEF_STACKSIZE, DEF_THREAD_PRIORITY_NORMAL, 1, false);

	while(sapp0_already_init)
	{
		if(draw)
			Sapp0_draw_init_exit_message();
		else
			Util_sleep(20000);
	}

	DEF_LOG_RESULT_SMART(result, threadJoin(sapp0_exit_thread, DEF_THREAD_WAIT_TIME), result, result);
	threadFree(sapp0_exit_thread);

	Util_remove_watch(WATCH_HANDLE_SUB_APP0, &sapp0_status.sequencial_id);
	Util_str_free(&sapp0_status);
	var_need_reflesh = true;

	DEF_LOG_STRING("Exited.");
}

void Sapp0_main(void)
{
	int color = DEF_DRAW_BLACK;
	int back_color = DEF_DRAW_WHITE;
	Watch_handle_bit watch_handle_bit = (DEF_WATCH_HANDLE_BIT_GLOBAL | DEF_WATCH_HANDLE_BIT_SUB_APP0);

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

			Draw(sapp0_msg[0], 0, 20, 0.5, 0.5, color);

			//Draw texture here.
			if(sapp0_image[0].subtex)
				Draw_texture(&sapp0_image[0], 0, 40, sapp0_image[0].subtex->width, sapp0_image[0].subtex->height);

			if(sapp0_image[1].subtex)
				Draw_texture(&sapp0_image[1], 200, 100, sapp0_image[1].subtex->width, sapp0_image[1].subtex->height);

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

			Draw(DEF_SAPP0_VER, 0, 0, 0.4, 0.4, DEF_DRAW_GREEN);

			if(Util_err_query_error_show_flag())
				Util_err_draw();

			Draw_bot_ui();
		}

		Draw_apply_draw();
	}
	else
		gspWaitForVBlank();
}

static void Sapp0_draw_init_exit_message(void)
{
	int color = DEF_DRAW_BLACK;
	int back_color = DEF_DRAW_WHITE;
	Watch_handle_bit watch_handle_bit = (DEF_WATCH_HANDLE_BIT_GLOBAL | DEF_WATCH_HANDLE_BIT_SUB_APP0);

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

		if(Util_log_query_log_show_flag())
			Util_log_draw();

		Draw_top_ui();
		if(var_monitor_cpu_usage)
			Draw_cpu_usage_info();

		Draw(sapp0_status.buffer, 0, 20, 0.65, 0.65, color);

		//Draw the same things on right screen if 3D mode is enabled.
		//So that user can easily see them.
		if(Draw_is_3d_mode())
		{
			Draw_screen_ready(SCREEN_TOP_RIGHT, back_color);

			if(Util_log_query_log_show_flag())
				Util_log_draw();

			Draw_top_ui();
			if(var_monitor_cpu_usage)
				Draw_cpu_usage_info();

			Draw(sapp0_status.buffer, 0, 20, 0.65, 0.65, color);
		}

		Draw_apply_draw();
	}
	else
		gspWaitForVBlank();
}

static void Sapp0_init_thread(void* arg)
{
	DEF_LOG_STRING("Thread started.");
	uint8_t* buffer = NULL;
	uint8_t* png_data = NULL;
	int width = 0, height = 0;
	int dled_size = 0;
	char file_path[] = "romfs:/gfx/draw/sapp0/sample.jpg";
	char url[] = "https://user-images.githubusercontent.com/45873899/167138864-b6a9e25e-2dce-49d0-9b5a-d5986e768ad6.png";
	//If you want to load picture from SD (not from romfs).
	//char file_path[] = "/test.png";
	Pixel_format color_format = PIXEL_FORMAT_INVALID;
	Result_with_string result;

	Util_str_set(&sapp0_status, "Initializing variables...");
	//Empty.

	Util_str_add(&sapp0_status, "\nInitializing queue...");
	//Empty.

	Util_str_add(&sapp0_status, "\nLoading picture from romfs...");
	//Load picture from romfs (or SD card).

	//1. Decode a picture.
	DEF_LOG_RESULT_SMART(result, Util_image_decoder_decode(file_path, &buffer, &width, &height, &color_format), (result.code == DEF_SUCCESS), result.code);
	DEF_LOG_FORMAT("Picture size : %" PRId32 "x%" PRId32, width, height);
	if(result.code == 0)
	{
		Color_converter_parameters parameters;

		//2. Convert color format.
		parameters.source = buffer;
		parameters.converted = NULL;
		parameters.in_width = width;
		parameters.in_height = height;
		parameters.out_width = width;
		parameters.out_height = height;
		parameters.in_color_format = color_format;
		//If input image has transparency, use FORMAT_ABGR8888 to keep transparency otherwise use FORMAT_RGB565LE.
		if(color_format == PIXEL_FORMAT_RGBA8888 || color_format == PIXEL_FORMAT_GRAYALPHA88)
			parameters.out_color_format = PIXEL_FORMAT_ABGR8888;
		else
			parameters.out_color_format = PIXEL_FORMAT_RGB565LE;

		DEF_LOG_RESULT_SMART(result, Util_converter_convert_color(&parameters), (result.code == DEF_SUCCESS), result.code);

		//3. Init tecture, 1024 is texture size, it must be multiple of 2, so 2, 4, 8, 16, 32, 64...etc.
		DEF_LOG_RESULT_SMART(result, Draw_texture_init(&sapp0_image[0], 1024, 1024, parameters.out_color_format), (result.code == DEF_SUCCESS), result.code);
		if(result.code == 0)
		{
			//4. Set raw image data to texture.
			Draw_set_texture_data(&sapp0_image[0], parameters.converted, parameters.out_width, parameters.out_height);
		}

		free(parameters.converted);
		parameters.converted = NULL;
	}
	free(buffer);
	buffer = NULL;


	Util_str_add(&sapp0_status, "\nLoading picture from the Internet...");
	color_format = PIXEL_FORMAT_INVALID;

	//Load picture from the Internet.

	//1. Download png from the Internet.
	DEF_LOG_RESULT_SMART(result, Util_curl_dl_data(url, &png_data, 1024 * 1024, &dled_size, true, 5), (result.code == DEF_SUCCESS), result.code);
	if(result.code == 0)
	{
		//2. Decode a picture.
		DEF_LOG_RESULT_SMART(result, Util_image_decoder_decode(png_data, dled_size, &buffer, &width, &height, &color_format), (result.code == DEF_SUCCESS), result.code);
		DEF_LOG_FORMAT("Picture size : %" PRId32 "x%" PRId32, width, height);
		if(result.code == 0)
		{
			Color_converter_parameters parameters;

			//3. Convert color format.
			parameters.source = buffer;
			parameters.converted = NULL;
			parameters.in_width = width;
			parameters.in_height = height;
			parameters.out_width = width;
			parameters.out_height = height;
			parameters.in_color_format = color_format;
			//If input image has transparency, use FORMAT_ABGR8888 to keep transparency otherwise use FORMAT_RGB565LE.
			if(color_format == PIXEL_FORMAT_RGBA8888 || color_format == PIXEL_FORMAT_GRAYALPHA88)
				parameters.out_color_format = PIXEL_FORMAT_ABGR8888;
			else
				parameters.out_color_format = PIXEL_FORMAT_RGB565LE;

			DEF_LOG_RESULT_SMART(result, Util_converter_convert_color(&parameters), (result.code == DEF_SUCCESS), result.code);

			//4. Init tecture, 1024 is texture size, it must be multiple of 2, so 2, 4, 8, 16, 32, 64...etc.
			DEF_LOG_RESULT_SMART(result, Draw_texture_init(&sapp0_image[1], 1024, 1024, parameters.out_color_format), (result.code == DEF_SUCCESS), result.code);
			if(result.code == 0)
			{
				//5. Set raw image data to texture.
				Draw_set_texture_data(&sapp0_image[1], parameters.converted, parameters.out_width, parameters.out_height);
			}

			free(parameters.converted);
			parameters.converted = NULL;
		}
		free(buffer);
		buffer = NULL;
	}
	free(png_data);
	png_data = NULL;

	Util_str_add(&sapp0_status, "\nStarting threads...");
	sapp0_thread_run = true;
	sapp0_worker_thread = threadCreate(Sapp0_worker_thread, (void*)(""), DEF_STACKSIZE, DEF_THREAD_PRIORITY_NORMAL, 1, false);

	sapp0_already_init = true;

	DEF_LOG_STRING("Thread exit.");
	threadExit(0);
}

static void Sapp0_exit_thread(void* arg)
{
	DEF_LOG_STRING("Thread started.");
	uint32_t result = DEF_ERR_OTHER;

	sapp0_thread_suspend = false;
	sapp0_thread_run = false;

	Util_str_set(&sapp0_status, "Exiting threads...");
	DEF_LOG_RESULT_SMART(result, threadJoin(sapp0_worker_thread, DEF_THREAD_WAIT_TIME), result, result);

	Util_str_add(&sapp0_status, "\nCleaning up...");
	threadFree(sapp0_worker_thread);

	//Free textures.
	Draw_texture_free(&sapp0_image[0]);
	Draw_texture_free(&sapp0_image[1]);

	sapp0_already_init = false;

	DEF_LOG_STRING("Thread exit.");
	threadExit(0);
}

static void Sapp0_worker_thread(void* arg)
{
	DEF_LOG_STRING("Thread started.");

	while (sapp0_thread_run)
	{
		if(false)
		{

		}
		else
			Util_sleep(DEF_ACTIVE_THREAD_SLEEP_TIME);

		while (sapp0_thread_suspend)
			Util_sleep(DEF_INACTIVE_THREAD_SLEEP_TIME);
	}

	DEF_LOG_STRING("Thread exit.");
	threadExit(0);
}
