#include "system/headers.hpp"

bool sapp6_main_run = false;
bool sapp6_thread_run = false;
bool sapp6_already_init = false;
bool sapp6_thread_suspend = true;
std::string sapp6_msg[DEF_SAPP6_NUM_OF_MSG];
std::string sapp6_status = "";
Thread sapp6_init_thread, sapp6_exit_thread, sapp6_worker_thread;

void Sapp6_suspend(void);

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

void Sapp6_hid(Hid_info key)
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
			Sapp6_suspend();
	}

	if(!key.p_touch && !key.h_touch)
		Draw_get_bot_ui_button()->selected = false;

	if(Util_log_query_log_show_flag())
		Util_log_main(key);
}

void Sapp6_init_thread(void* arg)
{
	Util_log_save(DEF_SAPP6_INIT_STR, "Thread started.");
	Result_with_string result;
	
	sapp6_status = "Starting threads...";

	sapp6_thread_run = true;
	sapp6_worker_thread = threadCreate(Sapp6_worker_thread, (void*)(""), DEF_STACKSIZE, DEF_THREAD_PRIORITY_NORMAL, 1, false);

	sapp6_already_init = true;

	Util_log_save(DEF_SAPP6_INIT_STR, "Thread exit.");
	threadExit(0);
}

void Sapp6_exit_thread(void* arg)
{
	Util_log_save(DEF_SAPP6_EXIT_STR, "Thread started.");

	sapp6_thread_suspend = false;
	sapp6_thread_run = false;

	sapp6_status = "Exiting threads...";
	Util_log_save(DEF_SAPP6_EXIT_STR, "threadJoin()...", threadJoin(sapp6_worker_thread, DEF_THREAD_WAIT_TIME));

	sapp6_status += "\nCleaning up...";	
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

Result_with_string Sapp6_load_msg(std::string lang)
{
	return  Util_load_msg("sapp6_" + lang + ".txt", sapp6_msg, DEF_SAPP6_NUM_OF_MSG);
}

void Sapp6_init(bool draw)
{
	Util_log_save(DEF_SAPP6_INIT_STR, "Initializing...");
	int color = DEF_DRAW_BLACK;
	int back_color = DEF_DRAW_WHITE;

	Util_add_watch(&sapp6_status);
	sapp6_status = "";

	if((var_model == CFG_MODEL_N2DSXL || var_model == CFG_MODEL_N3DSXL || var_model == CFG_MODEL_N3DS) && var_core_2_available)
		sapp6_init_thread = threadCreate(Sapp6_init_thread, (void*)(""), DEF_STACKSIZE, DEF_THREAD_PRIORITY_NORMAL, 2, false);
	else
	{
		APT_SetAppCpuTimeLimit(80);
		sapp6_init_thread = threadCreate(Sapp6_init_thread, (void*)(""), DEF_STACKSIZE, DEF_THREAD_PRIORITY_NORMAL, 1, false);
	}

	while(!sapp6_already_init)
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
				Draw_screen_ready(SCREEN_TOP_LEFT, back_color);
				Draw_top_ui();
				if(var_monitor_cpu_usage)
					Draw_cpu_usage_info();

				Draw(sapp6_status, 0, 20, 0.65, 0.65, color);

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

	Util_log_save(DEF_SAPP6_EXIT_STR, "threadJoin()...", threadJoin(sapp6_init_thread, DEF_THREAD_WAIT_TIME));	
	threadFree(sapp6_init_thread);
	Sapp6_resume();

	Util_log_save(DEF_SAPP6_INIT_STR, "Initialized.");
}

void Sapp6_exit(bool draw)
{
	Util_log_save(DEF_SAPP6_EXIT_STR, "Exiting...");

	int color = DEF_DRAW_BLACK;
	int back_color = DEF_DRAW_WHITE;

	sapp6_status = "";
	sapp6_exit_thread = threadCreate(Sapp6_exit_thread, (void*)(""), DEF_STACKSIZE, DEF_THREAD_PRIORITY_NORMAL, 1, false);

	while(sapp6_already_init)
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
				Draw_screen_ready(SCREEN_TOP_LEFT, back_color);
				Draw_top_ui();
				if(var_monitor_cpu_usage)
					Draw_cpu_usage_info();

				Draw(sapp6_status, 0, 20, 0.65, 0.65, color);

				Draw_apply_draw();
			}
			else
				gspWaitForVBlank();
		}
		else
			usleep(20000);
	}

	Util_log_save(DEF_SAPP6_EXIT_STR, "threadJoin()...", threadJoin(sapp6_exit_thread, DEF_THREAD_WAIT_TIME));	
	threadFree(sapp6_exit_thread);
	Util_remove_watch(&sapp6_status);
	var_need_reflesh = true;

	Util_log_save(DEF_SAPP6_EXIT_STR, "Exited.");
}

void Sapp6_main(void)
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
			Draw_screen_ready(SCREEN_TOP_LEFT, back_color);

			Draw(sapp6_msg[0], 0, 20, 0.5, 0.5, color);
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

			Draw(DEF_SAPP6_VER, 0, 0, 0.4, 0.4, DEF_DRAW_GREEN);

			if(Util_err_query_error_show_flag())
				Util_err_draw();

			Draw_bot_ui();
		}

		Draw_apply_draw();
	}
	else
		gspWaitForVBlank();
}
