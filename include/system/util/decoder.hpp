#pragma once

Result_with_string Util_mvd_video_decoder_init(int session);

Result_with_string Util_decoder_open_file(std::string file_path, int* num_of_video_tracks, int* num_of_audio_tracks, int session);

Result_with_string Util_audio_decoder_init(int num_of_audio_tracks, int session);

Result_with_string Util_video_decoder_init(int low_resolution, int num_of_video_tracks, int session);

void Util_audio_decoder_get_info(int* bitrate, int* sample_rate, int* ch, std::string* format_name, double* duration, int audio_index, std::string* track_lang, int session);

void Util_video_decoder_get_info(int* width, int* height, double* framerate, std::string* format_name, double* duration, int video_index, int session);

Result_with_string Util_decoder_read_packet(std::string* type, int* packet_index, bool* key_frame, int session);

Result_with_string Util_decoder_ready_audio_packet(int packet_index, int session);

Result_with_string Util_decoder_ready_video_packet(int packet_index, int session);

void Util_decoder_skip_audio_packet(int packet_index, int session);

void Util_decoder_skip_video_packet(int packet_index, int session);

Result_with_string Util_audio_decoder_decode(int* size, u8** raw_data, double* current_pos, int packet_index, int session);

Result_with_string Util_video_decoder_decode(int* width, int* height, double* current_pos, int packet_index, int session);

Result_with_string Util_mvd_video_decoder_decode(int* width, int* height, double* current_pos, int session);

Result_with_string Util_video_decoder_get_image(u8** raw_data, int width, int height, int packet_index, int session);

Result_with_string Util_mvd_video_decoder_get_image(u8** raw_data, int width, int height, int session);

Result_with_string Util_decoder_seek(u64 seek_pos, int flag, int session);

void Util_decoder_close_file(int session);

void Util_audio_decoder_exit(int session);

void Util_video_decoder_exit(int session);

void Util_mvd_video_decoder_exit(void);

Result_with_string Util_image_decoder_decode(std::string file_name, u8** raw_data, int* width, int* height);