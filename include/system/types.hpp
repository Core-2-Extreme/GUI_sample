#ifndef TYPES_HPP
#define TYPES_HPP

#include <string>

#include <3ds.h>
#include <citro2d.h>

#include "system_definitions.hpp"

#include "system/util/converter_types.h"

enum Multi_thread_type
{
	THREAD_TYPE_INVALID = -1,

	THREAD_TYPE_NONE,	//No multi-threading, using single thread.
	THREAD_TYPE_FRAME,	//Frame level multi-threading.
	THREAD_TYPE_SLICE,	//Slice level multi-threading.
	THREAD_TYPE_AUTO,	//Auto (only used when request multi-threading mode).

	THREAD_TYPE_MAX,
};

enum Packet_type
{
	PACKET_TYPE_INVALID = -1,

	PACKET_TYPE_UNKNOWN,	//This packet contains unknown data.
	PACKET_TYPE_AUDIO,		//This packet contains audio data.
	PACKET_TYPE_VIDEO,		//This packet contains video data.
	PACKET_TYPE_SUBTITLE,	//This packet contains subtitle data.

	PACKET_TYPE_MAX,
};

enum Seek_flag
{
	SEEK_FLAG_NONE		= 0,		//No seek flag.
	SEEK_FLAG_BACKWARD	= (1 << 1),	//Seek backward.
	SEEK_FLAG_BYTE		= (1 << 2),	//Seek to given byte offset instead of time.
	SEEK_FLAG_ANY		= (1 << 3),	//Seek to any location including non key frame.
	SEEK_FLAG_FRAME		= (1 << 4),	//Seek to given frame number instead of time.
};

enum Audio_codec
{
	AUDIO_CODEC_INVALID = -1,

	AUDIO_CODEC_AAC,	//Advanced audio coding.
	AUDIO_CODEC_AC3,	//Audio codec 3.
	AUDIO_CODEC_MP2,	//Mpeg audio layer 2.
	AUDIO_CODEC_MP3,	//Mpeg audio layer 3.

	AUDIO_CODEC_MAX,
};

enum Video_codec
{
	VIDEO_CODEC_INVALID = -1,

	VIDEO_CODEC_MJPEG,		//Motion jpeg.
	VIDEO_CODEC_H264,		//Advanced video coding.
	VIDEO_CODEC_MPEG4,		//Mpeg4 part 2.
	VIDEO_CODEC_MPEG2VIDEO,	//Mpeg2 video.

	VIDEO_CODEC_MAX,
};

enum Image_codec
{
	IMAGE_CODEC_INVALID = -1,

	IMAGE_CODEC_PNG,	//Portable network graphic.
	IMAGE_CODEC_JPG,	//Joint photographic experts group.
	IMAGE_CODEC_BMP,	//Bitmap.
	IMAGE_CODEC_TGA,	//Truevision TGA.

	IMAGE_CODEC_MAX,
};

enum Keyboard_button
{
	KEYBOARD_BUTTON_INVALID = -1,

	KEYBOARD_BUTTON_NONE,	//No button was pressed.
	KEYBOARD_BUTTON_LEFT,	//Left button (usually cancel) was pressed.
	KEYBOARD_BUTTON_MIDDLE,	//Middle button (usually I forgot) was pressed.
	KEYBOARD_BUTTON_RIGHT,	//Right button (usually confirm) was pressed.

	KEYBOARD_BUTTON_MAX,
};

enum Watch_handle
{
	WATCH_HANDLE_INVALID = -1,

	WATCH_HANDLE_GLOBAL,			//Watch handle for global data.
	WATCH_HANDLE_MAIN_MENU,			//(menu.cpp) Watch handle for main menu.
	WATCH_HANDLE_SETTINGS_MENU,		//(setting_menu.cpp) Watch handle for settings menu.
	WATCH_HANDLE_SUB_APP0,			//(sub_app0.cpp) Watch handle for sub app0.
	WATCH_HANDLE_SUB_APP1,			//(sub_app1.cpp) Watch handle for sub app1.
	WATCH_HANDLE_SUB_APP2,			//(sub_app2.cpp) Watch handle for sub app2.
	WATCH_HANDLE_SUB_APP3,			//(sub_app3.cpp) Watch handle for sub app3.
	WATCH_HANDLE_SUB_APP4,			//(sub_app4.cpp) Watch handle for sub app4.
	WATCH_HANDLE_SUB_APP5,			//(sub_app5.cpp) Watch handle for sub app5.
	WATCH_HANDLE_SUB_APP6,			//(sub_app6.cpp) Watch handle for sub app6.
	WATCH_HANDLE_SUB_APP7,			//(sub_app7.cpp) Watch handle for sub app7.

	WATCH_HANDLE_MAX,
	WATCH_HANDLE_FORCE_8BIT = INT8_MAX,
};

