#if !defined(DEF_CAMERA_HPP)
#define DEF_CAMERA_HPP

extern "C"
{
#include <stdbool.h>
#include <stdint.h>

typedef enum
{
	CAM_RES_INVALID = -1,

	CAM_RES_640x480,
	CAM_RES_512x384,
	CAM_RES_400x240,
	CAM_RES_352x288,
	CAM_RES_320x240,
	CAM_RES_256x192,
	CAM_RES_176x144,
	CAM_RES_160x120,

	CAM_RES_MAX,
} Camera_resolution;

typedef enum
{
	CAM_FPS_INVALID = -1,

	CAM_FPS_15,
	CAM_FPS_15_TO_5,
	CAM_FPS_15_TO_2,
	CAM_FPS_10,
	CAM_FPS_8_5,
	CAM_FPS_5,
	CAM_FPS_20,
	CAM_FPS_20_TO_5,
	CAM_FPS_30,
	CAM_FPS_30_TO_5,
	CAM_FPS_15_TO_10,
	CAM_FPS_20_TO_10,
	CAM_FPS_30_TO_10,

	CAM_FPS_MAX,
} Camera_framerate;

typedef enum
{
	CAM_LENS_CORRECTION_INVALID = -1,

	CAM_LENS_CORRECTION_OFF,
	CAM_LENS_CORRECTION_70,
	CAM_LENS_CORRECTION_90,

	CAM_LENS_CORRECTION_MAX,
} Camera_lens_correction;

typedef enum
{
	CAM_PORT_INVALID = -1,

	CAM_PORT_OUT_LEFT,
	CAM_PORT_OUT_RIGHT,
	CAM_PORT_IN,

	CAM_PORT_MAX,
} Camera_port;

typedef enum
{
	CAM_CONTRAST_INVALID = -1,

	CAM_CONTRAST_01,
	CAM_CONTRAST_02,
	CAM_CONTRAST_03,
	CAM_CONTRAST_04,
	CAM_CONTRAST_05,
	CAM_CONTRAST_06,
	CAM_CONTRAST_07,
	CAM_CONTRAST_08,
	CAM_CONTRAST_09,
	CAM_CONTRAST_10,
	CAM_CONTRAST_11,

	CAM_CONTRAST_MAX,
} Camera_contrast;

typedef enum
{
	CAM_WHITE_BALANCE_INVALID = -1,

	CAM_WHITE_BALANCE_AUTO,
	CAM_WHITE_BALANCE_3200K,
	CAM_WHITE_BALANCE_4150K,
	CAM_WHITE_BALANCE_5200K,
	CAM_WHITE_BALANCE_6000K,
	CAM_WHITE_BALANCE_7000K,

	CAM_WHITE_BALANCE_MAX,
} Camera_white_balance;

typedef enum
{
	CAM_EXPOSURE_INVALID = -1,

	CAM_EXPOSURE_0,
	CAM_EXPOSURE_1,
	CAM_EXPOSURE_2,
	CAM_EXPOSURE_3,
	CAM_EXPOSURE_4,
	CAM_EXPOSURE_5,

	CAM_EXPOSURE_MAX,
} Camera_exposure;

#if defined(DEF_ENABLE_CAM_API)

/**
 * @brief Initialize a camera.
 * @param color_format (in) Output color format (PIXEL_FORMAT_RGB565LE or PIXEL_FORMAT_YUV422P).
 * @return On success DEF_SUCCESS, on failure DEF_ERR_* or Nintendo API's error.
 * @warning Thread dangerous (untested)
*/
uint32_t Util_cam_init(Pixel_format color_format);

/**
 * @brief Take a picture.
 * @param raw_data (out) Pointer for picture data, the pointer will be allocated inside of function.
 * @param width (out) Picture width.
 * @param height (out) Picture height.
 * @param shutter_sound (in) When true, shutter sound will be played after taking a picture.
 * @return On success DEF_SUCCESS, on failure DEF_ERR_* or Nintendo API's error.
 * @warning Thread dangerous (untested)
*/
uint32_t Util_cam_take_a_picture(uint8_t** raw_data, uint16_t* width, uint16_t* height, bool shutter_sound);

/**
 * @brief Set picture resolution.
 * @param resolution_mode (in) Resolution mode.
 * @return On success DEF_SUCCESS, on failure DEF_ERR_* or Nintendo API's error.
 * @warning Thread dangerous (untested)
*/
uint32_t Util_cam_set_resolution(Camera_resolution resolution_mode);

/**
 * @brief Set framerate.
 * @param fps_mode (in) Framerate mode.
 * @return On success DEF_SUCCESS, on failure DEF_ERR_* or Nintendo API's error.
 * @warning Thread dangerous (untested)
*/
uint32_t Util_cam_set_fps(Camera_framerate fps_mode);

/**
 * @brief Set contrast.
 * @param contrast_mode (in) Contrast mode.
 * @return On success DEF_SUCCESS, on failure DEF_ERR_* or Nintendo API's error.
 * @warning Thread dangerous (untested)
*/
uint32_t Util_cam_set_contrast(Camera_contrast contrast_mode);

/**
 * @brief Set white balance.
 * @param white_balance_mode (in) White balance mode.
 * @return On success DEF_SUCCESS, on failure DEF_ERR_* or Nintendo API's error.
 * @warning Thread dangerous (untested)
*/
uint32_t Util_cam_set_white_balance(Camera_white_balance white_balance_mode);

/**
 * @brief Set lens correction.
 * @param lens_correction_mode (in) Lens correction mode.
 * @return On success DEF_SUCCESS, on failure DEF_ERR_* or Nintendo API's error.
 * @warning Thread dangerous (untested)
*/
uint32_t Util_cam_set_lens_correction(Camera_lens_correction lens_correction_mode);

/**
 * @brief Set camera port.
 * @param camera_mode (in) Camera port.
 * @return On success DEF_SUCCESS, on failure DEF_ERR_* or Nintendo API's error.
 * @warning Thread dangerous (untested)
*/
uint32_t Util_cam_set_camera(Camera_port camera_port);

/**
 * @brief Set exposure.
 * @param exposure_mode (in) Exposure mode.
 * @return On success DEF_SUCCESS, on failure DEF_ERR_* or Nintendo API's error.
 * @warning Thread dangerous (untested)
*/
uint32_t Util_cam_set_exposure(Camera_exposure exposure_mode);

/**
 * @brief Set noise filter.
 * @param noise_filter_mode (in) When true, noise filter will be turned on otherwise off.
 * @return On success DEF_SUCCESS, on failure DEF_ERR_* or Nintendo API's error.
 * @warning Thread dangerous (untested)
*/
uint32_t Util_cam_set_noise_filter(bool noise_filter_mode);

/**
 * @brief Uninitialize a camera.
 * Do nothing if camera api is not initialized.
 * @warning Thread dangerous (untested)
*/
void Util_cam_exit(void);

#else

#define Util_cam_init(...) DEF_ERR_DISABLED
#define Util_cam_take_a_picture(...) DEF_ERR_DISABLED
#define Util_cam_set_resolution(...) DEF_ERR_DISABLED
#define Util_cam_set_fps(...) DEF_ERR_DISABLED
#define Util_cam_set_contrast(...) DEF_ERR_DISABLED
#define Util_cam_set_white_balance(...) DEF_ERR_DISABLED
#define Util_cam_set_lens_correction(...) DEF_ERR_DISABLED
#define Util_cam_set_camera(...) DEF_ERR_DISABLED
#define Util_cam_set_exposure(...) DEF_ERR_DISABLED
#define Util_cam_set_noise_filter(...) DEF_ERR_DISABLED
#define Util_cam_exit()

#endif //defined(DEF_ENABLE_CAM_API)
}
#endif //!defined(DEF_CAMERA_HPP)
