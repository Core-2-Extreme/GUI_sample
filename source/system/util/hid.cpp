#include "headers.hpp"

bool util_hid_thread_run = false;
bool util_hid_key_A_press = false;
bool util_hid_key_B_press = false;
bool util_hid_key_X_press = false;
bool util_hid_key_Y_press = false;
bool util_hid_key_C_UP_press = false;
bool util_hid_key_C_RIGHT_press = false;
bool util_hid_key_C_DOWN_press = false;
bool util_hid_key_C_LEFT_press = false;
bool util_hid_key_D_UP_press = false;
bool util_hid_key_D_RIGHT_press = false;
bool util_hid_key_D_DOWN_press = false;
bool util_hid_key_D_LEFT_press = false;
bool util_hid_key_L_press = false;
bool util_hid_key_R_press = false;
bool util_hid_key_ZL_press = false;
bool util_hid_key_ZR_press = false;
bool util_hid_key_START_press = false;
bool util_hid_key_SELECT_press = false;
bool util_hid_key_CS_UP_press = false;
bool util_hid_key_CS_DOWN_press = false;
bool util_hid_key_CS_RIGHT_press = false;
bool util_hid_key_CS_LEFT_press = false;
bool util_hid_key_touch_press = false;
bool util_hid_key_any_press = false;
bool util_hid_key_A_held = false;
bool util_hid_key_B_held = false;
bool util_hid_key_X_held = false;
bool util_hid_key_Y_held = false;
bool util_hid_key_C_UP_held = false;
bool util_hid_key_C_DOWN_held = false;
bool util_hid_key_C_RIGHT_held = false;
bool util_hid_key_C_LEFT_held = false;
bool util_hid_key_D_UP_held = false;
bool util_hid_key_D_DOWN_held = false;
bool util_hid_key_D_RIGHT_held = false;
bool util_hid_key_D_LEFT_held = false;
bool util_hid_key_L_held = false;
bool util_hid_key_R_held = false;
bool util_hid_key_ZL_held = false;
bool util_hid_key_ZR_held = false;
bool util_hid_key_START_held = false;
bool util_hid_key_SELECT_held = false;
bool util_hid_key_CS_UP_held = false;
bool util_hid_key_CS_DOWN_held = false;
bool util_hid_key_CS_RIGHT_held = false;
bool util_hid_key_CS_LEFT_held = false;
bool util_hid_key_touch_held = false;
bool util_hid_key_any_held = false;
int util_hid_cpad_pos_x = 0;
int util_hid_cpad_pos_y = 0;
int util_hid_touch_pos_x = 0;
int util_hid_pre_touch_pos_x = 0;
int util_hid_touch_pos_x_moved = 0;
int util_hid_touch_pos_y = 0;
int util_hid_pre_touch_pos_y = 0;
int util_hid_touch_pos_y_moved = 0;
int util_hid_held_time = 0;
int util_hid_count = 0;
Thread util_hid_scan_thread;

void Util_hid_init(void)
{
	Util_log_save(DEF_HID_INIT_STR, "Initializing...");

	util_hid_thread_run = true;
	util_hid_scan_thread = threadCreate(Util_hid_scan_hid_thread, (void*)(""), DEF_STACKSIZE, DEF_THREAD_PRIORITY_REALTIME, 0, false);

	Util_log_save(DEF_HID_INIT_STR, "Initialized.");
}

void Util_hid_exit(void)
{
	Util_log_save(DEF_HID_EXIT_STR, "Exiting...");

	util_hid_thread_run = false;
	threadJoin(util_hid_scan_thread, DEF_THREAD_WAIT_TIME);
	threadFree(util_hid_scan_thread);

	Util_log_save(DEF_HID_EXIT_STR, "Exited.");
}

bool Util_hid_is_pressed(Hid_info hid_state, Image_data image)
{
	if(image.x < 0 || image.y < 0 || image.x_size < 1 || image.y_size < 1)
		return false;
	else if(hid_state.p_touch && hid_state.touch_x >= image.x && hid_state.touch_x <= (image.x + image.x_size - 1)
	&& hid_state.touch_y >= image.y && hid_state.touch_y <= (image.y + image.y_size - 1))
		return true;
	else
		return false;
}

