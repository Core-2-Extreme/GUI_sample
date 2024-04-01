#include "definitions.hpp"
#include "system/types.hpp"

#include "system/menu.hpp"
#include "system/variables.hpp"

#include "system/draw/draw.hpp"

#include "system/util/camera.hpp"
#include "system/util/converter.hpp"
#include "system/util/encoder.hpp"
#include "system/util/error.hpp"
#include "system/util/file.hpp"
#include "system/util/hid.hpp"
#include "system/util/log.hpp"
#include "system/util/mic.hpp"
#include "system/util/util.hpp"

extern "C"
{
	#include "system/util/queue.h"
	#include "system/util/str.h"
}

//Include myself.
#include "sub_app3.hpp"


enum Sapp3_camera_command
{
	CAM_NONE,

	CAM_DISABLE_REQUEST,
	CAM_ENABLE_REQUEST,
	CAM_TAKE_A_PICTURE_REQUEST,

	CAM_MAX = 0xFF,
};

enum Sapp3_camera_state
{
	CAM_IDLE,
	CAM_ENABLED,
	CAM_SAVING_A_PICTURE,
};

enum Sapp3_mic_command
{
	MIC_NONE,

	MIC_START_RECORDING_REQUEST,
	MIC_STOP_RECORDING_REQUEST,

	MIC_MAX = 0xFF,
};

enum Sapp3_mic_state
{
	MIC_IDLE,
	MIC_RECORDING,
	MIC_STOPPING_RECORDING,
};


bool sapp3_main_run = false;
bool sapp3_thread_run = false;
bool sapp3_already_init = false;
bool sapp3_thread_suspend = true;
int sapp3_camera_buffer_index = 0;
std::string sapp3_msg[DEF_SAPP3_NUM_OF_MSG];
Thread sapp3_init_thread, sapp3_exit_thread, sapp3_camera_thread, sapp3_mic_thread;
Image_data sapp3_camera_image[2];
Util_queue sapp3_camera_command_queue, sapp3_mic_command_queue;
Util_str sapp3_status = { 0, };
Util_str sapp3_camera_saved_file_path = { 0, };
Util_str sapp3_mic_saved_file = { 0, };
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
	//Do nothing if app is suspended.
	if(aptShouldJumpToHome())
		return;

	if(Util_err_query_error_show_flag())
		Util_err_main(key);
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
	var_need_reflesh = true;
	Menu_suspend();
}

void Sapp3_suspend(void)
{
	sapp3_thread_suspend = true;
	sapp3_main_run = false;
	var_need_reflesh = true;
	Menu_resume();
}

Result_with_string Sapp3_load_msg(std::string lang)
{
	return Util_load_msg("sapp3_" + lang + ".txt", sapp3_msg, DEF_SAPP3_NUM_OF_MSG);
}

void Sapp3_init(bool draw)
{
	DEF_LOG_STRING("Initializing...");
	uint32_t result = DEF_ERR_OTHER;

	DEF_LOG_RESULT_SMART(result, Util_str_init(&sapp3_status), (result == DEF_SUCCESS), result);

	Util_add_watch(WATCH_HANDLE_SUB_APP3, &sapp3_status.sequencial_id, sizeof(sapp3_status.sequencial_id));

	if((var_model == CFG_MODEL_N2DSXL || var_model == CFG_MODEL_N3DSXL || var_model == CFG_MODEL_N3DS) && var_core_2_available)
		sapp3_init_thread = threadCreate(Sapp3_init_thread, (void*)(""), DEF_STACKSIZE, DEF_THREAD_PRIORITY_NORMAL, 2, false);
	else
	{
		APT_SetAppCpuTimeLimit(80);
		sapp3_init_thread = threadCreate(Sapp3_init_thread, (void*)(""), DEF_STACKSIZE, DEF_THREAD_PRIORITY_NORMAL, 1, false);
	}

	while(!sapp3_already_init)
	{
		if(draw)
			Sapp3_draw_init_exit_message();
		else
			Util_sleep(20000);
	}

	if(!(var_model == CFG_MODEL_N2DSXL || var_model == CFG_MODEL_N3DSXL || var_model == CFG_MODEL_N3DS) || !var_core_2_available)
		APT_SetAppCpuTimeLimit(10);

	DEF_LOG_RESULT_SMART(result, threadJoin(sapp3_init_thread, DEF_THREAD_WAIT_TIME), result, result);
	threadFree(sapp3_init_thread);

	Util_str_clear(&sapp3_status);
	Sapp3_resume();

	DEF_LOG_STRING("Initialized.");
}

