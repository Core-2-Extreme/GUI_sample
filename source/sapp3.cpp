#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>

#include "system/menu.hpp"
#include "system/sem.hpp"

extern "C"
{
	#include "system/draw/draw.h"
	#include "system/util/cam.h"
	#include "system/util/converter.h"
	#include "system/util/cpu_usage.h"
	#include "system/util/encoder.h"
	#include "system/util/err.h"
	#include "system/util/expl.h"
	#include "system/util/file.h"
	#include "system/util/hid.h"
	#include "system/util/log.h"
	#include "system/util/mic.h"
	#include "system/util/queue.h"
	#include "system/util/str.h"
	#include "system/util/thread_types.h"
	#include "system/util/util_c.h"
	#include "system/util/watch.h"
}

//Include myself.
#include "sapp3.hpp"


typedef enum
{
	CAM_NONE,

	CAM_DISABLE_REQUEST,
	CAM_ENABLE_REQUEST,
	CAM_TAKE_A_PICTURE_REQUEST,

	CAM_MAX = 0xFF,
} Sapp3_camera_command;

DEF_LOG_ENUM_DEBUG
(
	Sapp3_camera_command,
	CAM_NONE,
	CAM_DISABLE_REQUEST,
	CAM_ENABLE_REQUEST,
	CAM_TAKE_A_PICTURE_REQUEST,
	CAM_MAX
);

typedef enum
{
	CAM_IDLE,
	CAM_ENABLED,
	CAM_SAVING_A_PICTURE,
} Sapp3_camera_state;

DEF_LOG_ENUM_DEBUG
(
	Sapp3_camera_state,
	CAM_IDLE,
	CAM_ENABLED,
	CAM_SAVING_A_PICTURE
);

typedef enum
{
	MIC_NONE,

	MIC_START_RECORDING_REQUEST,
	MIC_STOP_RECORDING_REQUEST,

	MIC_MAX = 0xFF,
} Sapp3_mic_command;

DEF_LOG_ENUM_DEBUG
(
	Sapp3_mic_command,
	MIC_NONE,
	MIC_START_RECORDING_REQUEST,
	MIC_STOP_RECORDING_REQUEST,
	MIC_MAX
);

typedef enum
{
	MIC_IDLE,
	MIC_RECORDING,
	MIC_STOPPING_RECORDING,
} Sapp3_mic_state;

DEF_LOG_ENUM_DEBUG
(
	Sapp3_mic_state,
	MIC_IDLE,
	MIC_RECORDING,
	MIC_STOPPING_RECORDING
);


bool sapp3_main_run = false;
bool sapp3_thread_run = false;
bool sapp3_already_init = false;
bool sapp3_thread_suspend = true;
uint8_t sapp3_camera_buffer_index = 0;
Thread sapp3_init_thread = NULL, sapp3_exit_thread = NULL, sapp3_camera_thread = NULL, sapp3_mic_thread = NULL;
Draw_image_data sapp3_camera_image[2] = { 0, };
Queue_data sapp3_camera_command_queue = { 0, }, sapp3_mic_command_queue = { 0, };
Str_data sapp3_status = { 0, };
Str_data sapp3_msg[DEF_SAPP3_NUM_OF_MSG] = { 0, };
Str_data sapp3_camera_saved_path = { 0, };
Str_data sapp3_mic_saved_path = { 0, };
Sapp3_camera_state sapp3_camera_state = CAM_IDLE;
Sapp3_mic_state sapp3_mic_state = MIC_IDLE;


static void Sapp3_draw_init_exit_message(void);
static void Sapp3_init_thread(void* arg);
static void Sapp3_exit_thread(void* arg);
static void Sapp3_camera_thread(void* arg);
static void Sapp3_mic_thread(void* arg);


bool Sapp3_query_init_flag(void)
{
	return sapp3_already_init;
}

bool Sapp3_query_running_flag(void)
{
	return sapp3_main_run;
}