bool Util_hid_is_held(Hid_info hid_state, Image_data image)
{
	if(image.x < 0 || image.y < 0 || image.x_size < 1 || image.y_size < 1)
		return false;
	else if(hid_state.h_touch && hid_state.touch_x >= image.x && hid_state.touch_x <= (image.x + image.x_size - 1)
	&& hid_state.touch_y >= image.y && hid_state.touch_y <= (image.y + image.y_size - 1))
		return true;
	else
		return false;
}

void Util_hid_query_key_state(Hid_info* out_key_state)
{
	out_key_state->p_a = util_hid_key_A_press;
	out_key_state->p_b = util_hid_key_B_press;
	out_key_state->p_x = util_hid_key_X_press;
	out_key_state->p_y = util_hid_key_Y_press;
	out_key_state->p_c_up = util_hid_key_C_UP_press;
	out_key_state->p_c_down = util_hid_key_C_DOWN_press;
	out_key_state->p_c_left = util_hid_key_C_LEFT_press;
	out_key_state->p_c_right = util_hid_key_C_RIGHT_press;
	out_key_state->p_d_up = util_hid_key_D_UP_press;
	out_key_state->p_d_down = util_hid_key_D_DOWN_press;
	out_key_state->p_d_left = util_hid_key_D_LEFT_press;
	out_key_state->p_d_right = util_hid_key_D_RIGHT_press;
	out_key_state->p_l = util_hid_key_L_press;
	out_key_state->p_r = util_hid_key_R_press;
	out_key_state->p_zl = util_hid_key_ZL_press;
	out_key_state->p_zr = util_hid_key_ZR_press;
	out_key_state->p_start = util_hid_key_START_press;
	out_key_state->p_select = util_hid_key_SELECT_press;
	out_key_state->p_cs_up = util_hid_key_CS_UP_press;
	out_key_state->p_cs_down = util_hid_key_CS_DOWN_press;
	out_key_state->p_cs_left = util_hid_key_CS_LEFT_press;
	out_key_state->p_cs_right = util_hid_key_CS_RIGHT_press;
	out_key_state->p_touch = util_hid_key_touch_press;
	out_key_state->p_any = util_hid_key_any_press;
	out_key_state->h_a = util_hid_key_A_held;
	out_key_state->h_b = util_hid_key_B_held;
	out_key_state->h_x = util_hid_key_X_held;
	out_key_state->h_y = util_hid_key_Y_held;
	out_key_state->h_c_up = util_hid_key_C_UP_held;
	out_key_state->h_c_down = util_hid_key_C_DOWN_held;
	out_key_state->h_c_left = util_hid_key_C_LEFT_held;
	out_key_state->h_c_right = util_hid_key_C_RIGHT_held;
	out_key_state->h_d_up = util_hid_key_D_UP_held;
	out_key_state->h_d_down = util_hid_key_D_DOWN_held;
	out_key_state->h_d_left = util_hid_key_D_LEFT_held;
	out_key_state->h_d_right = util_hid_key_D_RIGHT_held;
	out_key_state->h_l = util_hid_key_L_held;
	out_key_state->h_r = util_hid_key_R_held;
	out_key_state->h_zl = util_hid_key_ZL_held;
	out_key_state->h_zr = util_hid_key_ZR_held;
	out_key_state->h_start = util_hid_key_START_held;
	out_key_state->h_select = util_hid_key_SELECT_held;
	out_key_state->h_cs_up = util_hid_key_CS_UP_held;
	out_key_state->h_cs_down = util_hid_key_CS_DOWN_held;
	out_key_state->h_cs_left = util_hid_key_CS_LEFT_held;
	out_key_state->h_cs_right = util_hid_key_CS_RIGHT_held;
	out_key_state->h_touch = util_hid_key_touch_held;
	out_key_state->h_any = util_hid_key_any_held;
	out_key_state->cpad_x = util_hid_cpad_pos_x;
	out_key_state->cpad_y = util_hid_cpad_pos_y;
	out_key_state->touch_x = util_hid_touch_pos_x;
	out_key_state->touch_y = util_hid_touch_pos_y;
	out_key_state->touch_x_move = util_hid_touch_pos_x_moved;
	out_key_state->touch_y_move = util_hid_touch_pos_y_moved;
	out_key_state->held_time = util_hid_held_time;
	out_key_state->count = util_hid_count;
}