void Sapp3_exit(bool draw)
{
	DEF_LOG_STRING("Exiting...");
	uint32_t result = DEF_ERR_OTHER;

	sapp3_exit_thread = threadCreate(Sapp3_exit_thread, (void*)(""), DEF_STACKSIZE, DEF_THREAD_PRIORITY_NORMAL, 1, false);

	while(sapp3_already_init)
	{
		if(draw)
			Sapp3_draw_init_exit_message();
		else
			Util_sleep(20000);
	}

	DEF_LOG_RESULT_SMART(result, threadJoin(sapp3_exit_thread, DEF_THREAD_WAIT_TIME), result, result);
	threadFree(sapp3_exit_thread);

	Util_remove_watch(WATCH_HANDLE_SUB_APP3, &sapp3_status.sequencial_id);
	Util_str_free(&sapp3_status);
	var_need_reflesh = true;

	DEF_LOG_STRING("Exited.");
}

void Sapp3_main(void)
{
	int color = DEF_DRAW_BLACK;
	int back_color = DEF_DRAW_WHITE;
	Watch_handle_bit watch_handle_bit = (DEF_WATCH_HANDLE_BIT_GLOBAL | DEF_WATCH_HANDLE_BIT_SUB_APP3);

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
			int draw_cammera_buffer_index = (sapp3_camera_buffer_index == 0 ? 1 : 0);
			std::string status = "";

			Draw_screen_ready(SCREEN_TOP_LEFT, back_color);

			Draw(sapp3_msg[0], 0, 20, 0.5, 0.5, color);

			if(sapp3_camera_state != CAM_IDLE)
			{
				//Draw preview.
				Draw_texture(&sapp3_camera_image[draw_cammera_buffer_index], 0, 0, 400, 240);
			}

			//Notify user that we are saving a picture.
			if(sapp3_camera_state == CAM_SAVING_A_PICTURE)
				status = "Saving a picture...";

			//Notify user that we are recording.
			if(sapp3_mic_state == MIC_RECORDING || sapp3_mic_state == MIC_STOPPING_RECORDING)
			{
				if(status != "")
					status += "\n";

				status += "Recording sound...";
			}

			if(status != "")
			{
				Draw(status, 40, 40, 0.5, 0.5, DEF_DRAW_WHITE, X_ALIGN_CENTER,
				Y_ALIGN_CENTER, 320, 20, BACKGROUND_UNDER_TEXT, var_square_image[0], 0xA0000000);
			}

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

			Draw(DEF_SAPP3_VER, 0, 0, 0.4, 0.4, DEF_DRAW_GREEN);

			//Draw camera controls.
			if(sapp3_camera_state == CAM_IDLE)
				Draw("Press B to enable a camera.", 0, 30, 0.5, 0.5, color);
			else
			{
				Draw("Press A to take a picture.", 0, 20, 0.5, 0.5, color);
				Draw("Press B to disable a camera.", 0, 30, 0.5, 0.5, color);
			}

			//Draw picture path.
			if(Util_str_has_data(&sapp3_camera_saved_file_path) && sapp3_camera_state != CAM_SAVING_A_PICTURE)
			{
				Draw("Picture was saved as :", 0, 40, 0.45, 0.45, DEF_DRAW_BLUE);
				Draw(sapp3_camera_saved_file_path.buffer, 0, 50, 0.45, 0.45, DEF_DRAW_BLUE);
			}

			//Draw mic controls.
			if(sapp3_mic_state == MIC_IDLE)
				Draw("Press Y to start recording sound.", 0, 70, 0.5, 0.5, color);
			else
				Draw("Press X to stop recording sound.", 0, 70, 0.5, 0.5, color);

			//Draw sound recording path.
			if(Util_str_has_data(&sapp3_mic_saved_file) && sapp3_mic_state == MIC_IDLE)
			{
				Draw("Sound recording was saved as", 0, 80, 0.45, 0.45, DEF_DRAW_BLUE);
				Draw(sapp3_mic_saved_file.buffer, 0, 90, 0.45, 0.45, DEF_DRAW_BLUE);
			}

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
	int color = DEF_DRAW_BLACK;
	int back_color = DEF_DRAW_WHITE;
	Watch_handle_bit watch_handle_bit = (DEF_WATCH_HANDLE_BIT_GLOBAL | DEF_WATCH_HANDLE_BIT_SUB_APP3);

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

		Draw(sapp3_status.buffer, 0, 20, 0.65, 0.65, color);

		Draw_apply_draw();
	}
	else
		gspWaitForVBlank();
}

