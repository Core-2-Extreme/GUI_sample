#if !defined(DEF_MEDIA_TYPES_H)
#define DEF_MEDIA_TYPES_H
#include <stdbool.h>
#include <stdint.h>
#include "raw_types.h"

typedef enum
{
	VIDEO_CODEC_INVALID = -1,

	VIDEO_CODEC_MJPEG,		//Motion jpeg.
	VIDEO_CODEC_H264,		//Advanced video coding.
	VIDEO_CODEC_MPEG4,		//Mpeg4 part 2.
	VIDEO_CODEC_MPEG2VIDEO,	//Mpeg2 video.

	VIDEO_CODEC_MAX,
} Video_codec;

typedef enum
{
	AUDIO_CODEC_INVALID = -1,

	AUDIO_CODEC_AAC,	//Advanced audio coding.
	AUDIO_CODEC_AC3,	//Audio codec 3.
	AUDIO_CODEC_MP2,	//Mpeg audio layer 2.
	AUDIO_CODEC_MP3,	//Mpeg audio layer 3.

	AUDIO_CODEC_MAX,
} Audio_codec;

typedef enum
{
	IMAGE_CODEC_INVALID = -1,

	IMAGE_CODEC_PNG,	//Portable network graphic.
	IMAGE_CODEC_JPG,	//Joint photographic experts group.
	IMAGE_CODEC_BMP,	//Bitmap.
	IMAGE_CODEC_TGA,	//Truevision TGA.

	IMAGE_CODEC_MAX,
} Image_codec;

typedef enum
{
	PACKET_TYPE_INVALID = -1,

	PACKET_TYPE_UNKNOWN,	//This packet contains unknown data.
	PACKET_TYPE_AUDIO,		//This packet contains audio data.
	PACKET_TYPE_VIDEO,		//This packet contains video data.
	PACKET_TYPE_SUBTITLE,	//This packet contains subtitle data.

	PACKET_TYPE_MAX,
} Packet_type;

typedef enum
{
	SEEK_FLAG_NONE		= 0,		//No seek flag.
	SEEK_FLAG_BACKWARD	= (1 << 1),	//Seek backward.
	SEEK_FLAG_BYTE		= (1 << 2),	//Seek to given byte offset instead of time.
	SEEK_FLAG_ANY		= (1 << 3),	//Seek to any location including non key frame.
	SEEK_FLAG_FRAME		= (1 << 4),	//Seek to given frame number instead of time.
} Seek_flag;

typedef enum
{
	THREAD_TYPE_INVALID = -1,

	THREAD_TYPE_NONE,	//No multi-threading, using single thread.
	THREAD_TYPE_FRAME,	//Frame level multi-threading.
	THREAD_TYPE_SLICE,	//Slice level multi-threading.
	THREAD_TYPE_AUTO,	//Auto (only used when request multi-threading mode).

	THREAD_TYPE_MAX,
} Multi_thread_type;

typedef struct
{
	uint32_t bitrate;				//(out) Audio bitrate in Bps.
	uint32_t sample_rate;			//(out) Audio smaple rate in Hz.
	uint8_t ch;						//(out) Number of audio channels.
	double duration;				//(out) Audio track duration in seconds.
	char format_name[96];			//(out) Audio codec name.
	char short_format_name[16];		//(out) Audio short codec name.
	char track_lang[16];			//(out) Audio track language.
	Sample_format sample_format;	//(out) Audio sample format.
} Audio_info;

typedef struct
{
	uint32_t width;					//(out) Video width.
	uint32_t height;				//(out) Video height.
	uint32_t codec_width;			//(out) Video codec width (actual image width).
	uint32_t codec_height;			//(out) Video codec height (actual image height).
	double framerate;				//(out) Video framerate.
	double duration;				//(out) Video track duration in seconds.
	double sar_width;//1			//(out) Sample aspect ratio for width.
	double sar_height;//1			//(out) Sample aspect ratio for height.
	char format_name[96];			//(out) Video codec name.
	char short_format_name[16];		//(out) Video short codec name.
	Multi_thread_type thread_type;	//(out) Threading mode.
	Pixel_format pixel_format;		//(out) Video pixel format.
} Video_info;

typedef struct
{
	char format_name[96];			//(out) Subtitle codec name.
	char short_format_name[16];		//(out) Subtitle short codec name.
	char track_lang[16];			//(out) Subtitle track language.
} Subtitle_info;

typedef struct
{
	uint8_t* bitmap;			//(out) Subtitle bitmap, this may NULL.
	uint32_t bitmap_x;			//(out) X (horizontal) position, this field will be set if bitmap is not NULL.
	uint32_t bitmap_y;			//(out) Y (vertical) position, this field will be set if bitmap is not NULL.
	uint32_t bitmap_width;		//(out) Bitmap width, this field will be set if bitmap is not NULL.
	uint32_t bitmap_height;		//(out) Bitmap height, this field will be set if bitmap is not NULL.
	double start_time;			//(out) Start time in ms for this subtitle data. subtitle should be displayed if (start_time <= current_time <= end_time).
	double end_time;			//(out) End time in ms for this subtitle data. subtitle should be displayed if (start_time <= current_time <= end_time).
	char* text;					//(out) Subtitle text.
} Subtitle_data;

#endif //!defined(DEF_MEDIA_TYPES_H)
