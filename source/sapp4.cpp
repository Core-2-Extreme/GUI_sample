#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>

#include "system/menu.hpp"
#include "system/sem.hpp"

extern "C"
{
	#include "system/draw/draw.h"
	#include "system/util/converter.h"
	#include "system/util/cpu_usage.h"
	#include "system/util/decoder.h"
	#include "system/util/err.h"
	#include "system/util/expl.h"
	#include "system/util/hid.h"
	#include "system/util/log.h"
	#include "system/util/queue.h"
	#include "system/util/speaker.h"
	#include "system/util/str.h"
	#include "system/util/thread_types.h"
	#include "system/util/util_c.h"
	#include "system/util/watch.h"
}

//Include myself.
#include "sapp4.hpp"


typedef enum
{
	NONE,

	PLAY_REQUEST,
	STOP_REQUEST,

	MAX = 0xFF,
} Sapp4_command;

DEF_LOG_ENUM_DEBUG
(
	Sapp4_command,
	NONE,
	PLAY_REQUEST,
	STOP_REQUEST,
	MAX
);

typedef enum
{
	SPEAKER_IDLE,
	SPEAKER_PLAYING,
} Sapp4_speaker_state;

DEF_LOG_ENUM_DEBUG
(
	Sapp4_speaker_state,
	SPEAKER_IDLE,
	SPEAKER_PLAYING
);


bool sapp4_main_run = false;
bool sapp4_thread_run = false;
bool sapp4_already_init = false;
bool sapp4_thread_suspend = true;
double sapp4_buffer_health = 0;
double sapp4_last_decoded_pos_ms = 0;
Thread sapp4_init_thread = NULL, sapp4_exit_thread = NULL, sapp4_worker_thread = NULL;
Media_a_info sapp4_audio_info = { 0, };
Queue_data sapp4_command_queue = { 0, };
Str_data sapp4_status = { 0, };
Str_data sapp4_msg[DEF_SAPP4_NUM_OF_MSG] = { 0, };
Sapp4_speaker_state sapp4_speaker_state = SPEAKER_IDLE;


static void Sapp4_draw_init_exit_message(void);
static void Sapp4_init_thread(void* arg);
static void Sapp4_exit_thread(void* arg);
static void Sapp4_worker_thread(void* arg);


bool Sapp4_query_init_flag(void)
{
	return sapp4_already_init;
}

bool Sapp4_query_running_flag(void)
{
	return sapp4_main_run;
}

void Sapp4_hid(Hid_info key)
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
		Sapp4_command command = NONE;

		if(Util_hid_is_pressed(key, *Draw_get_bot_ui_button()))
			Draw_get_bot_ui_button()->selected = true;
		else if (key.p_start || (Util_hid_is_released(key, *Draw_get_bot_ui_button()) && Draw_get_bot_ui_button()->selected))
			Sapp4_suspend();

		if(key.p_a)//Play audio.
		{
			if(sapp4_speaker_state == SPEAKER_IDLE)
				command = PLAY_REQUEST;
		}
		else if(key.p_b)//Stop audio.
		{
			if(sapp4_speaker_state == SPEAKER_PLAYING)
				command = STOP_REQUEST;
		}

		if(command != NONE)
		{
			uint32_t result = DEF_ERR_OTHER;
			DEF_LOG_RESULT_SMART(result, Util_queue_add(&sapp4_command_queue, command, NULL, 10000, QUEUE_OPTION_DO_NOT_ADD_IF_EXIST), (result == DEF_SUCCESS), result);
		}
	}

	if(!key.p_touch && !key.h_touch)
		Draw_get_bot_ui_button()->selected = false;

	if(Util_log_query_log_show_flag())
		Util_log_main(key);
}

void Sapp4_resume(void)
{
	sapp4_thread_suspend = false;
	sapp4_main_run = true;
	Draw_set_refresh_needed(true);
	Menu_suspend();
}

void Sapp4_suspend(void)
{
	sapp4_thread_suspend = true;
	sapp4_main_run = false;
	Draw_set_refresh_needed(true);
	Menu_resume();
}

uint32_t Sapp4_load_msg(const char* lang)
{
	char file_name[32] = { 0, };

	snprintf(file_name, sizeof(file_name), "sapp4_%s.txt", (lang ? lang : ""));
	return Util_load_msg(file_name, sapp4_msg, DEF_SAPP4_NUM_OF_MSG);
}

