#if !defined(DEF_CONVERTER_HPP)
#define DEF_CONVERTER_HPP
#include <stdbool.h>
#include <stdint.h>
#include "system/util/converter_types.h"

#if DEF_ENABLE_SW_FFMPEG_COLOR_CONVERTER_API

/**
 * @brief Convert color format and/or size.
 * @param paraeters (in) Pointer for parameters. (See types.hpp for explanation).
 * @return On success DEF_SUCCESS, on failure DEF_ERR_*.
 * @note Thread safe.
*/
uint32_t Util_converter_convert_color(Color_converter_parameters* paraeters);

#else

#define Util_converter_convert_color(...) DEF_ERR_DISABLED

#endif //DEF_ENABLE_SW_FFMPEG_COLOR_CONVERTER_API

#if DEF_ENABLE_SW_FFMPEG_AUDIO_CONVERTER_API

/**
 * @brief Convert audio format and/or sample rate.
 * @param paraeters (in) Pointer for parameters. (See types.hpp for explanation).
 * @return On success DEF_SUCCESS, on failure DEF_ERR_*.
 * @note Thread safe.
*/
uint32_t Util_converter_convert_audio(Audio_converter_parameters* parameters);

#else

#define Util_converter_convert_audio(...) DEF_ERR_DISABLED

#endif //DEF_ENABLE_SW_FFMPEG_AUDIO_CONVERTER_API

#if DEF_ENABLE_SW_CONVERTER_API

/**
 * @brief Convert YUV420P to RGB565LE.
 * @param yuv420p (in) Pointer for yuv420p data.
 * @param rgb565 (out) Pointer for rgb565 data, the pointer will be allocated inside of function.
 * @param width (in) Picture width.
 * @param height (in) Picture height.
 * @return On success DEF_SUCCESS, on failure DEF_ERR_*.
 * @note Thread safe.
*/
uint32_t Util_converter_yuv420p_to_rgb565le(uint8_t* yuv420p, uint8_t** rgb565, uint32_t width, uint32_t height);

/**
 * @brief Convert YUV420P to RGB888LE.
 * @param yuv420p (in) Pointer for yuv420p data.
 * @param rgb888 (out) Pointer for rgb888 data, the pointer will be allocated inside of function.
 * @param width (in) Picture width.
 * @param height (in) Picture height.
 * @return On success DEF_SUCCESS, on failure DEF_ERR_*.
 * @note Thread safe.
*/
uint32_t Util_converter_yuv420p_to_rgb888le(uint8_t* yuv420p, uint8_t** rgb888, uint32_t width, uint32_t height);

/**
 * @brief Convert RGB8888BE to RGB8888LE.
 * @param rgb8888 (in&out) Pointer for rgb888 data.
 * @param width (in) Picture width.
 * @param height (in) Picture height.
 * @return On success DEF_SUCCESS, on failure DEF_ERR_*.
 * @note Thread safe.
*/
uint32_t Util_converter_rgba8888be_to_rgba8888le(uint8_t* rgba8888, uint32_t width, uint32_t height);

/**
 * @brief Convert RGB888BE to RGB888LE.
 * @param rgb888 (in&out) Pointer for rgb888 data.
 * @param width (in) Picture width.
 * @param height (in) Picture height.
 * @return On success DEF_SUCCESS, on failure DEF_ERR_*.
 * @note Thread safe
*/
uint32_t Util_converter_rgb888be_to_rgb888le(uint8_t* rgb888, uint32_t width, uint32_t height);

/**
 * @brief Rotate (any) RGB888 90 degree.
 * @param rgb888 (in) Pointer for rgb888 data.
 * @param rotated_rgb888 (out) Pointer for rotated_rgb888 data, the pointer will be allocated inside of function.
 * @param width (in) Picture width.
 * @param height (in) Picture height.
 * @param rotated_width (out) new(after rotated) picture width.
 * @param rotated_height (out) new(after rotated) picture height.
 * @return On success DEF_SUCCESS, on failure DEF_ERR_*.
 * @note Thread safe
*/
uint32_t Util_converter_rgb888_rotate_90_degree(uint8_t* rgb888, uint8_t** rotated_rgb888, uint32_t width, uint32_t height, uint32_t* rotated_width, uint32_t* rotated_height);