static void Sapp3_init_thread(void* arg)
{
	DEF_LOG_STRING("Thread started.");
	Result_with_string result;

	Util_str_set(&sapp3_status, "Initializing variables...");
	sapp3_camera_buffer_index = 0;
	sapp3_camera_state = CAM_IDLE;
	sapp3_mic_state = MIC_IDLE;

	DEF_LOG_RESULT_SMART(result.code, Util_str_init(&sapp3_camera_saved_file_path), (result.code == DEF_SUCCESS), result.code);
	DEF_LOG_RESULT_SMART(result.code, Util_str_init(&sapp3_mic_saved_file), (result.code == DEF_SUCCESS), result.code);

	//Add to watch to detect value changes, screen will be rerenderd when value is changed.
	Util_add_watch(WATCH_HANDLE_SUB_APP3, &sapp3_camera_state, sizeof(sapp3_camera_state));
	Util_add_watch(WATCH_HANDLE_SUB_APP3, &sapp3_mic_state, sizeof(sapp3_mic_state));

	Util_str_add(&sapp3_status, "\nInitializing queue...");
	//Create the queues for commands.
	DEF_LOG_RESULT_SMART(result.code, Util_queue_create(&sapp3_camera_command_queue, 10), (result.code == DEF_SUCCESS), result.code);
	DEF_LOG_RESULT_SMART(result.code, Util_queue_create(&sapp3_mic_command_queue, 10), (result.code == DEF_SUCCESS), result.code);

	Util_str_add(&sapp3_status, "\nInitializing mic...");
	//Init mic with 500KB buffer.
	DEF_LOG_RESULT_SMART(result, Util_mic_init(1000 * 500), (result.code == DEF_SUCCESS), result.code);

	Util_str_add(&sapp3_status, "\nInitializing camera...");
	//1. Init camera.
	DEF_LOG_RESULT_SMART(result, Util_cam_init(PIXEL_FORMAT_RGB565LE), (result.code == DEF_SUCCESS), result.code);

	//2. Set resolution.
	DEF_LOG_RESULT_SMART(result, Util_cam_set_resolution(CAM_RES_400x240), (result.code == DEF_SUCCESS), result.code);

	//3. Set framerate. Use 30fps for new 3ds, 20fps for old 3ds.
	if(var_model == CFG_MODEL_N2DSXL || var_model == CFG_MODEL_N3DSXL || var_model == CFG_MODEL_N3DS)
		DEF_LOG_RESULT_SMART(result, Util_cam_set_fps(CAM_FPS_30), (result.code == DEF_SUCCESS), result.code)
	else
		DEF_LOG_RESULT_SMART(result, Util_cam_set_fps(CAM_FPS_20), (result.code == DEF_SUCCESS), result.code);

	//4. Optionally, you can set these parameters.
	// Util_cam_set_camera(CAM_PORT_OUT_RIGHT);
	// Util_cam_set_contrast(CAM_CONTRAST_06);
	// Util_cam_set_exposure(CAM_EXPOSURE_3);
	// Util_cam_set_lens_correction(CAM_LENS_CORRECTION_70);
	// Util_cam_set_noise_filter(true);
	// Util_cam_set_white_balance(CAM_WHITE_BALANCE_AUTO);

	//5. Init 512x256 tectures (double buffering to prevent glitch).
	for(int i = 0; i < 2; i++)
		DEF_LOG_RESULT_SMART(result, Draw_texture_init(&sapp3_camera_image[i], 512, 256, PIXEL_FORMAT_RGB565LE);, (result.code == DEF_SUCCESS), result.code);

	Util_str_add(&sapp3_status, "\nStarting threads...");
	sapp3_thread_run = true;
	if((var_model == CFG_MODEL_N2DSXL || var_model == CFG_MODEL_N3DSXL || var_model == CFG_MODEL_N3DS) && var_core_2_available)
		sapp3_camera_thread = threadCreate(Sapp3_camera_thread, (void*)(""), DEF_STACKSIZE, DEF_THREAD_PRIORITY_NORMAL, 2, false);
	else
		sapp3_camera_thread = threadCreate(Sapp3_camera_thread, (void*)(""), DEF_STACKSIZE, DEF_THREAD_PRIORITY_NORMAL, 0, false);

	sapp3_mic_thread = threadCreate(Sapp3_mic_thread, (void*)(""), DEF_STACKSIZE, DEF_THREAD_PRIORITY_LOW, 0, false);

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
	DEF_LOG_RESULT_SMART(result, threadJoin(sapp3_camera_thread, DEF_THREAD_WAIT_TIME), result, result);
	DEF_LOG_RESULT_SMART(result, threadJoin(sapp3_mic_thread, DEF_THREAD_WAIT_TIME), result, result);

	Util_str_add(&sapp3_status, "\nCleaning up...");
	threadFree(sapp3_camera_thread);
	threadFree(sapp3_mic_thread);

	//Exit mic.
	Util_mic_exit();

	//Exit camera.
	Util_cam_exit();

	//Free textures.
	for(int i = 0; i < 2; i++)
		Draw_texture_free(&sapp3_camera_image[i]);

	//Delete the queues.
	Util_queue_delete(&sapp3_camera_command_queue);
	Util_queue_delete(&sapp3_mic_command_queue);

	//Remove watch on exit.
	Util_remove_watch(WATCH_HANDLE_SUB_APP3, &sapp3_camera_state);
	Util_remove_watch(WATCH_HANDLE_SUB_APP3, &sapp3_mic_state);

	//Free string buffers.
	Util_str_free(&sapp3_camera_saved_file_path);
	Util_str_free(&sapp3_mic_saved_file);

	sapp3_already_init = false;

	DEF_LOG_STRING("Thread exit.");
	threadExit(0);
}

