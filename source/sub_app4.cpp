#include "system/headers.hpp"

bool sapp4_main_run = false;
bool sapp4_thread_run = false;
bool sapp4_already_init = false;
bool sapp4_thread_suspend = true;
std::string sapp4_msg[DEF_SAPP4_NUM_OF_MSG];
std::string sapp4_status = "";
Thread sapp4_init_thread, sapp4_exit_thread, sapp4_worker_thread;

void Sapp4_suspend(void);

bool Sapp4_query_init_flag(void)
{
	return sapp4_already_init;
}

bool Sapp4_query_running_flag(void)
{
	return sapp4_main_run;
}

void Sapp4_worker_thread(void* arg)
{
	Util_log_save(DEF_SAPP4_WORKER_THREAD_STR, "Thread started.");
	
	while (sapp4_thread_run)
	{
		if(false)
		{

		}
		else
			usleep(DEF_ACTIVE_THREAD_SLEEP_TIME);

		while (sapp4_thread_suspend)
			usleep(DEF_INACTIVE_THREAD_SLEEP_TIME);
	}

	Util_log_save(DEF_SAPP4_WORKER_THREAD_STR, "Thread exit.");
	threadExit(0);
}

void Sapp4_hid(Hid_info key)
{
	if(Util_err_query_error_show_flag())
		Util_err_main(key);
	else
	{
		if(Util_hid_is_pressed(key, *Draw_get_bot_ui_button()))
			Draw_get_bot_ui_button()->selected = true;
		else if (key.p_start || (Util_hid_is_released(key, *Draw_get_bot_ui_button()) && Draw_get_bot_ui_button()->selected))
			Sapp4_suspend();
	}

	if(!key.p_touch && !key.h_touch)
		Draw_get_bot_ui_button()->selected = false;

	if(Util_log_query_log_show_flag())
		Util_log_main(key);
}

void Sapp4_init_thread(void* arg)
{
	Util_log_save(DEF_SAPP4_INIT_STR, "Thread started.");
	Result_with_string result;
	
	sapp4_status = "Starting threads...";

	sapp4_thread_run = true;
	sapp4_worker_thread = threadCreate(Sapp4_worker_thread, (void*)(""), DEF_STACKSIZE, DEF_THREAD_PRIORITY_NORMAL, 1, false);

	sapp4_already_init = true;

	Util_log_save(DEF_SAPP4_INIT_STR, "Thread exit.");
	threadExit(0);
}

void Sapp4_exit_thread(void* arg)
{
	Util_log_save(DEF_SAPP4_EXIT_STR, "Thread started.");

	sapp4_thread_suspend = false;
	sapp4_thread_run = false;

	sapp4_status = "Exiting threads...";
	Util_log_save(DEF_SAPP4_EXIT_STR, "threadJoin()...", threadJoin(sapp4_worker_thread, DEF_THREAD_WAIT_TIME));

	sapp4_status += "\nCleaning up...";	
	threadFree(sapp4_worker_thread);

	sapp4_already_init = false;

	Util_log_save(DEF_SAPP4_EXIT_STR, "Thread exit.");
	threadExit(0);
}

void Sapp4_resume(void)
{
	sapp4_thread_suspend = false;
	sapp4_main_run = true;
	var_need_reflesh = true;
	Menu_suspend();
}

void Sapp4_suspend(void)
{
	sapp4_thread_suspend = true;
	sapp4_main_run = false;
	var_need_reflesh = true;
	Menu_resume();
}

Result_with_string Sapp4_load_msg(std::string lang)
{
	return  Util_load_msg("sapp4_" + lang + ".txt", sapp4_msg, DEF_SAPP4_NUM_OF_MSG);
}

void Sapp4_init(bool draw)
{
	Util_log_save(DEF_SAPP4_INIT_STR, "Initializing...");
	int color = DEF_DRAW_BLACK;
	int back_color = DEF_DRAW_WHITE;

	Util_add_watch(&sapp4_status);
	sapp4_status = "";

	if((var_model == CFG_MODEL_N2DSXL || var_model == CFG_MODEL_N3DSXL || var_model == CFG_MODEL_N3DS) && var_core_2_available)
		sapp4_init_thread = threadCreate(Sapp4_init_thread, (void*)(""), DEF_STACKSIZE, DEF_THREAD_PRIORITY_NORMAL, 2, false);
	else
	{
		APT_SetAppCpuTimeLimit(80);
		sapp4_init_thread = threadCreate(Sapp4_init_thread, (void*)(""), DEF_STACKSIZE, DEF_THREAD_PRIORITY_NORMAL, 1, false);
	}

	while(!sapp4_already_init)
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
				Draw(sapp4_status, 0, 20, 0.65, 0.65, color);

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

	Util_log_save(DEF_SAPP4_EXIT_STR, "threadJoin()...", threadJoin(sapp4_init_thread, DEF_THREAD_WAIT_TIME));	
	threadFree(sapp4_init_thread);
	Sapp4_resume();

	Util_log_save(DEF_SAPP4_INIT_STR, "Initialized.");
}

void Sapp4_exit(bool draw)
{
	Util_log_save(DEF_SAPP4_EXIT_STR, "Exiting...");

	int color = DEF_DRAW_BLACK;
	int back_color = DEF_DRAW_WHITE;

	sapp4_status = "";
	sapp4_exit_thread = threadCreate(Sapp4_exit_thread, (void*)(""), DEF_STACKSIZE, DEF_THREAD_PRIORITY_NORMAL, 1, false);

	while(sapp4_already_init)
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
				Draw(sapp4_status, 0, 20, 0.65, 0.65, color);

				Draw_apply_draw();
			}
			else
				gspWaitForVBlank();
		}
		else
			usleep(20000);
	}

	Util_log_save(DEF_SAPP4_EXIT_STR, "threadJoin()...", threadJoin(sapp4_exit_thread, DEF_THREAD_WAIT_TIME));	
	threadFree(sapp4_exit_thread);
	Util_remove_watch(&sapp4_status);
	var_need_reflesh = true;

	Util_log_save(DEF_SAPP4_EXIT_STR, "Exited.");
}

void Sapp4_main(void)
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

			Draw(sapp4_msg[0], 0, 20, 0.5, 0.5, color);
			if(Util_log_query_log_show_flag())
				Util_log_draw();

			Draw_top_ui();

			if(var_3d_mode)
			{
				Draw_screen_ready(2, back_color);

				if(Util_log_query_log_show_flag())
					Util_log_draw();

				Draw_top_ui();
			}
		}
		
		if(var_turn_on_bottom_lcd)
		{
			Draw_screen_ready(1, back_color);

			Draw(DEF_SAPP4_VER, 0, 0, 0.4, 0.4, DEF_DRAW_GREEN);

			if(Util_err_query_error_show_flag())
				Util_err_draw();

			Draw_bot_ui();
		}

		Draw_apply_draw();
	}
	else
		gspWaitForVBlank();
}