void Sapp3_hid(Hid_info key)
{
	Sem_config config = { 0, };

	//Do nothing if app is suspended.
	if(aptShouldJumpToHome())
		return;

	Sem_get_config(&config);

	if(Util_err_query_error_show_flag())
		Util_err_main(key);
	else if(Util_expl_query_show_flag())
		Util_expl_main(key, config.scroll_speed);
	else
	{
		uint32_t result = DEF_ERR_OTHER;
		Sapp3_camera_command camera_command = CAM_NONE;
		Sapp3_mic_command mic_command = MIC_NONE;

		if(Util_hid_is_pressed(key, *Draw_get_bot_ui_button()))
			Draw_get_bot_ui_button()->selected = true;
		else if (key.p_start || (Util_hid_is_released(key, *Draw_get_bot_ui_button()) && Draw_get_bot_ui_button()->selected))
			Sapp3_suspend();

		if(key.p_a)//Take a picture.
		{
			if(sapp3_camera_state == CAM_ENABLED)
				camera_command = CAM_TAKE_A_PICTURE_REQUEST;
		}
		else if(key.p_b)//Enable or disable the camera.
		{
			if(sapp3_camera_state == CAM_IDLE)
				camera_command = CAM_ENABLE_REQUEST;
			else if(sapp3_camera_state == CAM_ENABLED)
				camera_command = CAM_DISABLE_REQUEST;
		}
		else if(key.p_y)//Start a recording.
		{
			if(sapp3_mic_state == MIC_IDLE)
				mic_command = MIC_START_RECORDING_REQUEST;
		}
		else if(key.p_x)//Stop a recording.
		{
			if(sapp3_mic_state == MIC_RECORDING)
				mic_command = MIC_STOP_RECORDING_REQUEST;
		}

		if(camera_command != CAM_NONE)
		{
			DEF_LOG_RESULT_SMART(result, Util_queue_add(&sapp3_camera_command_queue, camera_command, NULL, 10000, QUEUE_OPTION_DO_NOT_ADD_IF_EXIST),
			(result == DEF_SUCCESS), result);
		}
		if(mic_command != MIC_NONE)
		{
			DEF_LOG_RESULT_SMART(result, Util_queue_add(&sapp3_mic_command_queue, mic_command, NULL, 10000, QUEUE_OPTION_DO_NOT_ADD_IF_EXIST),
			(result == DEF_SUCCESS), result);
		}
	}

	if(!key.p_touch && !key.h_touch)
		Draw_get_bot_ui_button()->selected = false;

	if(Util_log_query_log_show_flag())
		Util_log_main(key);
}

void Sapp3_resume(void)
{
	sapp3_thread_suspend = false;
	sapp3_main_run = true;
	Draw_set_refresh_needed(true);
	Menu_suspend();
}

void Sapp3_suspend(void)
{
	sapp3_thread_suspend = true;
	sapp3_main_run = false;
	Draw_set_refresh_needed(true);
	Menu_resume();
}

uint32_t Sapp3_load_msg(const char* lang)
{
	char file_name[32] = { 0, };

	snprintf(file_name, sizeof(file_name), "sapp3_%s.txt", (lang ? lang : ""));
	return Util_load_msg(file_name, sapp3_msg, DEF_SAPP3_NUM_OF_MSG);
}

void Sapp3_init(bool draw)
{
	DEF_LOG_STRING("Initializing...");
	uint32_t result = DEF_ERR_OTHER;
	Sem_state state = { 0, };

	Sem_get_state(&state);
	DEF_LOG_RESULT_SMART(result, Util_str_init(&sapp3_status), (result == DEF_SUCCESS), result);

	Util_watch_add(WATCH_HANDLE_SUB_APP3, &sapp3_status.sequencial_id, sizeof(sapp3_status.sequencial_id));

	if(DEF_SEM_MODEL_IS_NEW(state.console_model) && Util_is_core_available(2))
		sapp3_init_thread = threadCreate(Sapp3_init_thread, (void*)(""), DEF_THREAD_STACKSIZE, DEF_THREAD_PRIORITY_NORMAL, 2, false);
	else
	{
		APT_SetAppCpuTimeLimit(80);
		sapp3_init_thread = threadCreate(Sapp3_init_thread, (void*)(""), DEF_THREAD_STACKSIZE, DEF_THREAD_PRIORITY_NORMAL, 1, false);
	}

	while(!sapp3_already_init)
	{
		if(draw)
			Sapp3_draw_init_exit_message();
		else
			Util_sleep(20000);
	}

	if(!DEF_SEM_MODEL_IS_NEW(state.console_model) || !Util_is_core_available(2))
		APT_SetAppCpuTimeLimit(10);

	DEF_LOG_RESULT_SMART(result, threadJoin(sapp3_init_thread, DEF_THREAD_WAIT_TIME), (result == DEF_SUCCESS), result);
	threadFree(sapp3_init_thread);

	Util_str_clear(&sapp3_status);
	Sapp3_resume();

	DEF_LOG_STRING("Initialized.");
}

