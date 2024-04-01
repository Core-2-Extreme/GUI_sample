#include "definitions.hpp"
#include "system/types.hpp"

#include "system/menu.hpp"
#include "system/variables.hpp"

#include "system/draw/draw.hpp"

#include "system/util/converter.hpp"
#include "system/util/decoder.hpp"
#include "system/util/error.hpp"
#include "system/util/hid.hpp"
#include "system/util/log.hpp"
#include "system/util/speaker.hpp"
#include "system/util/util.hpp"

extern "C"
{
	#include "system/util/queue.h"
	#include "system/util/str.h"
}

//Include myself.
#include "sub_app4.hpp"


enum Sapp4_command
{
	NONE,

	PLAY_REQUEST,
	STOP_REQUEST,

	MAX = 0xFF,
};

enum Sapp4_speaker_state
{
	SPEAKER_IDLE,
	SPEAKER_PLAYING,
};


bool sapp4_main_run = false;
bool sapp4_thread_run = false;
bool sapp4_already_init = false;
bool sapp4_thread_suspend = true;
double sapp4_buffer_health = 0;
double sapp4_last_decoded_pos_ms = 0;
std::string sapp4_msg[DEF_SAPP4_NUM_OF_MSG];
Thread sapp4_init_thread, sapp4_exit_thread, sapp4_worker_thread;
Audio_info sapp4_audio_info;
Util_queue sapp4_command_queue;
Util_str sapp4_status = { 0, };
Sapp4_speaker_state sapp4_speaker_state;


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
	//Do nothing if app is suspended.
	if(aptShouldJumpToHome())
		return;

	if(Util_err_query_error_show_flag())
		Util_err_main(key);
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
	return Util_load_msg("sapp4_" + lang + ".txt", sapp4_msg, DEF_SAPP4_NUM_OF_MSG);
}

void Sapp4_init(bool draw)
{
	DEF_LOG_STRING("Initializing...");
	uint32_t result = DEF_ERR_OTHER;

	DEF_LOG_RESULT_SMART(result, Util_str_init(&sapp4_status), (result == DEF_SUCCESS), result);

	Util_add_watch(WATCH_HANDLE_SUB_APP4, &sapp4_status.sequencial_id, sizeof(sapp4_status.sequencial_id));

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
			Sapp4_draw_init_exit_message();
		else
			Util_sleep(20000);
	}

	if(!(var_model == CFG_MODEL_N2DSXL || var_model == CFG_MODEL_N3DSXL || var_model == CFG_MODEL_N3DS) || !var_core_2_available)
		APT_SetAppCpuTimeLimit(10);

	DEF_LOG_RESULT_SMART(result, threadJoin(sapp4_init_thread, DEF_THREAD_WAIT_TIME), result, result);
	threadFree(sapp4_init_thread);

	Util_str_clear(&sapp4_status);
	Sapp4_resume();

	DEF_LOG_STRING("Initialized.");
}

void Sapp4_exit(bool draw)
{
	DEF_LOG_STRING("Exiting...");
	uint32_t result = DEF_ERR_OTHER;

	sapp4_exit_thread = threadCreate(Sapp4_exit_thread, (void*)(""), DEF_STACKSIZE, DEF_THREAD_PRIORITY_NORMAL, 1, false);

	while(sapp4_already_init)
	{
		if(draw)
			Sapp4_draw_init_exit_message();
		else
			Util_sleep(20000);
	}

	DEF_LOG_RESULT_SMART(result, threadJoin(sapp4_exit_thread, DEF_THREAD_WAIT_TIME), result, result);
	threadFree(sapp4_exit_thread);

	Util_remove_watch(WATCH_HANDLE_SUB_APP4, &sapp4_status.sequencial_id);
	Util_str_free(&sapp4_status);
	var_need_reflesh = true;

	DEF_LOG_STRING("Exited.");
}

