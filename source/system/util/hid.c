//Includes.
#include "system/util/hid.h"

#include <stdbool.h>
#include <stdint.h>

#include "system/draw/draw.h"
#include "system/util/err_types.h"
#include "system/util/log.h"
#include "system/util/thread_types.h"

//Defines.
//N/A.

//Typedefs.
//N/A.

//Prototypes.
void Util_hid_scan_hid_thread(void* arg);

//Variables.
static bool util_hid_thread_run = false;
static bool util_hid_init = false;
static bool util_hid_key_A_pressed = false;
static bool util_hid_key_B_pressed = false;
static bool util_hid_key_X_pressed = false;
static bool util_hid_key_Y_pressed = false;
static bool util_hid_key_C_UP_pressed = false;
static bool util_hid_key_C_RIGHT_pressed = false;
static bool util_hid_key_C_DOWN_pressed = false;
static bool util_hid_key_C_LEFT_pressed = false;
static bool util_hid_key_D_UP_pressed = false;
static bool util_hid_key_D_RIGHT_pressed = false;
static bool util_hid_key_D_DOWN_pressed = false;
static bool util_hid_key_D_LEFT_pressed = false;
static bool util_hid_key_L_pressed = false;
static bool util_hid_key_R_pressed = false;
static bool util_hid_key_ZL_pressed = false;
static bool util_hid_key_ZR_pressed = false;
static bool util_hid_key_START_pressed = false;
static bool util_hid_key_SELECT_pressed = false;
static bool util_hid_key_CS_UP_pressed = false;
static bool util_hid_key_CS_DOWN_pressed = false;
static bool util_hid_key_CS_RIGHT_pressed = false;
static bool util_hid_key_CS_LEFT_pressed = false;
static bool util_hid_key_touch_pressed = false;
static bool util_hid_key_any_pressed = false;
static bool util_hid_key_A_held = false;
static bool util_hid_key_B_held = false;
static bool util_hid_key_X_held = false;
static bool util_hid_key_Y_held = false;
static bool util_hid_key_C_UP_held = false;
static bool util_hid_key_C_DOWN_held = false;
static bool util_hid_key_C_RIGHT_held = false;
static bool util_hid_key_C_LEFT_held = false;
static bool util_hid_key_D_UP_held = false;
static bool util_hid_key_D_DOWN_held = false;
static bool util_hid_key_D_RIGHT_held = false;
static bool util_hid_key_D_LEFT_held = false;
static bool util_hid_key_L_held = false;
static bool util_hid_key_R_held = false;
static bool util_hid_key_ZL_held = false;
static bool util_hid_key_ZR_held = false;
static bool util_hid_key_START_held = false;
static bool util_hid_key_SELECT_held = false;
static bool util_hid_key_CS_UP_held = false;
static bool util_hid_key_CS_DOWN_held = false;
static bool util_hid_key_CS_RIGHT_held = false;
static bool util_hid_key_CS_LEFT_held = false;
static bool util_hid_key_touch_held = false;
static bool util_hid_key_any_held = false;
static bool util_hid_key_A_released = false;
static bool util_hid_key_B_released = false;
static bool util_hid_key_X_released = false;
static bool util_hid_key_Y_released = false;
static bool util_hid_key_C_UP_released = false;
static bool util_hid_key_C_RIGHT_released = false;
static bool util_hid_key_C_DOWN_released = false;
static bool util_hid_key_C_LEFT_released = false;
static bool util_hid_key_D_UP_released = false;
static bool util_hid_key_D_RIGHT_released = false;
static bool util_hid_key_D_DOWN_released = false;
static bool util_hid_key_D_LEFT_released = false;
static bool util_hid_key_L_released = false;
static bool util_hid_key_R_released = false;
static bool util_hid_key_ZL_released = false;
static bool util_hid_key_ZR_released = false;
static bool util_hid_key_START_released = false;
static bool util_hid_key_SELECT_released = false;
static bool util_hid_key_CS_UP_released = false;
static bool util_hid_key_CS_DOWN_released = false;
static bool util_hid_key_CS_RIGHT_released = false;
static bool util_hid_key_CS_LEFT_released = false;
static bool util_hid_key_touch_released = false;
static bool util_hid_key_any_released = false;
static int util_hid_cpad_pos_x = 0;
static int util_hid_cpad_pos_y = 0;
static int util_hid_touch_pos_x = 0;
static int util_hid_pre_touch_pos_x = 0;
static int util_hid_touch_pos_x_moved = 0;
static int util_hid_touch_pos_y = 0;
static int util_hid_pre_touch_pos_y = 0;
static int util_hid_touch_pos_y_moved = 0;
static int util_hid_held_time = 0;
static double util_hid_afk_time = 0;
static uint64_t util_hid_ts = 0;
static void (*util_hid_callbacks[DEF_HID_NUM_OF_CALLBACKS])(void) = { 0, };
static Thread util_hid_scan_thread = NULL;
static LightLock util_hid_callback_mutex = 1;//Initially unlocked state.

