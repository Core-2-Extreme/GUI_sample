#include "definitions.hpp"
#include "system/types.hpp"

#include "system/menu.hpp"
#include "system/variables.hpp"

#include "system/draw/draw.hpp"

#include "system/util/error.hpp"
#include "system/util/explorer.hpp"
#include "system/util/hid.hpp"
#include "system/util/log.hpp"
#include "system/util/util.hpp"

extern "C"
{
	#include "system/util/str.h"
}

//Include myself.
#include "sub_app1.hpp"


bool sapp1_main_run = false;
bool sapp1_thread_run = false;
bool sapp1_already_init = false;
bool sapp1_thread_suspend = true;
std::string sapp1_msg[DEF_SAPP1_NUM_OF_MSG];
Thread sapp1_init_thread, sapp1_exit_thread, sapp1_worker_thread;
Util_str sapp1_status = { 0, };
Util_str sapp1_selected_path = { 0, };
Util_str sapp1_file_info = { 0, };


static void Sapp1_draw_init_exit_message(void);
static void Sapp1_init_thread(void* arg);
static void Sapp1_exit_thread(void* arg);
static void Sapp1_worker_thread(void* arg);
static void Sapp1_expl_callback(std::string file_name, std::string dir_path);
static void Sapp1_expl_cancel_callback(void);


bool Sapp1_query_init_flag(void)
{
	return sapp1_already_init;
}

bool Sapp1_query_running_flag(void)
{
	return sapp1_main_run;
}

void Sapp1_hid(Hid_info key)
{
	//Do nothing if app is suspended.
	if(aptShouldJumpToHome())
		return;

	if(Util_err_query_error_show_flag())
		Util_err_main(key);
	else if(Util_expl_query_show_flag())//Handle file explorer key input here.
		Util_expl_main(key);
	else
	{
		if(Util_hid_is_pressed(key, *Draw_get_bot_ui_button()))
			Draw_get_bot_ui_button()->selected = true;
		else if (key.p_start || (Util_hid_is_released(key, *Draw_get_bot_ui_button()) && Draw_get_bot_ui_button()->selected))
			Sapp1_suspend();
		else if (key.p_x)
		{
			//Set callbacks and open file explorer.
			Util_expl_set_callback(Sapp1_expl_callback);
			Util_expl_set_cancel_callback(Sapp1_expl_cancel_callback);
			Util_expl_set_show_flag(true);
		}
	}

	if(!key.p_touch && !key.h_touch)
		Draw_get_bot_ui_button()->selected = false;

	if(Util_log_query_log_show_flag())
		Util_log_main(key);
}

void Sapp1_resume(void)
{
	sapp1_thread_suspend = false;
	sapp1_main_run = true;
	var_need_reflesh = true;
	Menu_suspend();
}

void Sapp1_suspend(void)
{
	sapp1_thread_suspend = true;
	sapp1_main_run = false;
	var_need_reflesh = true;
	Menu_resume();
}

Result_with_string Sapp1_load_msg(std::string lang)
{
	return Util_load_msg("sapp1_" + lang + ".txt", sapp1_msg, DEF_SAPP1_NUM_OF_MSG);
}

