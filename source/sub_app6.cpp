#include "headers.hpp"

#include "sub_app6.hpp"

bool sapp6_main_run = false;
bool sapp6_thread_run = false;
bool sapp6_already_init = false;
bool sapp6_thread_suspend = true;
std::string sapp6_msg[DEF_SAPP6_NUM_OF_MSG];
std::string sapp6_status = "";
Thread sapp6_init_thread, sapp6_exit_thread, sapp6_worker_thread;

bool Sapp6_query_init_flag(void)
{
	return sapp6_already_init;
}

bool Sapp6_query_running_flag(void)
{
	return sapp6_main_run;
}

void Sapp6_worker_thread(void* arg)
{
	Util_log_save(DEF_SAPP6_WORKER_THREAD_STR, "Thread started.");
	
	while (sapp6_thread_run)
	{
		if(false)
		{

		}
		else
			usleep(DEF_ACTIVE_THREAD_SLEEP_TIME);

		while (sapp6_thread_suspend)
			usleep(DEF_INACTIVE_THREAD_SLEEP_TIME);
	}

	Util_log_save(DEF_SAPP6_WORKER_THREAD_STR, "Thread exit.");
	threadExit(0);
}

void Sapp6_init_thread(void* arg)
{
	Util_log_save(DEF_SAPP6_INIT_STR, "Thread started.");
	Result_with_string result;
	
	sapp6_status = "Loading messages...";
	var_need_reflesh = true;

	result = Util_load_msg("sapp6_" + var_lang + ".txt", sapp6_msg, DEF_SAPP6_NUM_OF_MSG);
	if(result.code != 0)
		Util_log_save(DEF_SAPP6_INIT_STR, "Util_load_msg()..." + result.string + result.error_description, result.code);

	sapp6_status = "Starting threads...";
	var_need_reflesh = true;

	sapp6_thread_run = true;
	sapp6_worker_thread = threadCreate(Sapp6_worker_thread, (void*)(""), DEF_STACKSIZE, DEF_THREAD_PRIORITY_NORMAL, 1, false);

	sapp6_already_init = true;

	Util_log_save(DEF_SAPP6_INIT_STR, "Thread exit.");
	threadExit(0);
}

void Sapp6_exit_thread(void* arg)
{
	Util_log_save(DEF_SAPP6_EXIT_STR, "Thread started.");
	u64 time_out = 10000000000;

	sapp6_thread_suspend = false;
	sapp6_thread_run = false;

	sapp6_status = "Exiting threads...";
	var_need_reflesh = true;

	Util_log_save(DEF_SAPP6_EXIT_STR, "threadJoin()...", threadJoin(sapp6_init_thread, time_out));	

	sapp6_status += ".";
	var_need_reflesh = true;

	Util_log_save(DEF_SAPP6_EXIT_STR, "threadJoin()...", threadJoin(sapp6_worker_thread, time_out));

	sapp6_status = "Cleaning up...";
	
	threadFree(sapp6_init_thread);
	threadFree(sapp6_worker_thread);

	sapp6_already_init = false;

	Util_log_save(DEF_SAPP6_EXIT_STR, "Thread exit.");
	threadExit(0);
}

void Sapp6_resume(void)
{
	sapp6_thread_suspend = false;
	sapp6_main_run = true;
	var_need_reflesh = true;
	Menu_suspend();
}

void Sapp6_suspend(void)
{
	sapp6_thread_suspend = true;
	sapp6_main_run = false;
	var_need_reflesh = true;
	Menu_resume();
}