#else

#define Util_converter_yuv422_to_rgb565le(...) DEF_ERR_DISABLED
#define Util_converter_yuv422_to_yuv420p(...) DEF_ERR_DISABLED
#define Util_converter_yuv420p_to_rgb565le(...) DEF_ERR_DISABLED
#define Util_converter_yuv420p_to_rgb888le(...) DEF_ERR_DISABLED
#define Util_converter_rgba8888be_to_rgba8888le(...) DEF_ERR_DISABLED
#define Util_converter_rgb888be_to_rgb888le(...) DEF_ERR_DISABLED
#define Util_converter_rgb888_rotate_90_degree(...) DEF_ERR_DISABLED
#define Util_converter_rgb888le_to_yuv420p(...) DEF_ERR_DISABLED
#define Util_converter_rgb565le_to_rgb888le(...) DEF_ERR_DISABLED

#endif //DEF_ENABLE_SW_CONVERTER_API

#if DEF_ENABLE_SW_ASM_CONVERTER_API

/**
 * @brief Convert YUV420P to RGB888LE (assembly optimized).
 * @param yuv420p (in) Pointer for yuv420p data.
 * @param rgb888 (out) Pointer for rgb888 data, the pointer will be allocated inside of function.
 * @param width (in) Picture width.
 * @param height (in) Picture height.
 * @return On success DEF_SUCCESS, on failure DEF_ERR_*.
 * @note Thread safe
*/
uint32_t Util_converter_yuv420p_to_rgb888le_asm(uint8_t* yuv420p, uint8_t** rgb888, uint32_t width, uint32_t height);

/**
 * @brief Convert YUV420P to RGB565LE (assembly optimized).
 * @param yuv420p (in) Pointer for yuv420p data.
 * @param rgb565 (out) Pointer for rgb565 data, the pointer will be allocated inside of function.
 * @param width (in) Picture width.
 * @param height (in) Picture height.
 * @return On success DEF_SUCCESS, on failure DEF_ERR_*.
 * @note Thread safe
*/
uint32_t Util_converter_yuv420p_to_rgb565le_asm(uint8_t* yuv420p, uint8_t** rgb565, uint32_t width, uint32_t height);

#else

#define Util_converter_yuv420p_to_rgb888le_asm(...) DEF_ERR_DISABLED
#define Util_converter_yuv420p_to_rgb565le_asm(...) DEF_ERR_DISABLED

#endif //DEF_ENABLE_SW_ASM_CONVERTER_API

#if DEF_ENABLE_HW_CONVERTER_API

/**
 * @brief Initialize a y2r(hardware color converter).
 * @return On success DEF_SUCCESS, on failure DEF_ERR_* or Nintendo API's error.
 * @warning Thread dangerous (untested)
*/
uint32_t Util_converter_y2r_init(void);

/**
 * @brief Convert YUV420P to RGB565LE (hardware conversion).
 * @param yuv420p (in) Pointer for yuv420p data.
 * @param rgb565 (out) Pointer for rgb565 data, the pointer will be allocated inside of function.
 * @param width (in) Picture width.
 * @param height (in) Picture height.
 * @param texture_format (in) When true, rgb565 data will be outputted as texture format that
 * can be used for Draw_set_texture_data_direct().
 * @return On success DEF_SUCCESS, on failure DEF_ERR_* or Nintendo API's error.
 * @warning Thread dangerous (untested)
*/
uint32_t Util_converter_y2r_yuv420p_to_rgb565le(uint8_t* yuv420p, uint8_t** rgb565, uint16_t width, uint16_t height, bool texture_format);

/**
 * @brief Uninitialize a y2r(hardware color converter).
 * Do nothing if y2r api is not initialized.
 * @warning Thread dangerous (untested)
*/
void Util_converter_y2r_exit(void);

#else

#define Util_converter_y2r_init() DEF_ERR_DISABLED
#define Util_converter_y2r_yuv420p_to_rgb565le(...) DEF_ERR_DISABLED
#define Util_converter_y2r_exit()

#endif //DEF_ENABLE_HW_CONVERTER_API

#endif //!defined(DEF_CONVERTER_HPP)
