#ifndef DEFINITIONS_HPP
#define DEFINITIONS_HPP

//settings
#define DEF_MAIN_DIR_C (const char*)"/GUI_sample/"
#define DEF_MAIN_DIR (std::string)"/GUI_sample/"
#define DEF_UPDATE_DIR_PREFIX (std::string)"/3ds/GUI_sample_ver_"
#define DEF_UPDATE_FILE_PREFIX (std::string)"GUI"
#define DEF_CHECK_INTERNET_URL (std::string)"http://connectivitycheck.gstatic.com/generate_204"
#define DEF_SEND_APP_INFO_URL (std::string)"https://script.google.com/macros/s/AKfycbyn_blFyKWXCgJr6NIF8x6ETs7CHRN5FXKYEAAIrzV6jPYcCkI/exec"
#define DEF_CHECK_UPDATE_URL (std::string)"https://script.google.com/macros/s/AKfycbwvEedP97o8vgfpAG6EzcW6jxZZqFfZaMaqE1V7kCdp9BfuXySfRQ4own5CcFW1JxRBBQ/exec"
#define DEF_HTTP_USER_AGENT (std::string)"gui sample " + DEF_CURRENT_APP_VER
#define DEF_CURRENT_APP_VER (std::string)"v0.0.1"
#define DEF_CURRENT_APP_VER_INT 1
#define DEF_ENABLE_SUB_APP0
#define DEF_ENABLE_SUB_APP1
#define DEF_ENABLE_SUB_APP2
#define DEF_ENABLE_SUB_APP3
#define DEF_ENABLE_SUB_APP4
#define DEF_ENABLE_SUB_APP5
#define DEF_ENABLE_SUB_APP6
#define DEF_ENABLE_SUB_APP7
#define DEF_DRAW_MAX_NUM_OF_SPRITE_SHEETS 128
#define DEF_DECODER_MAX_CACHE_PACKETS 256
#define DEF_DECODER_MAX_AUDIO_TRACKS 8
#define DEF_DECODER_MAX_VIDEO_TRACKS 2
#define DEF_DECODER_MAX_SUBTITLE_TRACKS 8
#define DEF_DECODER_MAX_RAW_IMAGE 128
#define DEF_DECODER_MAX_SESSIONS 2
#define DEF_ENCODER_MAX_SESSIONS 2
#define DEF_EXPL_MAX_FILES 1024
#define DEF_HTTP_POST_BUFFER_SIZE 0x80000
#define DEF_SOCKET_BUFFER_SIZE 0x40000
#define DEF_LOG_BUFFER_LINES 512
#define DEF_LOG_COLOR 0xFF00BB60
#define DEF_MUXER_MAX_SESSIONS 2
#define DEF_SPEAKER_MAX_BUFFERS 192
#define DEF_SWKBD_MAX_DIC_WORDS 128

#define DEF_DECODER_USE_DMA					1 //Enable DMA in video decoder module for faster processing.
#define DEF_DRAW_USE_DMA					1 //Enable DMA in draw module for faster processing.

#define DEF_ENABLE_CAM_API							1 //Enable camera API. You can still use libctru functions if you disable this API.
#define DEF_ENABLE_HW_CONVERTER_API					1 //Enable hardware color converter API. You can still use libctru functions if you disable this API.
#define DEF_ENABLE_SW_ASM_CONVERTER_API				1 //Enable assembly optimized software color converter API.
#define DEF_ENABLE_SW_CONVERTER_API					1 //Enable software color converter API.
#define DEF_ENABLE_SW_FFMPEG_COLOR_CONVERTER_API	1 //Enable software color converter API. This will use ffmpeg functions.
#define DEF_ENABLE_SW_FFMPEG_AUDIO_CONVERTER_API	1 //Enable software audio samples converter API. This will use ffmpeg functions.
#define DEF_ENABLE_CPU_MONITOR_API					1 //Enable CPU usage monitor API.
#define DEF_ENABLE_CURL_API							1 //Enable curl API. This will use curl functions. This API supports TLS 1.2.
#define DEF_ENABLE_VIDEO_AUDIO_DECODER_API			1 //Enable video/audio decoder API. This will use ffmpeg functions.
#define DEF_ENABLE_IMAGE_DECODER_API				1 //Enable image decoder API. This will use stb_image functions.
#define DEF_ENABLE_VIDEO_AUDIO_ENCODER_API			1 //Enable video/audio encoder API. This will use ffmpeg functions.
#define DEF_ENABLE_IMAGE_ENCODER_API				1 //Enable image encoder API. This will use stb_image functions.
#define DEF_ENABLE_EXPL_API							1 //Enable file explorer API.
#define DEF_ENABLE_HTTPC_API						1 //Enable httpc API. This API only supports up to TLS 1.1. You can still use libctru functions if you disable this API.
#define DEF_ENABLE_MIC_API							1 //Enable mic API. You can still use libctru functions if you disable this API.
#define DEF_ENABLE_MUXER_API						1 //Enable muxer API. This will uses ffmpeg functions.
#define DEF_ENABLE_SPEAKER_API						1 //Enable speaker API. You can still use libctru functions if you disable this API.
#define DEF_ENABLE_SWKBD_API						1 //Enable software keyboard API. You can still use libctru functions if you disable this API.

