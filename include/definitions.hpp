#if !defined(DEF_DEFINITIONS_HPP)
#define DEF_DEFINITIONS_HPP
#include <stdbool.h>
#include <stdint.h>

//settings
#define DEF_MAIN_DIR				/*(const char*)(*/"/GUI_sample/"/*)*/
#define DEF_UPDATE_DIR_PREFIX		/*(const char*)(*/"/3ds/GUI_sample_ver_"/*)*/
#define DEF_UPDATE_FILE_PREFIX		/*(const char*)(*/"GUI"/*)*/
#define DEF_CHECK_INTERNET_URL		/*(const char*)(*/"http://connectivitycheck.gstatic.com/generate_204"/*)*/
#define DEF_SEND_APP_INFO_URL		/*(const char*)(*/"https://script.google.com/macros/s/AKfycbyn_blFyKWXCgJr6NIF8x6ETs7CHRN5FXKYEAAIrzV6jPYcCkI/exec"/*)*/
#define DEF_CHECK_UPDATE_URL		/*(const char*)(*/"https://script.google.com/macros/s/AKfycbwvEedP97o8vgfpAG6EzcW6jxZZqFfZaMaqE1V7kCdp9BfuXySfRQ4own5CcFW1JxRBBQ/exec"/*)*/
#define DEF_HTTP_USER_AGENT			/*(const char*)(*/"gui sample " DEF_CURRENT_APP_VER/*)*/
#define DEF_CURRENT_APP_VER			/*(const char*)(*/"v0.0.1"/*)*/
#define DEF_CURRENT_APP_VER_INT		(uint32_t)(1)
#define DEF_ENABLE_SUB_APP0
#define DEF_ENABLE_SUB_APP1
#define DEF_ENABLE_SUB_APP2
#define DEF_ENABLE_SUB_APP3
#define DEF_ENABLE_SUB_APP4
#define DEF_ENABLE_SUB_APP5
#define DEF_ENABLE_SUB_APP6
#define DEF_ENABLE_SUB_APP7
#define DEF_DRAW_MAX_NUM_OF_SPRITE_SHEETS	(uint32_t)(128)
#define DEF_DECODER_MAX_AUDIO_TRACKS		(uint8_t)(8)
#define DEF_DECODER_MAX_VIDEO_TRACKS		(uint8_t)(2)
#define DEF_DECODER_MAX_SUBTITLE_TRACKS		(uint8_t)(8)
#define DEF_DECODER_MAX_SESSIONS			(uint8_t)(2)
#define DEF_DECODER_MAX_CACHE_PACKETS		(uint16_t)(256)
#define DEF_DECODER_MAX_RAW_IMAGE			(uint16_t)(128)
#define DEF_ENCODER_MAX_SESSIONS			(uint8_t)(2)
#define DEF_EXPL_MAX_FILES					(uint32_t)(1024)
#define DEF_HTTP_POST_BUFFER_SIZE			(uint32_t)(0x80000)
#define DEF_SOCKET_BUFFER_SIZE				(uint32_t)(0x40000)
#define DEF_LOG_BUFFER_LINES				(uint32_t)(512)
#define DEF_LOG_COLOR						(uint32_t)(0xFF00BB60)
#define DEF_MUXER_MAX_SESSIONS				(uint8_t)(2)
#define DEF_SPEAKER_MAX_BUFFERS				(uint32_t)(192)
#define DEF_SWKBD_MAX_DIC_WORDS				(uint16_t)(128)

#define DEF_DECODER_USE_DMA					/*(bool)(*/1/*)*/	//Enable DMA in video decoder module for faster processing.
#define DEF_DRAW_USE_DMA					/*(bool)(*/1/*)*/	//Enable DMA in draw module for faster processing.

#define DEF_ENABLE_CAM_API							/*(bool)(*/1/*)*/	//Enable camera API. You can still use libctru functions if you disable this API.
#define DEF_ENABLE_HW_CONVERTER_API					/*(bool)(*/1/*)*/	//Enable hardware color converter API. You can still use libctru functions if you disable this API.
#define DEF_ENABLE_SW_ASM_CONVERTER_API				/*(bool)(*/1/*)*/	//Enable assembly optimized software color converter API.
#define DEF_ENABLE_SW_CONVERTER_API					/*(bool)(*/1/*)*/	//Enable software color converter API.
#define DEF_ENABLE_SW_FFMPEG_COLOR_CONVERTER_API	/*(bool)(*/1/*)*/	//Enable software color converter API. This will use ffmpeg functions.
#define DEF_ENABLE_SW_FFMPEG_AUDIO_CONVERTER_API	/*(bool)(*/1/*)*/	//Enable software audio samples converter API. This will use ffmpeg functions.
#define DEF_ENABLE_CPU_MONITOR_API					/*(bool)(*/1/*)*/	//Enable CPU usage monitor API.
#define DEF_ENABLE_CURL_API							/*(bool)(*/1/*)*/	//Enable curl API. This will use curl functions. This API supports TLS 1.2.
#define DEF_ENABLE_VIDEO_AUDIO_DECODER_API			/*(bool)(*/1/*)*/	//Enable video/audio decoder API. This will use ffmpeg functions.
#define DEF_ENABLE_IMAGE_DECODER_API				/*(bool)(*/1/*)*/	//Enable image decoder API. This will use stb_image functions.
#define DEF_ENABLE_VIDEO_AUDIO_ENCODER_API			/*(bool)(*/1/*)*/	//Enable video/audio encoder API. This will use ffmpeg functions.
#define DEF_ENABLE_IMAGE_ENCODER_API				/*(bool)(*/1/*)*/	//Enable image encoder API. This will use stb_image functions.
#define DEF_ENABLE_EXPL_API							/*(bool)(*/1/*)*/	//Enable file explorer API.
#define DEF_ENABLE_HTTPC_API						/*(bool)(*/1/*)*/	//Enable httpc API. This API only supports up to TLS 1.1. You can still use libctru functions if you disable this API.
#define DEF_ENABLE_MIC_API							/*(bool)(*/1/*)*/	//Enable mic API. You can still use libctru functions if you disable this API.
#define DEF_ENABLE_MUXER_API						/*(bool)(*/1/*)*/	//Enable muxer API. This will uses ffmpeg functions.
#define DEF_ENABLE_SPEAKER_API						/*(bool)(*/1/*)*/	//Enable speaker API. You can still use libctru functions if you disable this API.
#define DEF_ENABLE_SWKBD_API						/*(bool)(*/1/*)*/	//Enable software keyboard API. You can still use libctru functions if you disable this API.