//Code.
uint32_t Util_hid_init(void)
{
	uint32_t result = DEF_ERR_OTHER;

	if(util_hid_init)
		goto already_inited;

	for(uint16_t i = 0; i < DEF_HID_NUM_OF_CALLBACKS; i++)
		util_hid_callbacks[i] = NULL;

	result = hidInit();
	if(result != DEF_SUCCESS)
	{
		DEF_LOG_RESULT(hidInit, false, result);
		goto nintendo_api_failed;
	}

	util_hid_thread_run = true;
	util_hid_scan_thread = threadCreate(Util_hid_scan_hid_thread, NULL, DEF_THREAD_STACKSIZE, DEF_THREAD_PRIORITY_REALTIME, 0, false);
	if(!util_hid_scan_thread)
	{
		result = DEF_ERR_OTHER;
		DEF_LOG_RESULT(threadCreate, false, result);
		goto nintendo_api_failed_0;
	}

	util_hid_init = true;
	return DEF_SUCCESS;

	already_inited:
	return DEF_ERR_ALREADY_INITIALIZED;

	nintendo_api_failed_0:
	hidExit();
	//Fallthrough.

	nintendo_api_failed:
	return result;
}

void Util_hid_exit(void)
{
	if(!util_hid_init)
		return;

	util_hid_init = false;
	util_hid_thread_run = false;
	threadJoin(util_hid_scan_thread, DEF_THREAD_WAIT_TIME);
	threadFree(util_hid_scan_thread);
	hidExit();
}

bool Util_hid_is_pressed(Hid_info hid_state, Draw_image_data image)
{
	if(!util_hid_init)
		return false;
	else if(hid_state.p_touch && hid_state.touch_x >= image.x && hid_state.touch_x <= (image.x + image.x_size - 1)
	&& hid_state.touch_y >= image.y && hid_state.touch_y <= (image.y + image.y_size - 1))
		return true;
	else
		return false;
}

bool Util_hid_is_held(Hid_info hid_state, Draw_image_data image)
{
	if(!util_hid_init)
		return false;
	else if(hid_state.h_touch && hid_state.touch_x >= image.x && hid_state.touch_x <= (image.x + image.x_size - 1)
	&& hid_state.touch_y >= image.y && hid_state.touch_y <= (image.y + image.y_size - 1))
		return true;
	else
		return false;
}

bool Util_hid_is_released(Hid_info hid_state, Draw_image_data image)
{
	if(!util_hid_init)
		return false;
	else if(hid_state.r_touch && hid_state.touch_x >= image.x && hid_state.touch_x <= (image.x + image.x_size - 1)
	&& hid_state.touch_y >= image.y && hid_state.touch_y <= (image.y + image.y_size - 1))
		return true;
	else
		return false;
}