static void Sapp3_camera_thread(void* arg)
{
	DEF_LOG_STRING("Thread started.");
	uint8_t dummy = 0;
	Result_with_string result;

	//Create directory.
	Util_file_save_to_file(".", DEF_MAIN_DIR + "images/", &dummy, 1, true);

	while (sapp3_thread_run)
	{
		uint32_t event_id = 0;

		while (sapp3_thread_suspend)
			Util_sleep(DEF_INACTIVE_THREAD_SLEEP_TIME);

		result.code = Util_queue_get(&sapp3_camera_command_queue, &event_id, NULL, 0);
		if(result.code == 0)
		{
			//Got a command.
			DEF_LOG_FORMAT("Received event : %" PRIu32, event_id);

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
			int width = 0;
			int height = 0;

			//1. Take a picture with shutter sound when saving the picture otherwise without shutter sound.
			result = Util_cam_take_a_picture(&picture, &width, &height, shutter_sound);
			if(result.code == 0)
			{
				//2. Update texture.
				result = Draw_set_texture_data(&sapp3_camera_image[sapp3_camera_buffer_index], picture, width, height);

				//3. Update buffer index.
				sapp3_camera_buffer_index = (sapp3_camera_buffer_index == 0 ? 1 : 0);

				if(result.code == 0)
				{
					//4. Refresh screen.
					var_need_reflesh = true;

					if(sapp3_camera_state == CAM_SAVING_A_PICTURE)
					{
						Color_converter_parameters parameters;

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
						parameters.in_color_format = PIXEL_FORMAT_RGB565LE;
						parameters.out_color_format = PIXEL_FORMAT_RGB888;

						DEF_LOG_RESULT_SMART(result, Util_converter_convert_color(&parameters), (result.code == DEF_SUCCESS), result.code);

						if(result.code == 0)
						{
							char path[96];
							snprintf(path, sizeof(path), "%simages/%04d_%02d_%02d_%02d_%02d_%02d.png", (DEF_MAIN_DIR).c_str(),
							var_years, var_months, var_days, var_hours, var_minutes, var_seconds);

							//6. Save the picture as png.
							DEF_LOG_RESULT_SMART(result, Util_image_encoder_encode(path, parameters.converted, parameters.out_width, parameters.out_height, IMAGE_CODEC_PNG, 0),
							(result.code == DEF_SUCCESS), result.code);

							if(result.code == 0)
								Util_str_set(&sapp3_camera_saved_file_path, path);
							else
								Util_str_set(&sapp3_camera_saved_file_path, "");
						}

						free(parameters.converted);
						parameters.converted = NULL;
					}
				}
				else
					DEF_LOG_RESULT(Draw_set_texture_data, false, result.code);
			}
			else
			{
				DEF_LOG_RESULT(Util_cam_take_a_picture, false, result.code);
				Util_sleep(DEF_ACTIVE_THREAD_SLEEP_TIME);
			}

			free(picture);
			picture = NULL;
			sapp3_camera_state = CAM_ENABLED;
		}
		else//Nothing to do, just sleep.
			Util_sleep(DEF_ACTIVE_THREAD_SLEEP_TIME);
	}

	DEF_LOG_STRING("Thread exit.");
	threadExit(0);
}