void Sapp4_init(bool draw)
{
	DEF_LOG_STRING("Initializing...");
	uint32_t result = DEF_ERR_OTHER;
	Sem_state state = { 0, };

	Sem_get_state(&state);
	DEF_LOG_RESULT_SMART(result, Util_str_init(&sapp4_status), (result == DEF_SUCCESS), result);

	Util_watch_add(WATCH_HANDLE_SUB_APP4, &sapp4_status.sequencial_id, sizeof(sapp4_status.sequencial_id));

	if(DEF_SEM_MODEL_IS_NEW(state.console_model) && Util_is_core_available(2))
		sapp4_init_thread = threadCreate(Sapp4_init_thread, (void*)(""), DEF_THREAD_STACKSIZE, DEF_THREAD_PRIORITY_NORMAL, 2, false);
	else
	{
		APT_SetAppCpuTimeLimit(80);
		sapp4_init_thread = threadCreate(Sapp4_init_thread, (void*)(""), DEF_THREAD_STACKSIZE, DEF_THREAD_PRIORITY_NORMAL, 1, false);
	}

	while(!sapp4_already_init)
	{
		if(draw)
			Sapp4_draw_init_exit_message();
		else
			Util_sleep(20000);
	}

	if(!DEF_SEM_MODEL_IS_NEW(state.console_model) || !Util_is_core_available(2))
		APT_SetAppCpuTimeLimit(10);

	DEF_LOG_RESULT_SMART(result, threadJoin(sapp4_init_thread, DEF_THREAD_WAIT_TIME), (result == DEF_SUCCESS), result);
	threadFree(sapp4_init_thread);

	Util_str_clear(&sapp4_status);
	Sapp4_resume();

	DEF_LOG_STRING("Initialized.");
}

void Sapp4_exit(bool draw)
{
	DEF_LOG_STRING("Exiting...");
	uint32_t result = DEF_ERR_OTHER;

	sapp4_exit_thread = threadCreate(Sapp4_exit_thread, (void*)(""), DEF_THREAD_STACKSIZE, DEF_THREAD_PRIORITY_NORMAL, 1, false);

	while(sapp4_already_init)
	{
		if(draw)
			Sapp4_draw_init_exit_message();
		else
			Util_sleep(20000);
	}

	DEF_LOG_RESULT_SMART(result, threadJoin(sapp4_exit_thread, DEF_THREAD_WAIT_TIME), (result == DEF_SUCCESS), result);
	threadFree(sapp4_exit_thread);

	Util_watch_remove(WATCH_HANDLE_SUB_APP4, &sapp4_status.sequencial_id);
	Util_str_free(&sapp4_status);
	Draw_set_refresh_needed(true);

	DEF_LOG_STRING("Exited.");
}