void Util_hid_key_flag_reset(void)
{
	util_hid_key_A_press = false;
	util_hid_key_B_press = false;
	util_hid_key_X_press = false;
	util_hid_key_Y_press = false;
	util_hid_key_C_UP_press = false;
	util_hid_key_C_DOWN_press = false;
	util_hid_key_C_RIGHT_press = false;
	util_hid_key_C_LEFT_press = false;
	util_hid_key_D_UP_press = false;
	util_hid_key_D_DOWN_press = false;
	util_hid_key_D_RIGHT_press = false;
	util_hid_key_D_LEFT_press = false;
	util_hid_key_L_press = false;
	util_hid_key_R_press = false;
	util_hid_key_ZL_press = false;
	util_hid_key_ZR_press = false;
	util_hid_key_START_press = false;
	util_hid_key_SELECT_press = false;
	util_hid_key_CS_UP_press = false;
	util_hid_key_CS_DOWN_press = false;
	util_hid_key_CS_RIGHT_press = false;
	util_hid_key_CS_LEFT_press = false;
	util_hid_key_touch_press = false;
	util_hid_key_any_press = false;
	util_hid_key_A_held = false;
	util_hid_key_B_held = false;
	util_hid_key_X_held = false;
	util_hid_key_Y_held = false;
	util_hid_key_C_UP_held = false;
	util_hid_key_C_DOWN_held = false;
	util_hid_key_C_RIGHT_held = false;
	util_hid_key_C_LEFT_held = false;
	util_hid_key_D_UP_held = false;
	util_hid_key_D_DOWN_held = false;
	util_hid_key_D_RIGHT_held = false;
	util_hid_key_D_LEFT_held = false;
	util_hid_key_L_held = false;
	util_hid_key_R_held = false;
	util_hid_key_ZL_held = false;
	util_hid_key_ZR_held = false;
	util_hid_key_START_held = false;
	util_hid_key_SELECT_held = false;
	util_hid_key_CS_UP_held = false;
	util_hid_key_CS_DOWN_held = false;
	util_hid_key_CS_RIGHT_held = false;
	util_hid_key_CS_LEFT_held = false;
	util_hid_key_touch_held = false;
	util_hid_key_any_held = false;
	util_hid_touch_pos_x = 0;
	util_hid_touch_pos_y = 0;
	util_hid_count = 0;
}