void Sapp4_main(void)
{
	int color = DEF_DRAW_BLACK;
	int back_color = DEF_DRAW_WHITE;
	double samples = 0;
	double current_pos = 0;
	Watch_handle_bit watch_handle_bit = (DEF_WATCH_HANDLE_BIT_GLOBAL | DEF_WATCH_HANDLE_BIT_SUB_APP4);

	//Calc speaker buffer health.
	//Sample format is S16, so 2 bytes per sample.
	//The buffer health in second is (number_of_samples / sample_rate / num_of_ch).
	samples = Util_speaker_get_available_buffer_size(0) / 2;
	if(sapp4_audio_info.sample_rate != 0 && sapp4_audio_info.ch != 0)
		sapp4_buffer_health = (samples / sapp4_audio_info.sample_rate / sapp4_audio_info.ch);

	//Current position is (time_of_newest_decoded_frame) - (speaker_buffer_health).
	current_pos = (sapp4_last_decoded_pos_ms / 1000) - sapp4_buffer_health;

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
			char msg[64];

			Draw_screen_ready(SCREEN_TOP_LEFT, back_color);

			Draw(sapp4_msg[0], 0, 20, 0.5, 0.5, color);

			//Draw controls.
			if(sapp4_speaker_state == SPEAKER_IDLE)
				Draw("Press A to play music.", 0, 40, 0.425, 0.425, color);
			else
				Draw("Press B to stop music.", 0, 40, 0.425, 0.425, color);

			//Draw buffer health.
			snprintf(msg, sizeof(msg), "%.2fs of data is cached in speaker buffer.", sapp4_buffer_health);
			Draw(msg, 0, 70, 0.425, 0.425, color);

			//Draw playback position.
			Draw("Current pos : " + Util_convert_seconds_to_time(current_pos), 0, 80, 0.425, 0.425, color);

			//Draw dsp warning.
			Draw((std::string)"If you can't hear any audio, then you need to dump dsp firmawre.\n"
			+ "On luma3ds >= v10.3, you can use luma3ds menu -> miscellaneous\n"
			+ "-> dump dsp firmware.\n"
			+ "On older luma3ds, run dsp1 (https://github.com/zoogie/DSP1/releases).", 0, 180, 0.45, 0.45, DEF_DRAW_RED);

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

static void Sapp4_draw_init_exit_message(void)
{
	int color = DEF_DRAW_BLACK;
	int back_color = DEF_DRAW_WHITE;
	Watch_handle_bit watch_handle_bit = (DEF_WATCH_HANDLE_BIT_GLOBAL | DEF_WATCH_HANDLE_BIT_SUB_APP4);

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

		Draw(sapp4_status.buffer, 0, 20, 0.65, 0.65, color);

		Draw_apply_draw();
	}
	else
		gspWaitForVBlank();
}

static void Sapp4_init_thread(void* arg)
{
	DEF_LOG_STRING("Thread started.");
	Result_with_string result;

	Util_str_set(&sapp4_status, "Initializing variables...");
	sapp4_speaker_state = SPEAKER_IDLE;
	sapp4_buffer_health = 0;
	//Add to watch to detect value changes, screen will be rerenderd when value is changed.
	Util_add_watch(WATCH_HANDLE_SUB_APP4, &sapp4_buffer_health, sizeof(sapp4_buffer_health));
	Util_add_watch(WATCH_HANDLE_SUB_APP4, &sapp4_last_decoded_pos_ms, sizeof(sapp4_last_decoded_pos_ms));

	Util_str_add(&sapp4_status, "\nInitializing queue...");
	//Create the queue for commands.
	DEF_LOG_RESULT_SMART(result.code, Util_queue_create(&sapp4_command_queue, 10), (result.code == DEF_SUCCESS), result.code);

	Util_str_add(&sapp4_status, "\nInitializing speaker...");
	//Init speaker.
	DEF_LOG_RESULT_SMART(result, Util_speaker_init(), (result.code == DEF_SUCCESS), result.code);

	Util_str_add(&sapp4_status, "\nStarting threads...");
	sapp4_thread_run = true;
	sapp4_worker_thread = threadCreate(Sapp4_worker_thread, (void*)(""), DEF_STACKSIZE, DEF_THREAD_PRIORITY_NORMAL, 0, false);

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
	DEF_LOG_RESULT_SMART(result, threadJoin(sapp4_worker_thread, DEF_THREAD_WAIT_TIME), result, result);

	Util_str_add(&sapp4_status, "\nCleaning up...");
	threadFree(sapp4_worker_thread);

	//Exit speaker.
	Util_speaker_exit();

	//Delete the queue.
	Util_queue_delete(&sapp4_command_queue);

	//Remove watch on exit
	Util_remove_watch(WATCH_HANDLE_SUB_APP4, &sapp4_buffer_health);
	Util_remove_watch(WATCH_HANDLE_SUB_APP4, &sapp4_last_decoded_pos_ms);

	sapp4_already_init = false;

	DEF_LOG_STRING("Thread exit.");
	threadExit(0);
}

