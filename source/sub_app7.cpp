#include "headers.hpp"

#include "sub_app7.hpp"

bool sapp7_main_run = false;
bool sapp7_thread_run = false;
bool sapp7_already_init = false;
bool sapp7_thread_suspend = true;
std::string sapp7_msg[DEF_SAPP7_NUM_OF_MSG];
std::string sapp7_status = "";
Thread sapp7_init_thread, sapp7_exit_thread, sapp7_worker_thread;

bool Sapp7_query_init_flag(void)
{
	return sapp7_already_init;
}

bool Sapp7_query_running_flag(void)
{
	return sapp7_main_run;
}

void Sapp7_worker_thread(void* arg)
{
	Util_log_save(DEF_SAPP7_WORKER_THREAD_STR, "Thread started.");
	
	while (sapp7_thread_run)
	{
		if(false)
		{

		}
		else
			usleep(DEF_ACTIVE_THREAD_SLEEP_TIME);

		while (sapp7_thread_suspend)
			usleep(DEF_INACTIVE_THREAD_SLEEP_TIME);
	}

	Util_log_save(DEF_SAPP7_WORKER_THREAD_STR, "Thread exit.");
	threadExit(0);
}

void Sapp7_init_thread(void* arg)
{
	Util_log_save(DEF_SAPP7_INIT_STR, "Thread started.");
	Result_with_string result;
	
	sapp7_status = "Loading messages...";
	var_need_reflesh = true;

	result = Util_load_msg("sapp7_" + var_lang + ".txt", sapp7_msg, DEF_SAPP7_NUM_OF_MSG);
	if(result.code != 0)
		Util_log_save(DEF_SAPP7_INIT_STR, "Util_load_msg()..." + result.string + result.error_description, result.code);

	sapp7_status = "Starting threads...";
	var_need_reflesh = true;

	sapp7_thread_run = true;
	sapp7_worker_thread = threadCreate(Sapp7_worker_thread, (void*)(""), DEF_STACKSIZE, DEF_THREAD_PRIORITY_NORMAL, 1, false);

	sapp7_already_init = true;

	Util_log_save(DEF_SAPP7_INIT_STR, "Thread exit.");
	threadExit(0);
}

void Sapp7_exit_thread(void* arg)
{
	Util_log_save(DEF_SAPP7_EXIT_STR, "Thread started.");
	u64 time_out = 10000000000;

	sapp7_thread_suspend = false;
	sapp7_thread_run = false;

	sapp7_status = "Exiting threads...";
	var_need_reflesh = true;

	Util_log_save(DEF_SAPP7_EXIT_STR, "threadJoin()...", threadJoin(sapp7_init_thread, time_out));	

	sapp7_status += ".";
	var_need_reflesh = true;

	Util_log_save(DEF_SAPP7_EXIT_STR, "threadJoin()...", threadJoin(sapp7_worker_thread, time_out));

	sapp7_status = "Cleaning up...";
	
	threadFree(sapp7_init_thread);
	threadFree(sapp7_worker_thread);

	sapp7_already_init = false;

	Util_log_save(DEF_SAPP7_EXIT_STR, "Thread exit.");
	threadExit(0);
}

void Sapp7_resume(void)
{
	sapp7_thread_suspend = false;
	sapp7_main_run = true;
	var_need_reflesh = true;
	Menu_suspend();
}

void Sapp7_suspend(void)
{
	sapp7_thread_suspend = true;
	sapp7_main_run = false;
	var_need_reflesh = true;
	Menu_resume();
}

void Sapp7_init(void)
{
	Util_log_save(DEF_SAPP7_INIT_STR, "Initializing...");
	int color = DEF_DRAW_BLACK;
	int back_color = DEF_DRAW_WHITE;

	sapp7_status = var_model;
	var_need_reflesh = true;

	if(var_model == "NEW 2DS XL" || var_model == "NEW 3DS XL" || var_model == "NEW 3DS")
		sapp7_init_thread = threadCreate(Sapp7_init_thread, (void*)(""), DEF_STACKSIZE, DEF_THREAD_PRIORITY_NORMAL, 2, false);
	else
	{
		APT_SetAppCpuTimeLimit(80);
		sapp7_init_thread = threadCreate(Sapp7_init_thread, (void*)(""), DEF_STACKSIZE, DEF_THREAD_PRIORITY_NORMAL, 1, false);
	}

	while(!sapp7_already_init)
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
			Draw(sapp7_status, 0, 20, 0.65, 0.65, color);

			Draw_apply_draw();
		}
		else
			gspWaitForVBlank();
	}

	if(!(var_model == "NEW 2DS XL" || var_model == "NEW 3DS XL" || var_model == "NEW 3DS"))
		APT_SetAppCpuTimeLimit(10);

	Sapp7_resume();

	Util_log_save(DEF_SAPP7_INIT_STR, "Initialized.");
}

void Sapp7_exit(void)
{
	Util_log_save(DEF_SAPP7_EXIT_STR, "Exiting...");

	int color = DEF_DRAW_BLACK;
	int back_color = DEF_DRAW_WHITE;
	u64 time_out = 10000000000;

	sapp7_status = "";
	var_need_reflesh = true;

	sapp7_exit_thread = threadCreate(Sapp7_exit_thread, (void*)(""), DEF_STACKSIZE, DEF_THREAD_PRIORITY_NORMAL, 1, false);

	while(sapp7_already_init)
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
			Draw(sapp7_status, 0, 20, 0.65, 0.65, color);

			Draw_apply_draw();
		}
		else
			gspWaitForVBlank();
	}

	Util_log_save(DEF_SAPP7_EXIT_STR, "threadJoin()...", threadJoin(sapp7_exit_thread, time_out));	
	threadFree(sapp7_exit_thread);
	var_need_reflesh = true;

	Util_log_save(DEF_SAPP7_EXIT_STR, "Exited.");
}

void Sapp7_main(void)
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

		Draw(sapp7_msg[0], 0, 20, 0.5, 0.5, color);
		if(Util_log_query_log_show_flag())
			Util_log_draw();

		Draw_top_ui();

		Draw_screen_ready(1, back_color);

		Draw(DEF_SAPP7_VER, 0, 0, 0.4, 0.4, DEF_DRAW_GREEN);

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
			Sapp7_suspend();
	}

	if(Util_log_query_log_show_flag())
		Util_log_main(key);
}