void Util_hid_scan_hid_thread(void* arg)
{
	Util_log_save(DEF_HID_SCAN_THREAD_STR, "Thread started.");

	u32 kDown;
	u32 kHeld;
	touchPosition touch_pos;
	circlePosition circle_pos;
	Result_with_string result;

	while (util_hid_thread_run)
	{
		hidScanInput();
		hidTouchRead(&touch_pos);
		hidCircleRead(&circle_pos);
		kHeld = hidKeysHeld();
		kDown = hidKeysDown();

		util_hid_key_A_press = (kDown & KEY_A);
		util_hid_key_B_press = (kDown & KEY_B);
		util_hid_key_Y_press = (kDown & KEY_Y);
		util_hid_key_X_press = (kDown & KEY_X);

		util_hid_key_D_UP_press = (kDown & KEY_DUP);
		util_hid_key_D_DOWN_press = (kDown & KEY_DDOWN);
		util_hid_key_D_RIGHT_press = (kDown & KEY_DRIGHT);
		util_hid_key_D_LEFT_press = (kDown & KEY_DLEFT);

		util_hid_key_C_UP_press = (kDown & KEY_CPAD_UP);
		util_hid_key_C_DOWN_press = (kDown & KEY_CPAD_DOWN);
		util_hid_key_C_RIGHT_press = (kDown & KEY_CPAD_RIGHT);
		util_hid_key_C_LEFT_press = (kDown & KEY_CPAD_LEFT);

		util_hid_key_CS_UP_press = (kDown & KEY_CSTICK_UP);
		util_hid_key_CS_DOWN_press = (kDown & KEY_CSTICK_DOWN);
		util_hid_key_CS_RIGHT_press = (kDown & KEY_CSTICK_RIGHT);
		util_hid_key_CS_LEFT_press = (kDown & KEY_CSTICK_LEFT);

		util_hid_key_SELECT_press = (kDown & KEY_SELECT);
		util_hid_key_START_press = (kDown & KEY_START);

		util_hid_key_L_press = (kDown & KEY_L);
		util_hid_key_R_press = (kDown & KEY_R);
		util_hid_key_ZL_press = (kDown & KEY_ZL);
		util_hid_key_ZR_press = (kDown & KEY_ZR);

		util_hid_key_A_held = (kHeld & KEY_A);
		util_hid_key_B_held = (kHeld & KEY_B);
		util_hid_key_Y_held = (kHeld & KEY_Y);
		util_hid_key_X_held = (kHeld & KEY_X);

		util_hid_key_D_UP_held = (kHeld & KEY_DUP);
		util_hid_key_D_DOWN_held = (kHeld & KEY_DDOWN);
		util_hid_key_D_RIGHT_held = (kHeld & KEY_DRIGHT);
		util_hid_key_D_LEFT_held = (kHeld & KEY_DLEFT);

		util_hid_key_C_UP_held = (kHeld & KEY_CPAD_UP);
		util_hid_key_C_DOWN_held = (kHeld & KEY_CPAD_DOWN);
		util_hid_key_C_RIGHT_held = (kHeld & KEY_CPAD_RIGHT);
		util_hid_key_C_LEFT_held = (kHeld & KEY_CPAD_LEFT);

		util_hid_key_CS_UP_held = (kHeld & KEY_CSTICK_UP);
		util_hid_key_CS_DOWN_held = (kHeld & KEY_CSTICK_DOWN);
		util_hid_key_CS_RIGHT_held = (kHeld & KEY_CSTICK_RIGHT);
		util_hid_key_CS_LEFT_held = (kHeld & KEY_CSTICK_LEFT);

		util_hid_key_SELECT_held = (kHeld & KEY_SELECT);
		util_hid_key_START_held = (kHeld & KEY_START);

		util_hid_key_L_held = (kHeld & KEY_L);
		util_hid_key_R_held = (kHeld & KEY_R);
		util_hid_key_ZL_held = (kHeld & KEY_ZL);
		util_hid_key_ZR_held = (kHeld & KEY_ZR);

		if (kDown & KEY_TOUCH || kHeld & KEY_TOUCH)
		{
			if (kDown & KEY_TOUCH)
			{
				util_hid_key_touch_press = true;
				util_hid_key_touch_held = false;
				util_hid_pre_touch_pos_x = touch_pos.px;
				util_hid_pre_touch_pos_y = touch_pos.py;
				util_hid_touch_pos_x = touch_pos.px;
				util_hid_touch_pos_y = touch_pos.py;
			}
			else if (kHeld & KEY_TOUCH)
			{
				util_hid_key_touch_held = true;
				util_hid_key_touch_press = false;
				util_hid_touch_pos_x = touch_pos.px;
				util_hid_touch_pos_y = touch_pos.py;
				util_hid_touch_pos_x_moved = util_hid_pre_touch_pos_x - util_hid_touch_pos_x;
				util_hid_touch_pos_y_moved = util_hid_pre_touch_pos_y - util_hid_touch_pos_y;
				util_hid_pre_touch_pos_x = touch_pos.px;
				util_hid_pre_touch_pos_y = touch_pos.py;
			}
		}
		else
		{
			util_hid_key_touch_press = false;
			util_hid_key_touch_held = false;
			util_hid_touch_pos_x = -1;
			util_hid_touch_pos_y = -1;
			util_hid_touch_pos_x_moved = 0;
			util_hid_touch_pos_y_moved = 0;
			util_hid_pre_touch_pos_x = 0;
			util_hid_pre_touch_pos_y = 0;
		}

		util_hid_cpad_pos_x = circle_pos.dx;
		util_hid_cpad_pos_y = circle_pos.dy;

		if (util_hid_key_A_press || util_hid_key_B_press || util_hid_key_X_press || util_hid_key_Y_press || 
			util_hid_key_C_UP_press || util_hid_key_C_RIGHT_press || util_hid_key_C_DOWN_press || util_hid_key_C_LEFT_press || 
			util_hid_key_D_UP_press || util_hid_key_D_RIGHT_press || util_hid_key_D_DOWN_press || util_hid_key_D_LEFT_press || 
			util_hid_key_L_press || util_hid_key_R_press || util_hid_key_ZL_press || util_hid_key_ZR_press || 
			util_hid_key_START_press || util_hid_key_SELECT_press || util_hid_key_CS_UP_press || util_hid_key_CS_DOWN_press || 
			util_hid_key_CS_RIGHT_press || util_hid_key_CS_LEFT_press || util_hid_key_touch_press ||  util_hid_key_A_held || 
			util_hid_key_B_held || util_hid_key_X_held || util_hid_key_Y_held || util_hid_key_C_UP_held || util_hid_key_C_DOWN_held || 
			util_hid_key_C_RIGHT_held || util_hid_key_C_LEFT_held || util_hid_key_D_UP_held || util_hid_key_D_DOWN_held || 
			util_hid_key_D_RIGHT_held || util_hid_key_D_LEFT_held || util_hid_key_L_held || util_hid_key_R_held || util_hid_key_ZL_held || 
			util_hid_key_ZR_held || util_hid_key_START_held || util_hid_key_SELECT_held || util_hid_key_CS_UP_held || 
			util_hid_key_CS_DOWN_held || util_hid_key_CS_RIGHT_held || util_hid_key_CS_LEFT_held || util_hid_key_touch_held)
		{
			if(util_hid_key_A_press || util_hid_key_B_press || util_hid_key_X_press || util_hid_key_Y_press || 
			util_hid_key_C_UP_press || util_hid_key_C_RIGHT_press || util_hid_key_C_DOWN_press || util_hid_key_C_LEFT_press || 
			util_hid_key_D_UP_press || util_hid_key_D_RIGHT_press || util_hid_key_D_DOWN_press || util_hid_key_D_LEFT_press || 
			util_hid_key_L_press || util_hid_key_R_press || util_hid_key_ZL_press || util_hid_key_ZR_press || 
			util_hid_key_START_press || util_hid_key_SELECT_press || util_hid_key_CS_UP_press || util_hid_key_CS_DOWN_press || 
			util_hid_key_CS_RIGHT_press || util_hid_key_CS_LEFT_press || util_hid_key_touch_press)
				util_hid_key_any_press = true;
			if(util_hid_key_A_held || 
			util_hid_key_B_held || util_hid_key_X_held || util_hid_key_Y_held || util_hid_key_C_UP_held || util_hid_key_C_DOWN_held || 
			util_hid_key_C_RIGHT_held || util_hid_key_C_LEFT_held || util_hid_key_D_UP_held || util_hid_key_D_DOWN_held || 
			util_hid_key_D_RIGHT_held || util_hid_key_D_LEFT_held || util_hid_key_L_held || util_hid_key_R_held || util_hid_key_ZL_held || 
			util_hid_key_ZR_held || util_hid_key_START_held || util_hid_key_SELECT_held || util_hid_key_CS_UP_held || 
			util_hid_key_CS_DOWN_held || util_hid_key_CS_RIGHT_held || util_hid_key_CS_LEFT_held || util_hid_key_touch_held)
				util_hid_key_any_held = true;

			var_afk_time = 0;
		}

		if (util_hid_key_D_UP_held || util_hid_key_D_DOWN_held || util_hid_key_D_RIGHT_held || util_hid_key_D_LEFT_held
			|| util_hid_key_C_UP_held || util_hid_key_C_DOWN_held || util_hid_key_C_RIGHT_held || util_hid_key_C_LEFT_held
			|| util_hid_key_CS_UP_held || util_hid_key_CS_DOWN_held || util_hid_key_CS_RIGHT_held || util_hid_key_CS_LEFT_held
			|| util_hid_key_touch_held)
			util_hid_held_time++;
		else
			util_hid_held_time = 0;

		util_hid_count++;

		gspWaitForVBlank();
	}
	Util_log_save(DEF_HID_SCAN_THREAD_STR, "Thread exit");
	threadExit(0);
}