void Sapp4_main(void)
{
	uint32_t color = DEF_DRAW_BLACK;
	uint32_t back_color = DEF_DRAW_WHITE;
	double samples = 0;
	double current_pos = 0;
	Watch_handle_bit watch_handle_bit = (DEF_WATCH_HANDLE_BIT_GLOBAL | DEF_WATCH_HANDLE_BIT_SUB_APP4);
	Sem_config config = { 0, };
	Sem_state state = { 0, };

	Sem_get_config(&config);
	Sem_get_state(&state);

	//Calc speaker buffer health.
	//Sample format is S16, so 2 bytes per sample.
	//The buffer health in second is (number_of_samples / sample_rate / num_of_ch).
	samples = Util_speaker_get_available_buffer_size(0) / 2;
	if(sapp4_audio_info.sample_rate != 0 && sapp4_audio_info.ch != 0)
		sapp4_buffer_health = (samples / sapp4_audio_info.sample_rate / sapp4_audio_info.ch);

	//Current position is (time_of_newest_decoded_frame) - (speaker_buffer_health).
	current_pos = (sapp4_last_decoded_pos_ms / 1000) - sapp4_buffer_health;

	if (config.is_night)
	{
		color = DEF_DRAW_WHITE;
		back_color = DEF_DRAW_BLACK;
	}

	//Check if we should update the screen.
	if(Util_watch_is_changed(watch_handle_bit) || Draw_is_refresh_needed() || !config.is_eco)
	{
		Str_data temp_msg = { 0, };

		Draw_set_refresh_needed(false);
		Util_str_init(&temp_msg);

		Draw_frame_ready();

		if(config.is_top_lcd_on)
		{
			Str_data time = { 0, };

			Draw_screen_ready(DRAW_SCREEN_TOP_LEFT, back_color);

			Draw(&sapp4_msg[0], 0, 20, 0.5, 0.5, color);

			//Draw controls.
			if(sapp4_speaker_state == SPEAKER_IDLE)
				Draw_c("Press A to play music.", 0, 40, 0.425, 0.425, color);
			else
				Draw_c("Press B to stop music.", 0, 40, 0.425, 0.425, color);

			//Draw buffer health.
			Util_str_format(&temp_msg, "%.2fs of data is cached in speaker buffer.", sapp4_buffer_health);
			Draw(&temp_msg, 0, 70, 0.425, 0.425, color);

			//Draw playback position.
			if(Util_convert_seconds_to_time(current_pos, &time) == DEF_SUCCESS)
			{
				Util_str_format(&temp_msg, "Current pos : %s", time.buffer);
				Draw(&temp_msg, 0, 80, 0.425, 0.425, color);
			}

			//Draw current state.
			Util_str_format(&temp_msg, "State : %s (%" PRIu32 ")", Sapp4_speaker_state_get_name(sapp4_speaker_state), (uint32_t)sapp4_speaker_state);
			Draw(&temp_msg, 0, 90, 0.5, 0.5, color);

			//Draw dsp warning.
			Draw_c("If you can't hear any audio, then you need to dump dsp firmawre.\n"
			"On luma3ds >= v10.3, you can use luma3ds menu -> miscellaneous\n"
			"-> dump dsp firmware.\n"
			"On older luma3ds, run dsp1 (https://github.com/zoogie/DSP1/releases).", 0, 180, 0.45, 0.45, DEF_DRAW_RED);

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

			Util_str_free(&time);
		}

		if(config.is_bottom_lcd_on)
		{
			Draw_screen_ready(DRAW_SCREEN_BOTTOM, back_color);

			Draw_c(DEF_SAPP4_VER, 0, 0, 0.4, 0.4, DEF_DRAW_GREEN);

			if(Util_expl_query_show_flag())
				Util_expl_draw();

			if(Util_err_query_error_show_flag())
				Util_err_draw();

			Draw_bot_ui();
		}

		Draw_apply_draw();

		Util_str_free(&temp_msg);
	}
	else
		gspWaitForVBlank();
}

static void Sapp4_draw_init_exit_message(void)
{
	uint32_t color = DEF_DRAW_BLACK;
	uint32_t back_color = DEF_DRAW_WHITE;
	Watch_handle_bit watch_handle_bit = (DEF_WATCH_HANDLE_BIT_GLOBAL | DEF_WATCH_HANDLE_BIT_SUB_APP4);
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

		Draw(&sapp4_status, 0, 20, 0.65, 0.65, color);

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

			Draw(&sapp4_status, 0, 20, 0.65, 0.65, color);
		}

		Draw_apply_draw();
	}
	else
		gspWaitForVBlank();
}

static void Sapp4_init_thread(void* arg)
{
	DEF_LOG_STRING("Thread started.");
	uint32_t result = DEF_ERR_OTHER;

	Util_str_set(&sapp4_status, "Initializing variables...");
	sapp4_speaker_state = SPEAKER_IDLE;
	sapp4_buffer_health = 0;
	//Add to watch to detect value changes, screen will be rerenderd when value is changed.
	Util_watch_add(WATCH_HANDLE_SUB_APP4, &sapp4_buffer_health, sizeof(sapp4_buffer_health));
	Util_watch_add(WATCH_HANDLE_SUB_APP4, &sapp4_last_decoded_pos_ms, sizeof(sapp4_last_decoded_pos_ms));
	Util_watch_add(WATCH_HANDLE_SUB_APP4, &sapp4_speaker_state, sizeof(sapp4_speaker_state));

	Util_str_add(&sapp4_status, "\nInitializing queue...");
	//Create the queue for commands.
	DEF_LOG_RESULT_SMART(result, Util_queue_create(&sapp4_command_queue, 10), (result == DEF_SUCCESS), result);

	Util_str_add(&sapp4_status, "\nInitializing speaker...");
	//Init speaker.
	DEF_LOG_RESULT_SMART(result, Util_speaker_init(), (result == DEF_SUCCESS), result);

	Util_str_add(&sapp4_status, "\nStarting threads...");
	sapp4_thread_run = true;
	sapp4_worker_thread = threadCreate(Sapp4_worker_thread, (void*)(""), DEF_THREAD_STACKSIZE, DEF_THREAD_PRIORITY_NORMAL, 0, false);

	sapp4_already_init = true;

	DEF_LOG_STRING("Thread exit.");
	threadExit(0);
}