void Sapp6_init(void)
{
	Util_log_save(DEF_SAPP6_INIT_STR, "Initializing...");
	int color = DEF_DRAW_BLACK;
	int back_color = DEF_DRAW_WHITE;

	sapp6_status = var_model;
	var_need_reflesh = true;

	if(var_model == "NEW 2DS XL" || var_model == "NEW 3DS XL" || var_model == "NEW 3DS")
		sapp6_init_thread = threadCreate(Sapp6_init_thread, (void*)(""), DEF_STACKSIZE, DEF_THREAD_PRIORITY_NORMAL, 2, false);
	else
	{
		APT_SetAppCpuTimeLimit(80);
		sapp6_init_thread = threadCreate(Sapp6_init_thread, (void*)(""), DEF_STACKSIZE, DEF_THREAD_PRIORITY_NORMAL, 1, false);
	}

	while(!sapp6_already_init)
	{
		if (var_night_mode)
		{
			color = DEF_DRAW_WHITE;
			back_color = DEF_DRAW_BLACK;
		}

		if(var_need_reflesh || !var_eco_mode)
		{
			var_need_reflesh = false;
			Draw_frame_ready();
			Draw_screen_ready(0, back_color);
			Draw_top_ui();
			Draw(sapp6_status, 0, 20, 0.65, 0.65, color);

			Draw_apply_draw();
		}
		else
			gspWaitForVBlank();
	}

	if(!(var_model == "NEW 2DS XL" || var_model == "NEW 3DS XL" || var_model == "NEW 3DS"))
		APT_SetAppCpuTimeLimit(10);

	Sapp6_resume();

	Util_log_save(DEF_SAPP6_INIT_STR, "Initialized.");
}

void Sapp6_exit(void)
{
	Util_log_save(DEF_SAPP6_EXIT_STR, "Exiting...");

	int color = DEF_DRAW_BLACK;
	int back_color = DEF_DRAW_WHITE;
	u64 time_out = 10000000000;

	sapp6_status = "";
	var_need_reflesh = true;

	sapp6_exit_thread = threadCreate(Sapp6_exit_thread, (void*)(""), DEF_STACKSIZE, DEF_THREAD_PRIORITY_NORMAL, 1, false);

	while(sapp6_already_init)
	{
		if (var_night_mode)
		{
			color = DEF_DRAW_WHITE;
			back_color = DEF_DRAW_BLACK;
		}

		if(var_need_reflesh || !var_eco_mode)
		{
			var_need_reflesh = false;
			Draw_frame_ready();
			Draw_screen_ready(0, back_color);
			Draw_top_ui();
			Draw(sapp6_status, 0, 20, 0.65, 0.65, color);

			Draw_apply_draw();
		}
		else
			gspWaitForVBlank();
	}

	Util_log_save(DEF_SAPP6_EXIT_STR, "threadJoin()...", threadJoin(sapp6_exit_thread, time_out));	
	threadFree(sapp6_exit_thread);
	var_need_reflesh = true;

	Util_log_save(DEF_SAPP6_EXIT_STR, "Exited.");
}

void Sapp6_main(void)
{
	int color = DEF_DRAW_BLACK;
	int back_color = DEF_DRAW_WHITE;
	Hid_info key;
	Util_hid_query_key_state(&key);
	Util_hid_key_flag_reset();

	if (var_night_mode)
	{
		color = DEF_DRAW_WHITE;
		back_color = DEF_DRAW_BLACK;
	}

	if(var_need_reflesh || !var_eco_mode)
	{
		var_need_reflesh = false;
		Draw_frame_ready();
		Draw_screen_ready(0, back_color);

		Draw(sapp6_msg[0], 0, 20, 0.5, 0.5, color);
		if(Util_log_query_log_show_flag())
			Util_log_draw();

		Draw_top_ui();

		Draw_screen_ready(1, back_color);

		Draw(DEF_SAPP6_VER, 0, 0, 0.4, 0.4, DEF_DRAW_GREEN);

		if(Util_err_query_error_show_flag())
			Util_err_draw();

		Draw_bot_ui();
		Draw_touch_pos();

		Draw_apply_draw();
	}
	else
		gspWaitForVBlank();

	if(Util_err_query_error_show_flag())
		Util_err_main(key);
	else
	{
		if(key.p_touch || key.h_touch)
			var_need_reflesh = true;
		if (key.p_start || (key.p_touch && key.touch_x >= 110 && key.touch_x <= 230 && key.touch_y >= 220 && key.touch_y <= 240))
			Sapp6_suspend();
	}

	if(Util_log_query_log_show_flag())
		Util_log_main(key);
}