void Sapp1_init(bool draw)
{
	DEF_LOG_STRING("Initializing...");
	uint32_t result = DEF_ERR_OTHER;

	DEF_LOG_RESULT_SMART(result, Util_str_init(&sapp1_status), (result == DEF_SUCCESS), result);

	Util_add_watch(WATCH_HANDLE_SUB_APP1, &sapp1_status.sequencial_id, sizeof(sapp1_status.sequencial_id));

	if((var_model == CFG_MODEL_N2DSXL || var_model == CFG_MODEL_N3DSXL || var_model == CFG_MODEL_N3DS) && var_core_2_available)
		sapp1_init_thread = threadCreate(Sapp1_init_thread, (void*)(""), DEF_STACKSIZE, DEF_THREAD_PRIORITY_NORMAL, 2, false);
	else
	{
		APT_SetAppCpuTimeLimit(80);
		sapp1_init_thread = threadCreate(Sapp1_init_thread, (void*)(""), DEF_STACKSIZE, DEF_THREAD_PRIORITY_NORMAL, 1, false);
	}

	while(!sapp1_already_init)
	{
		if(draw)
			Sapp1_draw_init_exit_message();
		else
			Util_sleep(20000);
	}

	if(!(var_model == CFG_MODEL_N2DSXL || var_model == CFG_MODEL_N3DSXL || var_model == CFG_MODEL_N3DS) || !var_core_2_available)
		APT_SetAppCpuTimeLimit(10);

	DEF_LOG_RESULT_SMART(result, threadJoin(sapp1_init_thread, DEF_THREAD_WAIT_TIME), result, result);
	threadFree(sapp1_init_thread);

	Util_str_clear(&sapp1_status);
	Sapp1_resume();

	DEF_LOG_STRING("Initialized.");
}

void Sapp1_exit(bool draw)
{
	DEF_LOG_STRING("Exiting...");
	uint32_t result = DEF_ERR_OTHER;

	sapp1_exit_thread = threadCreate(Sapp1_exit_thread, (void*)(""), DEF_STACKSIZE, DEF_THREAD_PRIORITY_NORMAL, 1, false);

	while(sapp1_already_init)
	{
		if(draw)
			Sapp1_draw_init_exit_message();
		else
			Util_sleep(20000);
	}

	DEF_LOG_RESULT_SMART(result, threadJoin(sapp1_exit_thread, DEF_THREAD_WAIT_TIME), result, result);
	threadFree(sapp1_exit_thread);

	Util_remove_watch(WATCH_HANDLE_SUB_APP1, &sapp1_status.sequencial_id);
	Util_str_free(&sapp1_status);
	var_need_reflesh = true;

	DEF_LOG_STRING("Exited.");
}

void Sapp1_main(void)
{
	int color = DEF_DRAW_BLACK;
	int back_color = DEF_DRAW_WHITE;
	Watch_handle_bit watch_handle_bit = (DEF_WATCH_HANDLE_BIT_GLOBAL | DEF_WATCH_HANDLE_BIT_SUB_APP1);

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

			Draw(sapp1_msg[0], 0, 20, 0.5, 0.5, color);

			//Draw file info.
			if(Util_str_has_data(&sapp1_selected_path))
				Draw(sapp1_selected_path.buffer, 0, 40, 0.45, 0.45, color);
			if(Util_str_has_data(&sapp1_file_info))
				Draw(sapp1_file_info.buffer, 0, 90, 0.45, 0.45, color);

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

			Draw(DEF_SAPP1_VER, 0, 0, 0.4, 0.4, DEF_DRAW_GREEN);

			if(Util_expl_query_show_flag())//Draw file explorer.
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

static void Sapp1_draw_init_exit_message(void)
{
	int color = DEF_DRAW_BLACK;
	int back_color = DEF_DRAW_WHITE;
	Watch_handle_bit watch_handle_bit = (DEF_WATCH_HANDLE_BIT_GLOBAL | DEF_WATCH_HANDLE_BIT_SUB_APP1);

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

		Draw(sapp1_status.buffer, 0, 20, 0.65, 0.65, color);

		Draw_apply_draw();
	}
	else
		gspWaitForVBlank();
}

