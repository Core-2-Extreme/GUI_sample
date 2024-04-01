#ifndef ERROR_H_
#define ERROR_H_
#include <stdbool.h>
#include <stdint.h>

#define DEF_SUCCESS								(uint32_t)(0x00000000)
#define DEF_ERR_OTHER							(uint32_t)(0xFFFFFFFF)
#define DEF_ERR_OUT_OF_MEMORY					(uint32_t)(0xFFFFFFFE)
#define DEF_ERR_OUT_OF_LINEAR_MEMORY			(uint32_t)(0xFFFFFFFD)
#define DEF_ERR_GAS_RETURNED_NOT_SUCCESS		(uint32_t)(0xFFFFFFFC)
#define DEF_ERR_STB_IMG_RETURNED_NOT_SUCCESS	(uint32_t)(0xFFFFFFFB)
#define DEF_ERR_FFMPEG_RETURNED_NOT_SUCCESS		(uint32_t)(0xFFFFFFFA)
#define DEF_ERR_INVALID_ARG						(uint32_t)(0xFFFFFFF9)
#define DEF_ERR_JSMN_RETURNED_NOT_SUCCESS		(uint32_t)(0xFFFFFFF8)
#define DEF_ERR_TRY_AGAIN						(uint32_t)(0xFFFFFFF7)
#define DEF_ERR_ALREADY_INITIALIZED				(uint32_t)(0xFFFFFFF6)
#define DEF_ERR_NOT_INITIALIZED					(uint32_t)(0xFFFFFFF5)
#define DEF_ERR_CURL_RETURNED_NOT_SUCCESS		(uint32_t)(0xFFFFFFF4)
#define DEF_ERR_NEED_MORE_INPUT					(uint32_t)(0xFFFFFFF3)
//This is different from DEF_ERR_DECODER_TRY_AGAIN, No video output was made at this call, try again without calling Util_decoder_ready_video_packet().
#define DEF_ERR_DECODER_TRY_AGAIN_NO_OUTPUT		(uint32_t)(0xFFFFFFF2)
//This is different from DEF_ERR_DECODER_TRY_AGAIN_NO_OUTPUT, Video output was made at this call, try again without calling Util_decoder_ready_video_packet().
#define DEF_ERR_DECODER_TRY_AGAIN				(uint32_t)(0xFFFFFFF1)
#define DEF_ERR_DISABLED						(uint32_t)(0xFFFFFFF0)

// #define DEF_ERR_OTHER_STR (std::string)"[Error] Something went wrong. "
// #define DEF_ERR_OUT_OF_MEMORY_STR (std::string)"[Error] Out of memory. "
// #define DEF_ERR_OUT_OF_LINEAR_MEMORY_STR (std::string)"[Error] Out of linear memory. "
// #define DEF_ERR_GAS_RETURNED_NOT_SUCCESS_STR (std::string)"[Error] Google apps script returned NOT success. "
// #define DEF_ERR_STB_IMG_RETURNED_NOT_SUCCESS_STR (std::string)"[Error] stb image returned NOT success. "
// #define DEF_ERR_FFMPEG_RETURNED_NOT_SUCCESS_STR (std::string)"[Error] ffmpeg returned NOT success. "
// #define DEF_ERR_INVALID_ARG_STR (std::string)"[Error] Invalid arg. "
// #define DEF_ERR_JSMN_RETURNED_NOT_SUCCESS_STR (std::string)"[Error] jsmn returned NOT success. "
// #define DEF_ERR_TRY_AGAIN_STR (std::string)"[Error] Try again later. "
// #define DEF_ERR_ALREADY_INITIALIZED_STR (std::string)"[Error] Already initialized. "
// #define DEF_ERR_NOT_INITIALIZED_STR (std::string)"[Error] Not initialized. "
// #define DEF_ERR_NINTENDO_RETURNED_NOT_SUCCESS_STR (std::string)"[Error] Nintendo api returned NOT success. "
// #define DEF_ERR_CURL_RETURNED_NOT_SUCCESS_STR (std::string)"[Error] curl returned NOT success. "
// #define DEF_ERR_NEED_MORE_INPUT_STR (std::string)"[Error] Need more input to produce the output. "
// #define DEF_ERR_DECODER_TRY_AGAIN_NO_OUTPUT_STR (std::string)"[Error] Try again (video output was made). "
// #define DEF_ERR_DECODER_TRY_AGAIN_STR (std::string)"[Error] Try again. "
// #define DEF_ERR_DISABLED_STR (std::string)"[Error] This function is disabled. "


#endif //ERROR_H_