uint32_t Util_hid_query_key_state(Hid_info* out_key_state)
{
	if(!util_hid_init)
		goto not_inited;

	out_key_state->p_a = util_hid_key_A_pressed;
	out_key_state->p_b = util_hid_key_B_pressed;
	out_key_state->p_x = util_hid_key_X_pressed;
	out_key_state->p_y = util_hid_key_Y_pressed;
	out_key_state->p_c_up = util_hid_key_C_UP_pressed;
	out_key_state->p_c_down = util_hid_key_C_DOWN_pressed;
	out_key_state->p_c_left = util_hid_key_C_LEFT_pressed;
	out_key_state->p_c_right = util_hid_key_C_RIGHT_pressed;
	out_key_state->p_d_up = util_hid_key_D_UP_pressed;
	out_key_state->p_d_down = util_hid_key_D_DOWN_pressed;
	out_key_state->p_d_left = util_hid_key_D_LEFT_pressed;
	out_key_state->p_d_right = util_hid_key_D_RIGHT_pressed;
	out_key_state->p_l = util_hid_key_L_pressed;
	out_key_state->p_r = util_hid_key_R_pressed;
	out_key_state->p_zl = util_hid_key_ZL_pressed;
	out_key_state->p_zr = util_hid_key_ZR_pressed;
	out_key_state->p_start = util_hid_key_START_pressed;
	out_key_state->p_select = util_hid_key_SELECT_pressed;
	out_key_state->p_cs_up = util_hid_key_CS_UP_pressed;
	out_key_state->p_cs_down = util_hid_key_CS_DOWN_pressed;
	out_key_state->p_cs_left = util_hid_key_CS_LEFT_pressed;
	out_key_state->p_cs_right = util_hid_key_CS_RIGHT_pressed;
	out_key_state->p_touch = util_hid_key_touch_pressed;
	out_key_state->p_any = util_hid_key_any_pressed;
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
	out_key_state->r_a = util_hid_key_A_released;
	out_key_state->r_b = util_hid_key_B_released;
	out_key_state->r_x = util_hid_key_X_released;
	out_key_state->r_y = util_hid_key_Y_released;
	out_key_state->r_c_up = util_hid_key_C_UP_released;
	out_key_state->r_c_right = util_hid_key_C_RIGHT_released;
	out_key_state->r_c_down = util_hid_key_C_DOWN_released;
	out_key_state->r_c_left = util_hid_key_C_LEFT_released;
	out_key_state->r_d_up = util_hid_key_D_UP_released;
	out_key_state->r_d_right = util_hid_key_D_RIGHT_released;
	out_key_state->r_d_down = util_hid_key_D_DOWN_released;
	out_key_state->r_d_left = util_hid_key_D_LEFT_released;
	out_key_state->r_l = util_hid_key_L_released;
	out_key_state->r_r = util_hid_key_R_released;
	out_key_state->r_zl = util_hid_key_ZL_released;
	out_key_state->r_zr = util_hid_key_ZR_released;
	out_key_state->r_start = util_hid_key_START_released;
	out_key_state->r_select = util_hid_key_SELECT_released;
	out_key_state->r_cs_up = util_hid_key_CS_UP_released;
	out_key_state->r_cs_down = util_hid_key_CS_DOWN_released;
	out_key_state->r_cs_right = util_hid_key_CS_RIGHT_released;
	out_key_state->r_cs_left = util_hid_key_CS_LEFT_released;
	out_key_state->r_touch = util_hid_key_touch_released;
	out_key_state->cpad_x = util_hid_cpad_pos_x;
	out_key_state->cpad_y = util_hid_cpad_pos_y;
	out_key_state->touch_x = util_hid_touch_pos_x;
	out_key_state->touch_y = util_hid_touch_pos_y;
	out_key_state->touch_x_move = util_hid_touch_pos_x_moved;
	out_key_state->touch_y_move = util_hid_touch_pos_y_moved;
	out_key_state->held_time = util_hid_held_time;
	out_key_state->ts = util_hid_ts;
	out_key_state->afk_time_ms = (uint32_t)util_hid_afk_time;

	return DEF_SUCCESS;

	not_inited:
	return DEF_ERR_NOT_INITIALIZED;
}