static void Sapp3_mic_thread(void* arg)
{
	DEF_LOG_STRING("Thread started.");
	uint8_t dummy = 0;
	Result_with_string result;

	//Create directory.
	Util_file_save_to_file(".", DEF_MAIN_DIR + "sound/", &dummy, 1, true);

	while (sapp3_thread_run)
	{
		uint32_t event_id = 0;

		while (sapp3_thread_suspend)
			Util_sleep(DEF_INACTIVE_THREAD_SLEEP_TIME);

		result.code = Util_queue_get(&sapp3_mic_command_queue, &event_id, NULL, 0);
		if(result.code == 0)
		{
			//Got a command.
			DEF_LOG_FORMAT("Received event : %" PRIu32, event_id);

			switch ((Sapp3_mic_command)event_id)
			{
				case MIC_START_RECORDING_REQUEST:
				{
					char path[96];
					snprintf(path, sizeof(path), "%ssound/%04d_%02d_%02d_%02d_%02d_%02d.mp3", (DEF_MAIN_DIR).c_str(),
					var_years, var_months, var_days, var_hours, var_minutes, var_seconds);

					//1. Create an output file.
					DEF_LOG_RESULT_SMART(result, Util_encoder_create_output_file(path, 0), (result.code == DEF_SUCCESS), result.code);

					//2. Init encoder.
					if(var_model == CFG_MODEL_N2DSXL || var_model == CFG_MODEL_N3DSXL || var_model == CFG_MODEL_N3DS)
					{
						//For new 3ds, codec : mp3, sample rate : 32KHz, bit rate : 128kbps.
						DEF_LOG_RESULT_SMART(result, Util_audio_encoder_init(AUDIO_CODEC_MP3, 32728, 32000, 128000, 0),
						(result.code == DEF_SUCCESS), result.code);
					}
					else
					{
						//For old 3ds, codec : mp3, sample rate : 16KHz, bit rate : 96kbps.
						DEF_LOG_RESULT_SMART(result, Util_audio_encoder_init(AUDIO_CODEC_MP3, 16384, 16000, 96000, 0),
						(result.code == DEF_SUCCESS), result.code);
					}

					//3. Write a header, if needed.
					DEF_LOG_RESULT_SMART(result, Util_encoder_write_header(0), (result.code == DEF_SUCCESS), result.code);
					if(result.code == 0)
					{
						//4. Start a recording, for new 3ds use 32728Hz, for old 3ds use 16364Hz.
						if(var_model == CFG_MODEL_N2DSXL || var_model == CFG_MODEL_N3DSXL || var_model == CFG_MODEL_N3DS)
							DEF_LOG_RESULT_SMART(result, Util_mic_start_recording(MIC_SAMPLE_RATE_32728HZ), (result.code == DEF_SUCCESS), result.code)
						else
							DEF_LOG_RESULT_SMART(result, Util_mic_start_recording(MIC_SAMPLE_RATE_16364HZ), (result.code == DEF_SUCCESS), result.code);

						if(result.code == 0)
						{
							sapp3_mic_state = MIC_RECORDING;
							Util_str_set(&sapp3_mic_saved_file, path);
						}
						else//Error.
						{
							sapp3_mic_state = MIC_STOPPING_RECORDING;
							Util_str_set(&sapp3_mic_saved_file, "");
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
				int size = 0;

				DEF_LOG_RESULT_SMART(result, Util_mic_get_audio_data(&audio, &size), (result.code == DEF_SUCCESS), result.code);
				if(result.code == 0)
					DEF_LOG_RESULT_SMART(result, Util_audio_encoder_encode(size, audio, 0), (result.code == DEF_SUCCESS), result.code);

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
			Util_sleep(DEF_ACTIVE_THREAD_SLEEP_TIME);
	}

	Util_encoder_close_output_file(0);

	DEF_LOG_STRING("Thread exit.");
	threadExit(0);
}