void Sapp3_exit(bool draw)
{
	DEF_LOG_STRING("Exiting...");
	uint32_t result = DEF_ERR_OTHER;

	sapp3_exit_thread = threadCreate(Sapp3_exit_thread, (void*)(""), DEF_THREAD_STACKSIZE, DEF_THREAD_PRIORITY_NORMAL, 1, false);

	while(sapp3_already_init)
	{
		if(draw)
			Sapp3_draw_init_exit_message();
		else
			Util_sleep(20000);
	}

	DEF_LOG_RESULT_SMART(result, threadJoin(sapp3_exit_thread, DEF_THREAD_WAIT_TIME), (result == DEF_SUCCESS), result);
	threadFree(sapp3_exit_thread);

	Util_watch_remove(WATCH_HANDLE_SUB_APP3, &sapp3_status.sequencial_id);
	Util_str_free(&sapp3_status);
	Draw_set_refresh_needed(true);

	DEF_LOG_STRING("Exited.");
}

void Sapp3_main(void)
{
	uint32_t color = DEF_DRAW_BLACK;
	uint32_t back_color = DEF_DRAW_WHITE;
	Watch_handle_bit watch_handle_bit = (DEF_WATCH_HANDLE_BIT_GLOBAL | DEF_WATCH_HANDLE_BIT_SUB_APP3);
	Sem_config config = { 0, };
	Sem_state state = { 0, };

	Sem_get_config(&config);
	Sem_get_state(&state);

	if (config.is_night)
	{
		color = DEF_DRAW_WHITE;
		back_color = DEF_DRAW_BLACK;
	}

	//Check if we should update the screen.
	if(Util_watch_is_changed(watch_handle_bit) || Draw_is_refresh_needed() || !config.is_eco)
	{
		Str_data temp_msg = { 0, };

		Util_str_init(&temp_msg);
		Draw_set_refresh_needed(false);

		Draw_frame_ready();

		if(config.is_top_lcd_on)
		{
			uint8_t draw_cammera_buffer_index = (sapp3_camera_buffer_index == 0 ? 1 : 0);

			Draw_screen_ready(DRAW_SCREEN_TOP_LEFT, back_color);

			Draw(&sapp3_msg[0], 0, 20, 0.5, 0.5, color);

			if(sapp3_camera_state != CAM_IDLE)
			{
				//Draw preview.
				Draw_texture(&sapp3_camera_image[draw_cammera_buffer_index], DEF_DRAW_NO_COLOR, 0, 0, 400, 240);
			}

			Util_str_clear(&temp_msg);
			//Notify user that we are saving a picture.
			if(sapp3_camera_state == CAM_SAVING_A_PICTURE)
				Util_str_add(&temp_msg, "Saving a picture...");

			//Notify user that we are recording.
			if(sapp3_mic_state == MIC_RECORDING || sapp3_mic_state == MIC_STOPPING_RECORDING)
			{
				if(Util_str_has_data(&temp_msg))
					Util_str_add(&temp_msg, "\n");

				Util_str_add(&temp_msg, "Recording sound...");
			}

			if(Util_str_has_data(&temp_msg))
			{
				Draw_image_data background = Draw_get_empty_image();

				Draw_with_background(&temp_msg, 40, 40, 0.5, 0.5, DEF_DRAW_WHITE, DRAW_X_ALIGN_CENTER,
				DRAW_Y_ALIGN_CENTER, 320, 20, DRAW_BACKGROUND_UNDER_TEXT, &background, 0xA0000000);
			}

			if(Util_log_query_log_show_flag())
				Util_log_draw();

			Draw_top_ui(config.is_eco, state.is_charging, state.wifi_signal, state.battery_level, state.msg);

			if(config.is_debug)
				Draw_debug_info(config.is_night, state.free_ram, state.free_linear_ram);

			if(Util_cpu_usage_query_show_flag())
				Util_cpu_usage_draw();

			if(Draw_is_3d_mode())
			{
				Draw_screen_ready(DRAW_SCREEN_TOP_RIGHT, back_color);

				if(Util_log_query_log_show_flag())
					Util_log_draw();

				Draw_top_ui(config.is_eco, state.is_charging, state.wifi_signal, state.battery_level, state.msg);

				if(config.is_debug)
					Draw_debug_info(config.is_night, state.free_ram, state.free_linear_ram);

				if(Util_cpu_usage_query_show_flag())
					Util_cpu_usage_draw();
			}
		}

		if(config.is_bottom_lcd_on)
		{
			Draw_screen_ready(DRAW_SCREEN_BOTTOM, back_color);

			Draw_c(DEF_SAPP3_VER, 0, 0, 0.4, 0.4, DEF_DRAW_GREEN);

			//Draw camera controls.
			if(sapp3_camera_state == CAM_IDLE)
				Draw_c("Press B to enable a camera.", 0, 30, 0.5, 0.5, color);
			else
			{
				Draw_c("Press A to take a picture.", 0, 20, 0.5, 0.5, color);
				Draw_c("Press B to disable a camera.", 0, 30, 0.5, 0.5, color);
			}

			//Draw picture path.
			if(sapp3_camera_state != CAM_SAVING_A_PICTURE && Util_str_has_data(&sapp3_camera_saved_path))
			{
				Draw_c("Picture was saved as :", 0, 40, 0.45, 0.45, DEF_DRAW_BLUE);
				Draw(&sapp3_camera_saved_path, 0, 50, 0.45, 0.45, DEF_DRAW_BLUE);
			}

			//Draw mic controls.
			if(sapp3_mic_state == MIC_IDLE)
				Draw_c("Press Y to start recording sound.", 0, 70, 0.5, 0.5, color);
			else
				Draw_c("Press X to stop recording sound.", 0, 70, 0.5, 0.5, color);

			//Draw sound recording path.
			if(sapp3_mic_state == MIC_IDLE && Util_str_has_data(&sapp3_mic_saved_path))
			{
				Draw_c("Sound recording was saved as : ", 0, 80, 0.45, 0.45, DEF_DRAW_BLUE);
				Draw(&sapp3_mic_saved_path, 0, 90, 0.45, 0.45, DEF_DRAW_BLUE);
			}

			//Draw current camera state.
			Util_str_format(&temp_msg, "State (cam) : %s (%" PRIu32 ")", Sapp3_camera_state_get_name(sapp3_camera_state), (uint32_t)sapp3_camera_state);
			Draw(&temp_msg, 0, 110, 0.5, 0.5, color);

			//Draw current mic state.
			Util_str_format(&temp_msg, "State (mic) : %s (%" PRIu32 ")", Sapp3_mic_state_get_name(sapp3_mic_state), (uint32_t)sapp3_mic_state);
			Draw(&temp_msg, 0, 120, 0.5, 0.5, color);

			if(Util_expl_query_show_flag())
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

static void Sapp3_draw_init_exit_message(void)
{
	uint32_t color = DEF_DRAW_BLACK;
	uint32_t back_color = DEF_DRAW_WHITE;
	Watch_handle_bit watch_handle_bit = (DEF_WATCH_HANDLE_BIT_GLOBAL | DEF_WATCH_HANDLE_BIT_SUB_APP3);
	Sem_config config = { 0, };
	Sem_state state = { 0, };

	Sem_get_config(&config);
	Sem_get_state(&state);

	if (config.is_night)
	{
		color = DEF_DRAW_WHITE;
		back_color = DEF_DRAW_BLACK;
	}

	//Check if we should update the screen.
	if(Util_watch_is_changed(watch_handle_bit) || Draw_is_refresh_needed() || !config.is_eco)
	{
		Draw_set_refresh_needed(false);
		Draw_frame_ready();

		Draw_screen_ready(DRAW_SCREEN_TOP_LEFT, back_color);

		if(Util_log_query_log_show_flag())
			Util_log_draw();

		Draw_top_ui(config.is_eco, state.is_charging, state.wifi_signal, state.battery_level, state.msg);

		if(config.is_debug)
			Draw_debug_info(config.is_night, state.free_ram, state.free_linear_ram);

		if(Util_cpu_usage_query_show_flag())
			Util_cpu_usage_draw();

		Draw(&sapp3_status, 0, 20, 0.65, 0.65, color);

		//Draw the same things on right screen if 3D mode is enabled.
		//So that user can easily see them.
		if(Draw_is_3d_mode())
		{
			Draw_screen_ready(DRAW_SCREEN_TOP_RIGHT, back_color);

			if(Util_log_query_log_show_flag())
				Util_log_draw();

			Draw_top_ui(config.is_eco, state.is_charging, state.wifi_signal, state.battery_level, state.msg);

			if(config.is_debug)
				Draw_debug_info(config.is_night, state.free_ram, state.free_linear_ram);

			if(Util_cpu_usage_query_show_flag())
				Util_cpu_usage_draw();

			Draw(&sapp3_status, 0, 20, 0.65, 0.65, color);
		}

		Draw_apply_draw();
	}
	else
		gspWaitForVBlank();
}

static void Sapp3_init_thread(void* arg)
{
	DEF_LOG_STRING("Thread started.");
	uint32_t result = DEF_ERR_OTHER;
	Sem_state state = { 0, };

	Sem_get_state(&state);

	Util_str_set(&sapp3_status, "Initializing variables...");
	sapp3_camera_buffer_index = 0;
	sapp3_camera_state = CAM_IDLE;
	sapp3_mic_state = MIC_IDLE;

	DEF_LOG_RESULT_SMART(result, Util_str_init(&sapp3_camera_saved_path), (result == DEF_SUCCESS), result);
	DEF_LOG_RESULT_SMART(result, Util_str_init(&sapp3_mic_saved_path), (result == DEF_SUCCESS), result);

	//Add to watch to detect value changes, screen will be rerenderd when value is changed.
	Util_watch_add(WATCH_HANDLE_SUB_APP3, &sapp3_camera_state, sizeof(sapp3_camera_state));
	Util_watch_add(WATCH_HANDLE_SUB_APP3, &sapp3_mic_state, sizeof(sapp3_mic_state));
	Util_watch_add(WATCH_HANDLE_SUB_APP3, &sapp3_camera_saved_path.sequencial_id, sizeof(sapp3_camera_saved_path.sequencial_id));
	Util_watch_add(WATCH_HANDLE_SUB_APP3, &sapp3_mic_saved_path.sequencial_id, sizeof(sapp3_mic_saved_path.sequencial_id));

	Util_str_add(&sapp3_status, "\nInitializing queue...");
	//Create the queues for commands.
	DEF_LOG_RESULT_SMART(result, Util_queue_create(&sapp3_camera_command_queue, 10), (result == DEF_SUCCESS), result);
	DEF_LOG_RESULT_SMART(result, Util_queue_create(&sapp3_mic_command_queue, 10), (result == DEF_SUCCESS), result);

	Util_str_add(&sapp3_status, "\nInitializing mic...");
	//Init mic with 500KB buffer.
	DEF_LOG_RESULT_SMART(result, Util_mic_init(1000 * 500), (result == DEF_SUCCESS), result);

	Util_str_add(&sapp3_status, "\nInitializing camera...");
	//1. Init camera.
	DEF_LOG_RESULT_SMART(result, Util_cam_init(RAW_PIXEL_RGB565LE), (result == DEF_SUCCESS), result);

	//2. Set resolution.
	DEF_LOG_RESULT_SMART(result, Util_cam_set_resolution(CAM_RES_400x240), (result == DEF_SUCCESS), result);

	//3. Set framerate. Use 30fps for new 3ds, 20fps for old 3ds.
	if(DEF_SEM_MODEL_IS_NEW(state.console_model))
		DEF_LOG_RESULT_SMART(result, Util_cam_set_fps(CAM_FPS_30), (result == DEF_SUCCESS), result)
	else
		DEF_LOG_RESULT_SMART(result, Util_cam_set_fps(CAM_FPS_20), (result == DEF_SUCCESS), result);

	//4. Optionally, you can set these parameters.
	// Util_cam_set_camera(CAM_PORT_OUT_RIGHT);
	// Util_cam_set_contrast(CAM_CONTRAST_06);
	// Util_cam_set_exposure(CAM_EXPOSURE_3);
	// Util_cam_set_lens_correction(CAM_LENS_CORRECTION_70);
	// Util_cam_set_noise_filter(true);
	// Util_cam_set_white_balance(CAM_WHITE_BALANCE_AUTO);

	//5. Init 512x256 tectures (double buffering to prevent glitch).
	for(uint8_t i = 0; i < 2; i++)
		DEF_LOG_RESULT_SMART(result, Draw_texture_init(&sapp3_camera_image[i], 512, 256, RAW_PIXEL_RGB565LE);, (result == DEF_SUCCESS), result);

	Util_str_add(&sapp3_status, "\nStarting threads...");
	sapp3_thread_run = true;
	if(DEF_SEM_MODEL_IS_NEW(state.console_model) && Util_is_core_available(2))
		sapp3_camera_thread = threadCreate(Sapp3_camera_thread, (void*)(""), DEF_THREAD_STACKSIZE, DEF_THREAD_PRIORITY_NORMAL, 2, false);
	else
		sapp3_camera_thread = threadCreate(Sapp3_camera_thread, (void*)(""), DEF_THREAD_STACKSIZE, DEF_THREAD_PRIORITY_NORMAL, 0, false);

	sapp3_mic_thread = threadCreate(Sapp3_mic_thread, (void*)(""), DEF_THREAD_STACKSIZE, DEF_THREAD_PRIORITY_LOW, 0, false);

	sapp3_already_init = true;

	DEF_LOG_STRING("Thread exit.");
	threadExit(0);
}

static void Sapp3_exit_thread(void* arg)
{
	DEF_LOG_STRING("Thread started.");
	uint32_t result = DEF_ERR_OTHER;

	sapp3_thread_suspend = false;
	sapp3_thread_run = false;

	Util_str_set(&sapp3_status, "Exiting threads...");
	DEF_LOG_RESULT_SMART(result, threadJoin(sapp3_camera_thread, DEF_THREAD_WAIT_TIME), (result == DEF_SUCCESS), result);
	DEF_LOG_RESULT_SMART(result, threadJoin(sapp3_mic_thread, DEF_THREAD_WAIT_TIME), (result == DEF_SUCCESS), result);

	Util_str_add(&sapp3_status, "\nCleaning up...");
	threadFree(sapp3_camera_thread);
	threadFree(sapp3_mic_thread);

	//Exit mic.
	Util_mic_exit();

	//Exit camera.
	Util_cam_exit();

	//Free textures.
	for(uint8_t i = 0; i < 2; i++)
		Draw_texture_free(&sapp3_camera_image[i]);

	//Delete the queues.
	Util_queue_delete(&sapp3_camera_command_queue);
	Util_queue_delete(&sapp3_mic_command_queue);

	//Remove watch on exit.
	Util_watch_remove(WATCH_HANDLE_SUB_APP3, &sapp3_camera_state);
	Util_watch_remove(WATCH_HANDLE_SUB_APP3, &sapp3_mic_state);
	Util_watch_remove(WATCH_HANDLE_SUB_APP3, &sapp3_camera_saved_path);
	Util_watch_remove(WATCH_HANDLE_SUB_APP3, &sapp3_mic_saved_path);

	//Free string buffers.
	Util_str_free(&sapp3_camera_saved_path);
	Util_str_free(&sapp3_mic_saved_path);

	sapp3_already_init = false;

	DEF_LOG_STRING("Thread exit.");
	threadExit(0);
}

static void Sapp3_camera_thread(void* arg)
{
	DEF_LOG_STRING("Thread started.");
	uint8_t dummy = 0;

	//Create directory.
	Util_file_save_to_file(".", DEF_MENU_MAIN_DIR "images/", &dummy, 1, true);

	while (sapp3_thread_run)
	{
		uint32_t event_id = 0;
		uint32_t result = DEF_ERR_OTHER;

		while (sapp3_thread_suspend)
			Util_sleep(DEF_THREAD_INACTIVE_SLEEP_TIME);

		result = Util_queue_get(&sapp3_camera_command_queue, &event_id, NULL, 0);
		if(result == DEF_SUCCESS)
		{
			//Got a command.
			DEF_LOG_FORMAT("Received event : %s (%" PRIu32 ")", Sapp3_camera_command_get_name((Sapp3_camera_command)event_id), event_id);

			switch ((Sapp3_camera_command)event_id)
			{
				case CAM_ENABLE_REQUEST:
				{
					sapp3_camera_state = CAM_ENABLED;
					break;
				}
				case CAM_DISABLE_REQUEST:
				{
					sapp3_camera_state = CAM_IDLE;
					break;
				}
				case CAM_TAKE_A_PICTURE_REQUEST:
				{
					sapp3_camera_state = CAM_SAVING_A_PICTURE;
					break;
				}

				default:
				{
					break;
				}
			}
		}

		if(sapp3_camera_state == CAM_ENABLED || sapp3_camera_state == CAM_SAVING_A_PICTURE)
		{
			bool shutter_sound = (sapp3_camera_state == CAM_SAVING_A_PICTURE);
			uint8_t* picture = NULL;
			uint16_t width = 0;
			uint16_t height = 0;

			//1. Take a picture with shutter sound when saving the picture otherwise without shutter sound.
			result = Util_cam_take_a_picture(&picture, &width, &height, shutter_sound);
			if(result == DEF_SUCCESS)
			{
				//2. Update texture.
				result = Draw_set_texture_data(&sapp3_camera_image[sapp3_camera_buffer_index], picture, width, height, 0, 0);

				//3. Update buffer index.
				sapp3_camera_buffer_index = (sapp3_camera_buffer_index == 0 ? 1 : 0);

				if(result == DEF_SUCCESS)
				{
					//4. Refresh screen.
					Draw_set_refresh_needed(true);

					if(sapp3_camera_state == CAM_SAVING_A_PICTURE)
					{
						Converter_color_parameters parameters = { 0, };

						/*
						Note : Preview on the screen won't be updated while saving the picture
						because encoding functions will block this thread so that it can't update preview.
						To avoid it, you can create a different thread with lower priority for encoding.
						This sample code supposes to provide as simple code as possible so don't do that here.
						*/

						//5. Convert color format.
						parameters.source = picture;
						parameters.converted = NULL;
						parameters.in_width = width;
						parameters.in_height = height;
						parameters.out_width = width;
						parameters.out_height = height;
						parameters.in_color_format = RAW_PIXEL_RGB565LE;
						parameters.out_color_format = RAW_PIXEL_RGB888;

						DEF_LOG_RESULT_SMART(result, Util_converter_convert_color(&parameters), (result == DEF_SUCCESS), result);

						if(result == DEF_SUCCESS)
						{
							char path[96] = { 0, };
							Sem_state state = { 0, };

							Sem_get_state(&state);

							snprintf(path, sizeof(path), "%simages/%04" PRIu16 "_%02" PRIu8 "_%02" PRIu8 "_%02" PRIu8 "_%02" PRIu8 "_%02" PRIu8 ".png",
							DEF_MENU_MAIN_DIR, state.time.years, state.time.months, state.time.days, state.time.hours, state.time.minutes, state.time.seconds);

							//6. Save the picture as png.
							DEF_LOG_RESULT_SMART(result, Util_encoder_image_encode(path, parameters.converted, width, height, MEDIA_I_CODEC_PNG, 0), (result == DEF_SUCCESS), result);

							if(result == DEF_SUCCESS)
								Util_str_set(&sapp3_camera_saved_path, path);
							else
								Util_str_set(&sapp3_camera_saved_path, "");
						}

						free(parameters.converted);
						parameters.converted = NULL;
					}
				}
				else
					DEF_LOG_RESULT(Draw_set_texture_data, false, result);
			}
			else
			{
				DEF_LOG_RESULT(Util_cam_take_a_picture, false, result);
				Util_sleep(DEF_THREAD_ACTIVE_SLEEP_TIME);
			}

			free(picture);
			picture = NULL;
			sapp3_camera_state = CAM_ENABLED;
		}
		else//Nothing to do, just sleep.
			Util_sleep(DEF_THREAD_ACTIVE_SLEEP_TIME);
	}

	DEF_LOG_STRING("Thread exit.");
	threadExit(0);
}

static void Sapp3_mic_thread(void* arg)
{
	DEF_LOG_STRING("Thread started.");
	uint8_t dummy = 0;

	//Create directory.
	Util_file_save_to_file(".", DEF_MENU_MAIN_DIR "sound/", &dummy, 1, true);

	while (sapp3_thread_run)
	{
		uint32_t event_id = 0;
		uint32_t result = DEF_ERR_OTHER;

		while (sapp3_thread_suspend)
			Util_sleep(DEF_THREAD_INACTIVE_SLEEP_TIME);

		result = Util_queue_get(&sapp3_mic_command_queue, &event_id, NULL, 0);
		if(result == DEF_SUCCESS)
		{
			//Got a command.
			DEF_LOG_FORMAT("Received event : %s (%" PRIu32 ")", Sapp3_mic_command_get_name((Sapp3_mic_command)event_id), event_id);

			switch ((Sapp3_mic_command)event_id)
			{
				case MIC_START_RECORDING_REQUEST:
				{
					char path[96] = { 0, };
					Sem_state state = { 0, };

					Sem_get_state(&state);

					snprintf(path, sizeof(path), "%ssound/%04" PRIu16 "_%02" PRIu8 "_%02" PRIu8 "_%02" PRIu8 "_%02" PRIu8 "_%02" PRIu8 ".mp3",
					DEF_MENU_MAIN_DIR, state.time.years, state.time.months, state.time.days, state.time.hours, state.time.minutes, state.time.seconds);

					//1. Create an output file.
					DEF_LOG_RESULT_SMART(result, Util_encoder_create_output_file(path, 0), (result == DEF_SUCCESS), result);

					//2. Init encoder.
					if(DEF_SEM_MODEL_IS_NEW(state.console_model))
					{
						//For new 3ds, codec : mp3, sample rate : 32KHz, bit rate : 128kbps.
						DEF_LOG_RESULT_SMART(result, Util_encoder_audio_init(MEDIA_A_CODEC_MP3, 32728, 32000, 128000, 0), (result == DEF_SUCCESS), result);
					}
					else
					{
						//For old 3ds, codec : mp3, sample rate : 16KHz, bit rate : 96kbps.
						DEF_LOG_RESULT_SMART(result, Util_encoder_audio_init(MEDIA_A_CODEC_MP3, 16384, 16000, 96000, 0), (result == DEF_SUCCESS), result);
					}

					//3. Write a header, if needed.
					DEF_LOG_RESULT_SMART(result, Util_encoder_write_header(0), (result == DEF_SUCCESS), result);
					if(result == DEF_SUCCESS)
					{
						//4. Start a recording, for new 3ds use 32728Hz, for old 3ds use 16364Hz.
						if(DEF_SEM_MODEL_IS_NEW(state.console_model))
							DEF_LOG_RESULT_SMART(result, Util_mic_start_recording(MIC_SAMPLE_RATE_32728HZ), (result == DEF_SUCCESS), result)
						else
							DEF_LOG_RESULT_SMART(result, Util_mic_start_recording(MIC_SAMPLE_RATE_16364HZ), (result == DEF_SUCCESS), result);

						if(result == DEF_SUCCESS)
						{
							sapp3_mic_state = MIC_RECORDING;
							Util_str_set(&sapp3_mic_saved_path, path);
						}
						else//Error.
						{
							sapp3_mic_state = MIC_STOPPING_RECORDING;
							Util_str_set(&sapp3_mic_saved_path, "");
						}
					}
					else
						Util_encoder_close_output_file(0);

					break;
				}
				case MIC_STOP_RECORDING_REQUEST:
				{
					Util_mic_stop_recording();
					if(sapp3_mic_state == MIC_RECORDING)
						sapp3_mic_state = MIC_STOPPING_RECORDING;
					else
						sapp3_mic_state = MIC_IDLE;

					break;
				}

				default:
				{
					break;
				}
			}
		}

		if(sapp3_mic_state == MIC_RECORDING || sapp3_mic_state == MIC_STOPPING_RECORDING)
		{
			Util_sleep(5000);

			//If remaining buffer is less than 500ms or stop request has been received, encode pcm data to mp3.
			if(Util_mic_query_remaining_buffer_time() < 500 || sapp3_mic_state == MIC_STOPPING_RECORDING)
			{
				uint8_t* audio = NULL;
				uint32_t size = 0;

				DEF_LOG_RESULT_SMART(result, Util_mic_get_audio_data(&audio, &size), (result == DEF_SUCCESS), result);
				if(result == DEF_SUCCESS)
					DEF_LOG_RESULT_SMART(result, Util_encoder_audio_encode(size, audio, 0), (result == DEF_SUCCESS), result);

				free(audio);
				audio = NULL;
			}

			//Upon receiving stop recording request, close the output file and set mic state to idle.
			if(sapp3_mic_state == MIC_STOPPING_RECORDING)
			{
				Util_encoder_close_output_file(0);
				sapp3_mic_state = MIC_IDLE;
			}
		}
		else//Nothing to do, just sleep.
			Util_sleep(DEF_THREAD_ACTIVE_SLEEP_TIME);
	}

	Util_encoder_close_output_file(0);

	DEF_LOG_STRING("Thread exit.");
	threadExit(0);
}
