#include "system/headers.hpp"

bool sapp7_main_run = false;
bool sapp7_thread_run = false;
bool sapp7_already_init = false;
bool sapp7_thread_suspend = true;
std::string sapp7_msg[DEF_SAPP7_NUM_OF_MSG];
std::string sapp7_status = "";
Thread sapp7_init_thread, sapp7_exit_thread, sapp7_worker_thread;

void Sapp7_suspend(void);

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

void Sapp7_hid(Hid_info key)
{
	if(Util_err_query_error_show_flag())
		Util_err_main(key);
	else
	{
		if(Util_hid_is_pressed(key, *Draw_get_bot_ui_button()))
			Draw_get_bot_ui_button()->selected = true;
		else if (key.p_start || (Util_hid_is_released(key, *Draw_get_bot_ui_button()) && Draw_get_bot_ui_button()->selected))
			Sapp7_suspend();
	}

	if(!key.p_touch && !key.h_touch)
		Draw_get_bot_ui_button()->selected = false;

	if(Util_log_query_log_show_flag())
		Util_log_main(key);
}

void Sapp7_init_thread(void* arg)
{
	Util_log_save(DEF_SAPP7_INIT_STR, "Thread started.");
	Result_with_string result;
	
	sapp7_status = "Starting threads...";

	sapp7_thread_run = true;
	sapp7_worker_thread = threadCreate(Sapp7_worker_thread, (void*)(""), DEF_STACKSIZE, DEF_THREAD_PRIORITY_NORMAL, 1, false);

	sapp7_already_init = true;

	Util_log_save(DEF_SAPP7_INIT_STR, "Thread exit.");
	threadExit(0);
}

void Sapp7_exit_thread(void* arg)
{
	Util_log_save(DEF_SAPP7_EXIT_STR, "Thread started.");

	sapp7_thread_suspend = false;
	sapp7_thread_run = false;

	sapp7_status = "Exiting threads...";
	Util_log_save(DEF_SAPP7_EXIT_STR, "threadJoin()...", threadJoin(sapp7_worker_thread, DEF_THREAD_WAIT_TIME));

	sapp7_status += "\nCleaning up...";	
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

Result_with_string Sapp7_load_msg(std::string lang)
{
	return  Util_load_msg("sapp7_" + lang + ".txt", sapp7_msg, DEF_SAPP7_NUM_OF_MSG);
}

void Sapp7_init(bool draw)
{
	Util_log_save(DEF_SAPP7_INIT_STR, "Initializing...");
	int color = DEF_DRAW_BLACK;
	int back_color = DEF_DRAW_WHITE;

	Util_add_watch(&sapp7_status);
	sapp7_status = "";

	if((var_model == CFG_MODEL_N2DSXL || var_model == CFG_MODEL_N3DSXL || var_model == CFG_MODEL_N3DS) && var_core_2_available)
		sapp7_init_thread = threadCreate(Sapp7_init_thread, (void*)(""), DEF_STACKSIZE, DEF_THREAD_PRIORITY_NORMAL, 2, false);
	else
	{
		APT_SetAppCpuTimeLimit(80);
		sapp7_init_thread = threadCreate(Sapp7_init_thread, (void*)(""), DEF_STACKSIZE, DEF_THREAD_PRIORITY_NORMAL, 1, false);
	}

	while(!sapp7_already_init)
	{
		if(draw)
		{
			if (var_night_mode)
			{
				color = DEF_DRAW_WHITE;
				back_color = DEF_DRAW_BLACK;
			}

			if(Util_is_watch_changed() || var_need_reflesh || !var_eco_mode)
			{
				var_need_reflesh = false;
				Draw_frame_ready();
				Draw_screen_ready(0, back_color);
				Draw_top_ui();
				if(var_monitor_cpu_usage)
					Draw_cpu_usage_info();

				Draw(sapp7_status, 0, 20, 0.65, 0.65, color);

				Draw_apply_draw();
			}
			else
				gspWaitForVBlank();
		}
		else
			usleep(20000);
	}

	if(!(var_model == CFG_MODEL_N2DSXL || var_model == CFG_MODEL_N3DSXL || var_model == CFG_MODEL_N3DS) || !var_core_2_available)
		APT_SetAppCpuTimeLimit(10);

	Util_log_save(DEF_SAPP7_EXIT_STR, "threadJoin()...", threadJoin(sapp7_init_thread, DEF_THREAD_WAIT_TIME));	
	threadFree(sapp7_init_thread);
	Sapp7_resume();

	Util_log_save(DEF_SAPP7_INIT_STR, "Initialized.");
}

void Sapp7_exit(bool draw)
{
	Util_log_save(DEF_SAPP7_EXIT_STR, "Exiting...");

	int color = DEF_DRAW_BLACK;
	int back_color = DEF_DRAW_WHITE;

	sapp7_status = "";
	sapp7_exit_thread = threadCreate(Sapp7_exit_thread, (void*)(""), DEF_STACKSIZE, DEF_THREAD_PRIORITY_NORMAL, 1, false);

	while(sapp7_already_init)
	{
		if(draw)
		{
			if (var_night_mode)
			{
				color = DEF_DRAW_WHITE;
				back_color = DEF_DRAW_BLACK;
			}

			if(Util_is_watch_changed() || var_need_reflesh || !var_eco_mode)
			{
				var_need_reflesh = false;
				Draw_frame_ready();
				Draw_screen_ready(0, back_color);
				Draw_top_ui();
				if(var_monitor_cpu_usage)
					Draw_cpu_usage_info();

				Draw(sapp7_status, 0, 20, 0.65, 0.65, color);

				Draw_apply_draw();
			}
			else
				gspWaitForVBlank();
		}
		else
			usleep(20000);
	}

	Util_log_save(DEF_SAPP7_EXIT_STR, "threadJoin()...", threadJoin(sapp7_exit_thread, DEF_THREAD_WAIT_TIME));	
	threadFree(sapp7_exit_thread);
	Util_remove_watch(&sapp7_status);
	var_need_reflesh = true;

	Util_log_save(DEF_SAPP7_EXIT_STR, "Exited.");
}

void Sapp7_main(void)
{
	int color = DEF_DRAW_BLACK;
	int back_color = DEF_DRAW_WHITE;

	if (var_night_mode)
	{
		color = DEF_DRAW_WHITE;
		back_color = DEF_DRAW_BLACK;
	}

	if(Util_is_watch_changed() || var_need_reflesh || !var_eco_mode)
	{
		var_need_reflesh = false;
		Draw_frame_ready();

		if(var_turn_on_top_lcd)
		{
			Draw_screen_ready(0, back_color);

			Draw(sapp7_msg[0], 0, 20, 0.5, 0.5, color);
			if(Util_log_query_log_show_flag())
				Util_log_draw();

			Draw_top_ui();

			if(var_monitor_cpu_usage)
				Draw_cpu_usage_info();

			if(var_3d_mode)
			{
				Draw_screen_ready(2, back_color);

				if(Util_log_query_log_show_flag())
					Util_log_draw();

				Draw_top_ui();

				if(var_monitor_cpu_usage)
					Draw_cpu_usage_info();

			}
		}
		
		if(var_turn_on_bottom_lcd)
		{
			Draw_screen_ready(1, back_color);

			Draw(DEF_SAPP7_VER, 0, 0, 0.4, 0.4, DEF_DRAW_GREEN);

			if(Util_err_query_error_show_flag())
				Util_err_draw();

			Draw_bot_ui();
		}

		Draw_apply_draw();
	}
	else
		gspWaitForVBlank();
}