void Util_hid_reset_afk_time(void)
{
	if(!util_hid_init)
		return;

	LightLock_Lock(&util_hid_callback_mutex);
	util_hid_afk_time = 0;
	LightLock_Unlock(&util_hid_callback_mutex);
}

bool Util_hid_add_callback(void (*const callback)(void))
{
	if(!util_hid_init)
		return false;

	LightLock_Lock(&util_hid_callback_mutex);

	for(uint16_t i = 0; i < DEF_HID_NUM_OF_CALLBACKS; i++)
	{
		if(util_hid_callbacks[i] == callback)
			goto success;//Already exist.
	}

	for(uint16_t i = 0; i < DEF_HID_NUM_OF_CALLBACKS; i++)
	{
		if(!util_hid_callbacks[i])
		{
			util_hid_callbacks[i] = callback;
			goto success;
		}
	}

	//No free spaces left.
	LightLock_Unlock(&util_hid_callback_mutex);
	return false;

	success:
	LightLock_Unlock(&util_hid_callback_mutex);
	return true;
}

void Util_hid_remove_callback(void (*const callback)(void))
{
	if(!util_hid_init)
		return;

	LightLock_Lock(&util_hid_callback_mutex);

	for(uint16_t i = 0; i < DEF_HID_NUM_OF_CALLBACKS; i++)
	{
		if(util_hid_callbacks[i] == callback)
		{
			util_hid_callbacks[i] = NULL;
			break;
		}
	}

	LightLock_Unlock(&util_hid_callback_mutex);
}