//sample app 0
#define DEF_SAPP0_NUM_OF_MSG 1
//#define DEF_SAPP0_ENABLE_ICON
#define DEF_SAPP0_ENABLE_NAME
#define DEF_SAPP0_ICON_PATH (std::string)"romfs:/"
#define DEF_SAPP0_NAME (std::string)"draw image\nsample"
#define DEF_SAPP0_VER (std::string)"v0.0.1"

//sample app 1
#define DEF_SAPP1_NUM_OF_MSG 1
//#define DEF_SAPP1_ENABLE_ICON
#define DEF_SAPP1_ENABLE_NAME
#define DEF_SAPP1_ICON_PATH (std::string)"romfs:/"
#define DEF_SAPP1_NAME (std::string)"file explorer\nsample"
#define DEF_SAPP1_VER (std::string)"v0.0.1"

//sample app 2
#define DEF_SAPP2_NUM_OF_MSG 1
//#define DEF_SAPP2_ENABLE_ICON
#define DEF_SAPP2_ENABLE_NAME
#define DEF_SAPP2_ICON_PATH (std::string)"romfs:/"
#define DEF_SAPP2_NAME (std::string)"hardware\nsettings\nsample"
#define DEF_SAPP2_VER (std::string)"v0.0.1"

//sample app 3
#define DEF_SAPP3_NUM_OF_MSG 1
//#define DEF_SAPP3_ENABLE_ICON
#define DEF_SAPP3_ENABLE_NAME
#define DEF_SAPP3_ICON_PATH (std::string)"romfs:/"
#define DEF_SAPP3_NAME (std::string)"Camera\nand mic\nsample"
#define DEF_SAPP3_VER (std::string)"v0.0.1"

//sample app 4
#define DEF_SAPP4_NUM_OF_MSG 1
//#define DEF_SAPP4_ENABLE_ICON
#define DEF_SAPP4_ENABLE_NAME
#define DEF_SAPP4_ICON_PATH (std::string)"romfs:/"
#define DEF_SAPP4_NAME (std::string)"Speaker\nsample"
#define DEF_SAPP4_VER (std::string)"v0.0.1"

//sample app 5
#define DEF_SAPP5_NUM_OF_MSG 1
//#define DEF_SAPP5_ENABLE_ICON
#define DEF_SAPP5_ENABLE_NAME
#define DEF_SAPP5_ICON_PATH (std::string)"romfs:/"
#define DEF_SAPP5_NAME (std::string)"sample 5"
#define DEF_SAPP5_VER (std::string)"v0.0.1"

//sample app 6
#define DEF_SAPP6_NUM_OF_MSG 1
//#define DEF_SAPP6_ENABLE_ICON
#define DEF_SAPP6_ENABLE_NAME
#define DEF_SAPP6_ICON_PATH (std::string)"romfs:/"
#define DEF_SAPP6_NAME (std::string)"sample 6"
#define DEF_SAPP6_VER (std::string)"v0.0.1"

//sample app 7
#define DEF_SAPP7_NUM_OF_MSG 1
//#define DEF_SAPP7_ENABLE_ICON
#define DEF_SAPP7_ENABLE_NAME
#define DEF_SAPP7_ICON_PATH (std::string)"romfs:/"
#define DEF_SAPP7_NAME (std::string)"sample 7"
#define DEF_SAPP7_VER (std::string)"v0.0.1"

#endif