//sample app 0
#define DEF_SAPP0_NUM_OF_MSG	(uint16_t)(1)
//#define DEF_SAPP0_ENABLE_ICON
#define DEF_SAPP0_ENABLE_NAME
#define DEF_SAPP0_ICON_PATH		/*(const char*)(*/"romfs:/"/*)*/
#define DEF_SAPP0_NAME			/*(const char*)(*/"draw image\nsample"/*)*/
#define DEF_SAPP0_VER			/*(const char*)(*/"v0.0.1"/*)*/

//sample app 1
#define DEF_SAPP1_NUM_OF_MSG	(uint16_t)(1)
//#define DEF_SAPP1_ENABLE_ICON
#define DEF_SAPP1_ENABLE_NAME
#define DEF_SAPP1_ICON_PATH		/*(const char*)(*/"romfs:/"/*)*/
#define DEF_SAPP1_NAME			/*(const char*)(*/"file explorer\nsample"/*)*/
#define DEF_SAPP1_VER			/*(const char*)(*/"v0.0.1"/*)*/

//sample app 2
#define DEF_SAPP2_NUM_OF_MSG	(uint16_t)(1)
//#define DEF_SAPP2_ENABLE_ICON
#define DEF_SAPP2_ENABLE_NAME
#define DEF_SAPP2_ICON_PATH		/*(const char*)(*/"romfs:/"/*)*/
#define DEF_SAPP2_NAME			/*(const char*)(*/"hardware\nsettings\nsample"/*)*/
#define DEF_SAPP2_VER			/*(const char*)(*/"v0.0.1"/*)*/

//sample app 3
#define DEF_SAPP3_NUM_OF_MSG	(uint16_t)(1)
//#define DEF_SAPP3_ENABLE_ICON
#define DEF_SAPP3_ENABLE_NAME
#define DEF_SAPP3_ICON_PATH		/*(const char*)(*/"romfs:/"/*)*/
#define DEF_SAPP3_NAME			/*(const char*)(*/"Camera\nand mic\nsample"/*)*/
#define DEF_SAPP3_VER			/*(const char*)(*/"v0.0.1"/*)*/

//sample app 4
#define DEF_SAPP4_NUM_OF_MSG	(uint16_t)(1)
//#define DEF_SAPP4_ENABLE_ICON
#define DEF_SAPP4_ENABLE_NAME
#define DEF_SAPP4_ICON_PATH		/*(const char*)(*/"romfs:/"/*)*/
#define DEF_SAPP4_NAME			/*(const char*)(*/"Speaker\nsample"/*)*/
#define DEF_SAPP4_VER			/*(const char*)(*/"v0.0.1"/*)*/

//sample app 5
#define DEF_SAPP5_NUM_OF_MSG	(uint16_t)(1)
//#define DEF_SAPP5_ENABLE_ICON
#define DEF_SAPP5_ENABLE_NAME
#define DEF_SAPP5_ICON_PATH		/*(const char*)(*/"romfs:/"/*)*/
#define DEF_SAPP5_NAME			/*(const char*)(*/"sample 5"/*)*/
#define DEF_SAPP5_VER			/*(const char*)(*/"v0.0.1"/*)*/

//sample app 6
#define DEF_SAPP6_NUM_OF_MSG	(uint16_t)(1)
//#define DEF_SAPP6_ENABLE_ICON
#define DEF_SAPP6_ENABLE_NAME
#define DEF_SAPP6_ICON_PATH		/*(const char*)(*/"romfs:/"/*)*/
#define DEF_SAPP6_NAME			/*(const char*)(*/"sample 6"/*)*/
#define DEF_SAPP6_VER			/*(const char*)(*/"v0.0.1"/*)*/

//sample app 7
#define DEF_SAPP7_NUM_OF_MSG	(uint16_t)(1)
//#define DEF_SAPP7_ENABLE_ICON
#define DEF_SAPP7_ENABLE_NAME
#define DEF_SAPP7_ICON_PATH		/*(const char*)(*/"romfs:/"/*)*/
#define DEF_SAPP7_NAME			/*(const char*)(*/"sample 7"/*)*/
#define DEF_SAPP7_VER			/*(const char*)(*/"v0.0.1"/*)*/

#endif //!defined(DEF_DEFINITIONS_HPP)