typedef uint16_t Watch_handle_bit;
#define	DEF_WATCH_HANDLE_BIT_NONE			(Watch_handle_bit)(0 << 0)							//No watch handles.
#define	DEF_WATCH_HANDLE_BIT_GLOBAL			(Watch_handle_bit)(1 << WATCH_HANDLE_GLOBAL)		//Watch handle bit for WATCH_HANDLE_GLOBAL.
#define	DEF_WATCH_HANDLE_BIT_MAIN_MENU		(Watch_handle_bit)(1 << WATCH_HANDLE_MAIN_MENU)		//Watch handle bit for WATCH_HANDLE_MAIN_MENU.
#define	DEF_WATCH_HANDLE_BIT_SETTINGS_MENU	(Watch_handle_bit)(1 << WATCH_HANDLE_SETTINGS_MENU)	//Watch handle bit for WATCH_HANDLE_SETTINGS_MENU.
#define	DEF_WATCH_HANDLE_BIT_SUB_APP0		(Watch_handle_bit)(1 << WATCH_HANDLE_SUB_APP0)		//Watch handle bit for WATCH_HANDLE_SUB_APP0.
#define	DEF_WATCH_HANDLE_BIT_SUB_APP1		(Watch_handle_bit)(1 << WATCH_HANDLE_SUB_APP1)		//Watch handle bit for WATCH_HANDLE_SUB_APP1.
#define	DEF_WATCH_HANDLE_BIT_SUB_APP2		(Watch_handle_bit)(1 << WATCH_HANDLE_SUB_APP2)		//Watch handle bit for WATCH_HANDLE_SUB_APP2.
#define	DEF_WATCH_HANDLE_BIT_SUB_APP3		(Watch_handle_bit)(1 << WATCH_HANDLE_SUB_APP3)		//Watch handle bit for WATCH_HANDLE_SUB_APP3.
#define	DEF_WATCH_HANDLE_BIT_SUB_APP4		(Watch_handle_bit)(1 << WATCH_HANDLE_SUB_APP4)		//Watch handle bit for WATCH_HANDLE_SUB_APP4.
#define	DEF_WATCH_HANDLE_BIT_SUB_APP5		(Watch_handle_bit)(1 << WATCH_HANDLE_SUB_APP5)		//Watch handle bit for WATCH_HANDLE_SUB_APP5.
#define	DEF_WATCH_HANDLE_BIT_SUB_APP6		(Watch_handle_bit)(1 << WATCH_HANDLE_SUB_APP6)		//Watch handle bit for WATCH_HANDLE_SUB_APP6.
#define	DEF_WATCH_HANDLE_BIT_SUB_APP7		(Watch_handle_bit)(1 << WATCH_HANDLE_SUB_APP7)		//Watch handle bit for WATCH_HANDLE_SUB_APP7.

struct Watch_data
{
	void* original_address = NULL;				//Original data address.
	void* previous_data = NULL;					//Previous data.
	uint32_t data_length = 0;					//Data length for this data.
	Watch_handle handle = WATCH_HANDLE_INVALID;	//Watch handle.
};

struct Result_with_string
{
	std::string string = "[Success] ";
	std::string error_description = "";
	uint code = DEF_SUCCESS;
};

struct Audio_info
{
	int bitrate = 0;					//Audio bitrate in Bps.
	int sample_rate = 0;				//Audio smaple rate in Hz.
	int ch = 0;							//Number of audio channels.
	std::string format_name = "";		//Audio codec name.
	std::string short_format_name = "";	//Audio short codec name.
	double duration = 0;				//Audio track duration in seconds.
	std::string track_lang = "";		//Audio track language.
	Sample_format sample_format = SAMPLE_FORMAT_INVALID;	//Audio sample format.
};

struct Video_info
{
	int width = 0;						//Video width.
	int height = 0;						//Video height.
	int codec_width = 0;				//Video codec width (actual image width).
	int codec_height = 0;				//Video codec height (actual image height).
	double framerate = 0;				//Video framerate.
	std::string format_name = "";		//Video codec name.
	std::string short_format_name = "";	//Video short codec name.
	double duration = 0;				//Video track duration in seconds.
	Multi_thread_type thread_type = THREAD_TYPE_NONE;	//Threading mode.
	double sar_width = 1;				//Sample aspect ratio for width.
	double sar_height = 1;				//Sample aspect ratio for height.
	Pixel_format pixel_format = PIXEL_FORMAT_INVALID;	//Video pixel format.
};

struct Subtitle_info
{
	std::string format_name = "";		//Subtitle codec name.
	std::string short_format_name = "";	//Subtitle short codec name.
	std::string track_lang = "";		//Subtitle track language.
};

struct Subtitle_data
{
	uint8_t* bitmap = NULL;	//Subtitle bitmap, this may NULL.
	int bitmap_x = 0;		//X (horizontal) position, this field will be set if bitmap is not NULL.
	int bitmap_y = 0;		//Y (vertical) position, this field will be set if bitmap is not NULL.
	int bitmap_width = 0;	//Bitmap width, this field will be set if bitmap is not NULL.
	int bitmap_height = 0;	//Bitmap height, this field will be set if bitmap is not NULL.
	double start_time = 0;	//Start time in ms for this subtitle data. subtitle should be displayed if (start_time <= current_time <= end_time).
	double end_time = 0;	//End time in ms for this subtitle data. subtitle should be displayed if (start_time <= current_time <= end_time).
	std::string text = "";	//Subtitle text.
};

#endif
