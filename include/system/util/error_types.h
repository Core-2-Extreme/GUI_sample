#if !defined(DEF_ERROR_TYPES_H)
#define DEF_ERROR_TYPES_H
#include <stdbool.h>
#include <stdint.h>

#define DEF_SUCCESS									(uint32_t)(0x00000000)	//Success.
#define DEF_ERR_OTHER								(uint32_t)(0xFFFFFFFF)	//Something went wrong.
#define DEF_ERR_OUT_OF_MEMORY						(uint32_t)(0xFFFFFFFE)	//Out of memory.
#define DEF_ERR_OUT_OF_LINEAR_MEMORY				(uint32_t)(0xFFFFFFFD)	//Out of memory.
#define DEF_ERR_GAS_RETURNED_NOT_SUCCESS			(uint32_t)(0xFFFFFFFC)	//GAS API returned non-success result.
#define DEF_ERR_STB_IMG_RETURNED_NOT_SUCCESS		(uint32_t)(0xFFFFFFFB)	//Stbi API returned non-success result.
#define DEF_ERR_FFMPEG_RETURNED_NOT_SUCCESS			(uint32_t)(0xFFFFFFFA)	//FFmpeg API returned non-success result.
#define DEF_ERR_INVALID_ARG							(uint32_t)(0xFFFFFFF9)	//Invalid args were provided.
#define DEF_ERR_JSMN_RETURNED_NOT_SUCCESS			(uint32_t)(0xFFFFFFF8)	//JSMN API returned non-success result.
#define DEF_ERR_TRY_AGAIN							(uint32_t)(0xFFFFFFF7)	//Try again later.
#define DEF_ERR_ALREADY_INITIALIZED					(uint32_t)(0xFFFFFFF6)	//Already initialized.
#define DEF_ERR_NOT_INITIALIZED						(uint32_t)(0xFFFFFFF5)	//Not initialized.
#define DEF_ERR_CURL_RETURNED_NOT_SUCCESS			(uint32_t)(0xFFFFFFF4)	//CURL API returned non-success result.
#define DEF_ERR_NEED_MORE_INPUT						(uint32_t)(0xFFFFFFF3)	//Need more data to produce an output.
//This is different from DEF_ERR_DECODER_TRY_AGAIN, No video output was made at this call, try again without calling Util_decoder_ready_video_packet().
#define DEF_ERR_DECODER_TRY_AGAIN_NO_OUTPUT			(uint32_t)(0xFFFFFFF2)
//This is different from DEF_ERR_DECODER_TRY_AGAIN_NO_OUTPUT, Video output was made at this call, try again without calling Util_decoder_ready_video_packet().
#define DEF_ERR_DECODER_TRY_AGAIN					(uint32_t)(0xFFFFFFF1)
#define DEF_ERR_DISABLED							(uint32_t)(0xFFFFFFF0)	//This feature has been disabled.
#define DEF_ERR_NO_RESULT_CODE						(uint32_t)(1234567890)	//No result codes available.

#define DEF_ERR_OTHER_STR							(const char*)"[Error] Something went wrong. "
#define DEF_ERR_OUT_OF_MEMORY_STR					(const char*)"[Error] Out of memory. "
#define DEF_ERR_OUT_OF_LINEAR_MEMORY_STR			(const char*)"[Error] Out of linear memory. "
#define DEF_ERR_GAS_RETURNED_NOT_SUCCESS_STR		(const char*)"[Error] Google apps script returned NOT success. "
#define DEF_ERR_STB_IMG_RETURNED_NOT_SUCCESS_STR	(const char*)"[Error] stb image returned NOT success. "
#define DEF_ERR_FFMPEG_RETURNED_NOT_SUCCESS_STR		(const char*)"[Error] ffmpeg returned NOT success. "
#define DEF_ERR_INVALID_ARG_STR						(const char*)"[Error] Invalid arg. "
#define DEF_ERR_JSMN_RETURNED_NOT_SUCCESS_STR		(const char*)"[Error] jsmn returned NOT success. "
#define DEF_ERR_TRY_AGAIN_STR						(const char*)"[Error] Try again later. "
#define DEF_ERR_ALREADY_INITIALIZED_STR				(const char*)"[Error] Already initialized. "
#define DEF_ERR_NOT_INITIALIZED_STR					(const char*)"[Error] Not initialized. "
#define DEF_ERR_NINTENDO_RETURNED_NOT_SUCCESS_STR	(const char*)"[Error] Nintendo api returned NOT success. "
#define DEF_ERR_CURL_RETURNED_NOT_SUCCESS_STR		(const char*)"[Error] curl returned NOT success. "
#define DEF_ERR_NEED_MORE_INPUT_STR					(const char*)"[Error] Need more input to produce the output. "
#define DEF_ERR_DECODER_TRY_AGAIN_NO_OUTPUT_STR		(const char*)"[Error] Try again (video output was made). "
#define DEF_ERR_DECODER_TRY_AGAIN_STR				(const char*)"[Error] Try again. "
#define DEF_ERR_DISABLED_STR						(const char*)"[Error] This function is disabled. "

#endif //!defined(DEF_ERROR_TYPES_H)