static void Sapp1_init_thread(void* arg)
{
	DEF_LOG_STRING("Thread started.");
	uint32_t result = DEF_ERR_OTHER;

	Util_str_set(&sapp1_status, "Initializing variables...");

	DEF_LOG_RESULT_SMART(result, Util_str_init(&sapp1_selected_path), (result == DEF_SUCCESS), result);
	DEF_LOG_RESULT_SMART(result, Util_str_init(&sapp1_file_info), (result == DEF_SUCCESS), result);

	//Add to watch to detect value changes, screen will be rerenderd when value is changed.
	Util_add_watch(WATCH_HANDLE_SUB_APP1, &sapp1_selected_path.sequencial_id, sizeof(sapp1_selected_path.sequencial_id));
	Util_add_watch(WATCH_HANDLE_SUB_APP1, &sapp1_file_info.sequencial_id, sizeof(sapp1_file_info.sequencial_id));
	Util_str_set(&sapp1_selected_path, "Press X button to open file explorer.");

	Util_str_add(&sapp1_status, "\nInitializing queue...");
	//Empty.

	Util_str_add(&sapp1_status, "\nStarting threads...");
	sapp1_thread_run = true;
	sapp1_worker_thread = threadCreate(Sapp1_worker_thread, (void*)(""), DEF_STACKSIZE, DEF_THREAD_PRIORITY_NORMAL, 1, false);

	sapp1_already_init = true;

	DEF_LOG_STRING("Thread exit.");
	threadExit(0);
}

static void Sapp1_exit_thread(void* arg)
{
	DEF_LOG_STRING("Thread started.");
	uint32_t result = DEF_ERR_OTHER;

	sapp1_thread_suspend = false;
	sapp1_thread_run = false;

	Util_str_set(&sapp1_status, "Exiting threads...");
	DEF_LOG_RESULT_SMART(result, threadJoin(sapp1_worker_thread, DEF_THREAD_WAIT_TIME), result, result);

	Util_str_add(&sapp1_status, "\nCleaning up...");
	threadFree(sapp1_worker_thread);

	//Remove watch on exit.
	Util_remove_watch(WATCH_HANDLE_SUB_APP1, &sapp1_selected_path.sequencial_id);
	Util_remove_watch(WATCH_HANDLE_SUB_APP1, &sapp1_file_info.sequencial_id);

	//Free string buffers.
	Util_str_free(&sapp1_selected_path);
	Util_str_free(&sapp1_file_info);

	sapp1_already_init = false;

	DEF_LOG_STRING("Thread exit.");
	threadExit(0);
}

static void Sapp1_worker_thread(void* arg)
{
	DEF_LOG_STRING("Thread started.");

	while (sapp1_thread_run)
	{
		if(false)
		{

		}
		else
			Util_sleep(DEF_ACTIVE_THREAD_SLEEP_TIME);

		while (sapp1_thread_suspend)
			Util_sleep(DEF_INACTIVE_THREAD_SLEEP_TIME);
	}

	DEF_LOG_STRING("Thread exit.");
	threadExit(0);
}

static void Sapp1_expl_callback(std::string file_name, std::string dir_path)
{
	int file_type = Util_expl_query_type(Util_expl_query_current_file_index());
	int file_size = Util_expl_query_size(Util_expl_query_current_file_index());
	Util_str temp_string = { 0, };

	if(Util_str_init(&temp_string) == DEF_SUCCESS)
	{
		if(file_type != FILE_TYPE_NONE)
		{
			if(file_type & FILE_TYPE_HIDDEN)
				Util_str_add(&temp_string, "Hidden ");
			if(file_type & FILE_TYPE_READ_ONLY)
				Util_str_add(&temp_string, "Read only ");
			if(file_type & FILE_TYPE_DIR)
				Util_str_add(&temp_string, "Directory ");
			if(file_type & FILE_TYPE_FILE)
				Util_str_add(&temp_string, "File ");
		}
		else
			Util_str_set(&temp_string, "Unknown");

		//Set file info.
		Util_str_format(&sapp1_selected_path, "User selected : \n%s%s\nPress X button to open file explorer.", dir_path.c_str(), file_name.c_str());
		Util_str_format(&sapp1_file_info, "File size : %dKB (%dB)\nType : %s", (file_size / 1024), file_size, temp_string.buffer);
	}
	else
		Util_str_set(&sapp1_file_info, "Out of memory.");

	Util_str_free(&temp_string);
}

static void Sapp1_expl_cancel_callback(void)
{
	Util_str_set(&sapp1_selected_path, "Canceled by user.\nPress X button to open file explorer.");
	Util_str_set(&sapp1_file_info, "");
}