static void Sapp4_worker_thread(void* arg)
{
	DEF_LOG_STRING("Thread started.");
	Result_with_string result;

	while (sapp4_thread_run)
	{
		uint32_t event_id = 0;

		while (sapp4_thread_suspend)
			Util_sleep(DEF_INACTIVE_THREAD_SLEEP_TIME);

		result.code = Util_queue_get(&sapp4_command_queue, &event_id, NULL, DEF_ACTIVE_THREAD_SLEEP_TIME * 20);
		if(result.code == 0)
		{
			//Got a command.
			DEF_LOG_FORMAT("Received event : %" PRIu32, event_id);

			switch((Sapp4_command)event_id)
			{
				case PLAY_REQUEST:
				{
					int num_of_audio = 0;
					int num_of_videos = 0;
					int num_of_subtitles = 0;
					char path[] = "romfs:/gfx/sound/sapp4/BigBuckBunny.mp3";
					//You can also load a file from SD card.
					//char path[] = "/test.mp3";

					//1. Open an input file.
					DEF_LOG_RESULT_SMART(result, Util_decoder_open_file(path, &num_of_audio, &num_of_videos, &num_of_subtitles, 0),
					(result.code == DEF_SUCCESS), result.code);

					//2. Initialize audio decoder.
					//Since we only interested in audio here, so we only initialize audio decoder.
					DEF_LOG_RESULT_SMART(result, Util_audio_decoder_init(num_of_audio, 0), (result.code == DEF_SUCCESS), result.code);

					if(result.code == 0)
					{
						//3. Get audio info.
						Util_audio_decoder_get_info(&sapp4_audio_info, 0, 0);

						//4. Set speaker parameters.
						DEF_LOG_RESULT_SMART(result, Util_speaker_set_audio_info(0, sapp4_audio_info.ch, sapp4_audio_info.sample_rate),
						(result.code == DEF_SUCCESS), result.code);

						if(result.code == 0)
							sapp4_speaker_state = SPEAKER_PLAYING;
					}

					if(result.code != 0)
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
			Result_with_string read_packet_result;
			//1. Read the next packet (next frame).
			//If we couldn't read the next packet, it usually means we reached EOF.
			//Or something went wrong such as file is corrupted (unlikely).
			read_packet_result = Util_decoder_read_packet(0);
			if(read_packet_result.code != 0)
			{
				//If audio playback has finished, close the file and reset speaker state.
				if(Util_speaker_get_available_buffer_num(0) == 0)
				{
					Util_speaker_clear_buffer(0);
					Util_decoder_close_file(0);
					sapp4_speaker_state = SPEAKER_IDLE;
				}
			}

			while(read_packet_result.code == 0)
			{
				bool is_buffer_full = false;
				bool key_frame = false;
				int packet_index = 0;
				Packet_type type = PACKET_TYPE_UNKNOWN;

				//2. Parse packet to check what type of packet it is.
				result = Util_decoder_parse_packet(&type, &packet_index, &key_frame, 0);
				if(result.code == 0)
				{
					if(type == PACKET_TYPE_AUDIO)
					{
						int samples = 0;
						uint8_t* audio = NULL;
						double pos = 0;
						Audio_converter_parameters parameters;

						//3. Prepare packet.
						//Since we are interested in audio, so tell API to we want to use this packet.
						result = Util_decoder_ready_audio_packet(packet_index, 0);
						if(result.code == 0)
						{
							//4. Decode audio.
							result = Util_audio_decoder_decode(&samples, &audio, &pos, packet_index, 0);
							if(result.code == 0)
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
								parameters.out_sample_format = SAMPLE_FORMAT_S16;
								parameters.out_sample_rate = sapp4_audio_info.sample_rate;

								result = Util_converter_convert_audio(&parameters);
								if(result.code == 0)
								{
									while(true)
									{
										//6. Add converted pcm data to speaker queue.
										result = Util_speaker_add_buffer(0, parameters.converted, (parameters.out_samples * 2 * parameters.out_ch));
										if(result.code == 0)
											break;
										else if(result.code == DEF_ERR_TRY_AGAIN)
										{
											//If speaker buffer is full, wait until free space is available.
											//Also set is_buffer_full flag to break from the decode loop.
											is_buffer_full = true;
											Util_sleep(5000);
										}
										else
										{
											DEF_LOG_RESULT(Util_speaker_add_buffer, false, result.code);
											break;
										}
									}
								}
								else
									DEF_LOG_RESULT(Util_converter_convert_audio, false, result.code);
							}
							else
								DEF_LOG_RESULT(Util_audio_decoder_decode, false, result.code);
						}
						else
							DEF_LOG_RESULT(Util_decoder_ready_audio_packet, false, result.code);

						free(audio);
						free(parameters.converted);
						audio = NULL;
						parameters.converted = NULL;
					}
					else if(type == PACKET_TYPE_VIDEO)//We are not interested in video and subtitle here, so just skip them.
						Util_decoder_skip_video_packet(packet_index, 0);
					else if(type == PACKET_TYPE_SUBTITLE)
						Util_decoder_skip_subtitle_packet(packet_index, 0);

					//If speaker buffer is full, break and check for new command, then come back here later.
					if(is_buffer_full)
						break;
					else//If not, read the next packet.
						read_packet_result = Util_decoder_read_packet(0);
				}
				else
					DEF_LOG_RESULT(Util_decoder_parse_packet, false, result.code);
			}
		}
	}

	DEF_LOG_STRING("Thread exit.");
	threadExit(0);
}