void Util_hid_scan_hid_thread(void* arg)
{
	(void)arg;
	DEF_LOG_STRING("Thread started.");
	TickCounter counter = { 0, };

	osTickCounterStart(&counter);
	osTickCounterUpdate(&counter);

	while (util_hid_thread_run)
	{
		uint32_t key_pressed = 0;
		uint32_t key_held = 0;
		uint32_t key_released = 0;
		touchPosition touch_pos = { 0, };
		circlePosition circle_pos = { 0, };

		hidScanInput();
		hidTouchRead(&touch_pos);
		hidCircleRead(&circle_pos);
		key_held = hidKeysHeld();
		key_pressed = hidKeysDown();
		key_released = hidKeysUp();

		util_hid_ts = osGetTime();

		util_hid_key_A_pressed = (key_pressed & KEY_A);
		util_hid_key_B_pressed = (key_pressed & KEY_B);
		util_hid_key_Y_pressed = (key_pressed & KEY_Y);
		util_hid_key_X_pressed = (key_pressed & KEY_X);

		util_hid_key_D_UP_pressed = (key_pressed & KEY_DUP);
		util_hid_key_D_DOWN_pressed = (key_pressed & KEY_DDOWN);
		util_hid_key_D_RIGHT_pressed = (key_pressed & KEY_DRIGHT);
		util_hid_key_D_LEFT_pressed = (key_pressed & KEY_DLEFT);

		util_hid_key_C_UP_pressed = (key_pressed & KEY_CPAD_UP);
		util_hid_key_C_DOWN_pressed = (key_pressed & KEY_CPAD_DOWN);
		util_hid_key_C_RIGHT_pressed = (key_pressed & KEY_CPAD_RIGHT);
		util_hid_key_C_LEFT_pressed = (key_pressed & KEY_CPAD_LEFT);

		util_hid_key_CS_UP_pressed = (key_pressed & KEY_CSTICK_UP);
		util_hid_key_CS_DOWN_pressed = (key_pressed & KEY_CSTICK_DOWN);
		util_hid_key_CS_RIGHT_pressed = (key_pressed & KEY_CSTICK_RIGHT);
		util_hid_key_CS_LEFT_pressed = (key_pressed & KEY_CSTICK_LEFT);

		util_hid_key_SELECT_pressed = (key_pressed & KEY_SELECT);
		util_hid_key_START_pressed = (key_pressed & KEY_START);

		util_hid_key_L_pressed = (key_pressed & KEY_L);
		util_hid_key_R_pressed = (key_pressed & KEY_R);
		util_hid_key_ZL_pressed = (key_pressed & KEY_ZL);
		util_hid_key_ZR_pressed = (key_pressed & KEY_ZR);


		util_hid_key_A_held = (key_held & KEY_A);
		util_hid_key_B_held = (key_held & KEY_B);
		util_hid_key_Y_held = (key_held & KEY_Y);
		util_hid_key_X_held = (key_held & KEY_X);

		util_hid_key_D_UP_held = (key_held & KEY_DUP);
		util_hid_key_D_DOWN_held = (key_held & KEY_DDOWN);
		util_hid_key_D_RIGHT_held = (key_held & KEY_DRIGHT);
		util_hid_key_D_LEFT_held = (key_held & KEY_DLEFT);

		util_hid_key_C_UP_held = (key_held & KEY_CPAD_UP);
		util_hid_key_C_DOWN_held = (key_held & KEY_CPAD_DOWN);
		util_hid_key_C_RIGHT_held = (key_held & KEY_CPAD_RIGHT);
		util_hid_key_C_LEFT_held = (key_held & KEY_CPAD_LEFT);

		util_hid_key_CS_UP_held = (key_held & KEY_CSTICK_UP);
		util_hid_key_CS_DOWN_held = (key_held & KEY_CSTICK_DOWN);
		util_hid_key_CS_RIGHT_held = (key_held & KEY_CSTICK_RIGHT);
		util_hid_key_CS_LEFT_held = (key_held & KEY_CSTICK_LEFT);

		util_hid_key_SELECT_held = (key_held & KEY_SELECT);
		util_hid_key_START_held = (key_held & KEY_START);

		util_hid_key_L_held = (key_held & KEY_L);
		util_hid_key_R_held = (key_held & KEY_R);
		util_hid_key_ZL_held = (key_held & KEY_ZL);
		util_hid_key_ZR_held = (key_held & KEY_ZR);


		util_hid_key_A_released = (key_released & KEY_A);
		util_hid_key_B_released = (key_released & KEY_B);
		util_hid_key_Y_released = (key_released & KEY_Y);
		util_hid_key_X_released = (key_released & KEY_X);

		util_hid_key_D_UP_released = (key_released & KEY_DUP);
		util_hid_key_D_DOWN_released = (key_released & KEY_DDOWN);
		util_hid_key_D_RIGHT_released = (key_released & KEY_DRIGHT);
		util_hid_key_D_LEFT_released = (key_released & KEY_DLEFT);

		util_hid_key_C_UP_released = (key_released & KEY_CPAD_UP);
		util_hid_key_C_DOWN_released = (key_released & KEY_CPAD_DOWN);
		util_hid_key_C_RIGHT_released = (key_released & KEY_CPAD_RIGHT);
		util_hid_key_C_LEFT_released = (key_released & KEY_CPAD_LEFT);

		util_hid_key_CS_UP_released = (key_released & KEY_CSTICK_UP);
		util_hid_key_CS_DOWN_released = (key_released & KEY_CSTICK_DOWN);
		util_hid_key_CS_RIGHT_released = (key_released & KEY_CSTICK_RIGHT);
		util_hid_key_CS_LEFT_released = (key_released & KEY_CSTICK_LEFT);

		util_hid_key_SELECT_released = (key_released & KEY_SELECT);
		util_hid_key_START_released = (key_released & KEY_START);

		util_hid_key_L_released = (key_released & KEY_L);
		util_hid_key_R_released = (key_released & KEY_R);
		util_hid_key_ZL_released = (key_released & KEY_ZL);
		util_hid_key_ZR_released = (key_released & KEY_ZR);

		if (key_pressed & KEY_TOUCH || key_held & KEY_TOUCH || key_released & KEY_TOUCH)
		{
			if (key_pressed & KEY_TOUCH)
			{
				util_hid_key_touch_pressed = true;
				util_hid_key_touch_held = false;
				util_hid_key_touch_released = false;
				util_hid_pre_touch_pos_x = touch_pos.px;
				util_hid_pre_touch_pos_y = touch_pos.py;
				util_hid_touch_pos_x = touch_pos.px;
				util_hid_touch_pos_y = touch_pos.py;
			}
			else if (key_held & KEY_TOUCH)
			{
				util_hid_key_touch_held = true;
				util_hid_key_touch_pressed = false;
				util_hid_key_touch_released = false;
				util_hid_touch_pos_x = touch_pos.px;
				util_hid_touch_pos_y = touch_pos.py;
				util_hid_touch_pos_x_moved = util_hid_pre_touch_pos_x - util_hid_touch_pos_x;
				util_hid_touch_pos_y_moved = util_hid_pre_touch_pos_y - util_hid_touch_pos_y;
				util_hid_pre_touch_pos_x = touch_pos.px;
				util_hid_pre_touch_pos_y = touch_pos.py;
			}
			else if(key_released & KEY_TOUCH)
			{
				util_hid_key_touch_released = true;
				util_hid_key_touch_pressed = false;
				util_hid_key_touch_held = false;
				util_hid_touch_pos_x = util_hid_pre_touch_pos_x;
				util_hid_touch_pos_y = util_hid_pre_touch_pos_y;
			}
		}
		else
		{
			util_hid_key_touch_pressed = false;
			util_hid_key_touch_held = false;
			util_hid_key_touch_released = false;
			util_hid_touch_pos_x = -1;
			util_hid_touch_pos_y = -1;
			util_hid_pre_touch_pos_x = -1;
			util_hid_pre_touch_pos_y = -1;
			util_hid_touch_pos_x_moved = 0;
			util_hid_touch_pos_y_moved = 0;
		}

		util_hid_cpad_pos_x = circle_pos.dx;
		util_hid_cpad_pos_y = circle_pos.dy;

		if (util_hid_key_D_UP_held || util_hid_key_D_DOWN_held || util_hid_key_D_RIGHT_held || util_hid_key_D_LEFT_held
			|| util_hid_key_C_UP_held || util_hid_key_C_DOWN_held || util_hid_key_C_RIGHT_held || util_hid_key_C_LEFT_held
			|| util_hid_key_CS_UP_held || util_hid_key_CS_DOWN_held || util_hid_key_CS_RIGHT_held || util_hid_key_CS_LEFT_held
			|| util_hid_key_touch_held)
			util_hid_held_time++;
		else
			util_hid_held_time = 0;

		LightLock_Lock(&util_hid_callback_mutex);

		osTickCounterUpdate(&counter);

		if (key_pressed != 0 || key_held != 0 || key_released != 0)
		{
			if(key_pressed != 0)
				util_hid_key_any_pressed = true;
			if(key_held != 0)
				util_hid_key_any_held = true;
			if(key_released != 0)
				util_hid_key_any_released = true;

			util_hid_afk_time = 0;
		}
		else
			util_hid_afk_time += osTickCounterRead(&counter);

		//Call callback functions.
		for(uint16_t i = 0; i < DEF_HID_NUM_OF_CALLBACKS; i++)
		{
			if(util_hid_callbacks[i])
				util_hid_callbacks[i]();
		}

		LightLock_Unlock(&util_hid_callback_mutex);

		gspWaitForVBlank();
	}
	DEF_LOG_STRING("Thread exit.");
	threadExit(0);
}