static void Sapp4_exit_thread(void* arg)
{
	DEF_LOG_STRING("Thread started.");
	uint32_t result = DEF_ERR_OTHER;

	sapp4_thread_suspend = false;
	sapp4_thread_run = false;

	Util_str_set(&sapp4_status, "Exiting threads...");
	DEF_LOG_RESULT_SMART(result, threadJoin(sapp4_worker_thread, DEF_THREAD_WAIT_TIME), (result == DEF_SUCCESS), result);

	Util_str_add(&sapp4_status, "\nCleaning up...");
	threadFree(sapp4_worker_thread);

	//Exit speaker.
	Util_speaker_exit();

	//Delete the queue.
	Util_queue_delete(&sapp4_command_queue);

	//Remove watch on exit
	Util_watch_remove(WATCH_HANDLE_SUB_APP4, &sapp4_buffer_health);
	Util_watch_remove(WATCH_HANDLE_SUB_APP4, &sapp4_last_decoded_pos_ms);
	Util_watch_remove(WATCH_HANDLE_SUB_APP4, &sapp4_speaker_state);

	sapp4_already_init = false;

	DEF_LOG_STRING("Thread exit.");
	threadExit(0);
}

static void Sapp4_worker_thread(void* arg)
{
	DEF_LOG_STRING("Thread started.");

	while (sapp4_thread_run)
	{
		uint32_t event_id = 0;
		uint32_t result = DEF_ERR_OTHER;

		while (sapp4_thread_suspend)
			Util_sleep(DEF_THREAD_INACTIVE_SLEEP_TIME);

		result = Util_queue_get(&sapp4_command_queue, &event_id, NULL, DEF_THREAD_ACTIVE_SLEEP_TIME * 20);
		if(result == DEF_SUCCESS)
		{
			//Got a command.
			DEF_LOG_FORMAT("Received event : %s (%" PRIu32 ")", Sapp4_command_get_name((Sapp4_command)event_id), event_id);

			switch((Sapp4_command)event_id)
			{
				case PLAY_REQUEST:
				{
					uint8_t num_of_audio = 0;
					uint8_t num_of_videos = 0;
					uint8_t num_of_subtitles = 0;
					char path[] = "romfs:/gfx/sound/sapp4/BigBuckBunny.mp3";
					//You can also load a file from SD card.
					//char path[] = "/test.mp3";

					//1. Open an input file.
					DEF_LOG_RESULT_SMART(result, Util_decoder_open_file(path, &num_of_audio, &num_of_videos, &num_of_subtitles, 0), (result == DEF_SUCCESS), result);

					//2. Initialize audio decoder.
					//Since we only interested in audio here, so we only initialize audio decoder.
					DEF_LOG_RESULT_SMART(result, Util_decoder_audio_init(num_of_audio, 0), (result == DEF_SUCCESS), result);

					if(result == DEF_SUCCESS)
					{
						//3. Get audio info.
						Util_decoder_audio_get_info(&sapp4_audio_info, 0, 0);

						//4. Set speaker parameters.
						DEF_LOG_RESULT_SMART(result, Util_speaker_set_audio_info(0, sapp4_audio_info.ch, sapp4_audio_info.sample_rate),
						(result == DEF_SUCCESS), result);

						if(result == DEF_SUCCESS)
							sapp4_speaker_state = SPEAKER_PLAYING;
					}

					if(result != DEF_SUCCESS)
					{
						//Error.
						Util_speaker_clear_buffer(0);
						Util_decoder_close_file(0);
					}

					break;
				}
				case STOP_REQUEST:
				{
					if(sapp4_speaker_state == SPEAKER_PLAYING)
					{
						//Upon receiving stop request, close file and clear speaker buffer.
						Util_speaker_clear_buffer(0);
						Util_decoder_close_file(0);
					}

					sapp4_last_decoded_pos_ms = 0;
					sapp4_speaker_state = SPEAKER_IDLE;
					break;
				}
				default:
					break;
			}
		}

		if(sapp4_speaker_state == SPEAKER_PLAYING)
		{
			uint32_t read_packet_result = DEF_ERR_OTHER;
			//1. Read the next packet (next frame).
			//If we couldn't read the next packet, it usually means we reached EOF.
			//Or something went wrong such as file is corrupted (unlikely).
			read_packet_result = Util_decoder_read_packet(0);
			if(read_packet_result != DEF_SUCCESS)
			{
				//If audio playback has finished, close the file and reset speaker state.
				if(Util_speaker_get_available_buffer_num(0) == 0)
				{
					Util_speaker_clear_buffer(0);
					Util_decoder_close_file(0);
					sapp4_speaker_state = SPEAKER_IDLE;
				}
			}

			while(read_packet_result == DEF_SUCCESS)
			{
				bool is_buffer_full = false;
				bool key_frame = false;
				uint8_t packet_index = 0;
				Media_packet_type type = MEDIA_PACKET_TYPE_UNKNOWN;

				//2. Parse packet to check what type of packet it is.
				result = Util_decoder_parse_packet(&type, &packet_index, &key_frame, 0);
				if(result == DEF_SUCCESS)
				{
					if(type == MEDIA_PACKET_TYPE_AUDIO)
					{
						uint8_t* audio = NULL;
						uint32_t samples = 0;
						double pos = 0;
						Converter_audio_parameters parameters = { 0, };

						//3. Prepare packet.
						//Since we are interested in audio, so tell API to we want to use this packet.
						result = Util_decoder_ready_audio_packet(packet_index, 0);
						if(result == DEF_SUCCESS)
						{
							//4. Decode audio.
							result = Util_decoder_audio_decode(&samples, &audio, &pos, packet_index, 0);
							if(result == DEF_SUCCESS)
							{
								//Set last decoded frame timestamp.
								//This is used to display current playback position.
								sapp4_last_decoded_pos_ms = pos;

								//5. Convert sample format.
								//Popular formats such as mp3 and aac use FLOAT as native sample format,
								//however speaker API requires S16 as input format,
								//so convert sample format here (don't change ch and sample rate).
								parameters.source = audio;
								parameters.converted = NULL;
								parameters.in_ch = sapp4_audio_info.ch;
								parameters.in_sample_format = sapp4_audio_info.sample_format;
								parameters.in_sample_rate = sapp4_audio_info.sample_rate;
								parameters.in_samples = samples;
								parameters.out_ch = sapp4_audio_info.ch;
								parameters.out_sample_format = RAW_SAMPLE_S16;
								parameters.out_sample_rate = sapp4_audio_info.sample_rate;

								result = Util_converter_convert_audio(&parameters);
								if(result == DEF_SUCCESS)
								{
									while(true)
									{
										//6. Add converted pcm data to speaker queue.
										result = Util_speaker_add_buffer(0, parameters.converted, (parameters.out_samples * 2 * parameters.out_ch));
										if(result == DEF_SUCCESS)
											break;
										else if(result == DEF_ERR_TRY_AGAIN)
										{
											//If speaker buffer is full, wait until free space is available.
											//Also set is_buffer_full flag to break from the decode loop.
											is_buffer_full = true;
											Util_sleep(5000);
										}
										else
										{
											DEF_LOG_RESULT(Util_speaker_add_buffer, false, result);
											break;
										}
									}
								}
								else
									DEF_LOG_RESULT(Util_converter_convert_audio, false, result);
							}
							else
								DEF_LOG_RESULT(Util_decoder_audio_decode, false, result);
						}
						else
							DEF_LOG_RESULT(Util_decoder_ready_audio_packet, false, result);

						free(audio);
						free(parameters.converted);
						audio = NULL;
						parameters.converted = NULL;
					}
					else if(type == MEDIA_PACKET_TYPE_VIDEO)//We are not interested in video and subtitle here, so just skip them.
						Util_decoder_skip_video_packet(packet_index, 0);
					else if(type == MEDIA_PACKET_TYPE_SUBTITLE)
						Util_decoder_skip_subtitle_packet(packet_index, 0);

					//If speaker buffer is full, break and check for new command, then come back here later.
					if(is_buffer_full)
						break;
					else//If not, read the next packet.
						read_packet_result = Util_decoder_read_packet(0);
				}
				else
					DEF_LOG_RESULT(Util_decoder_parse_packet, false, result);
			}
		}
	}

	DEF_LOG_STRING("Thread exit.");
	threadExit(0);
}
