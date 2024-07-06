#if !defined(DEF_CAMERA_TYPES_H)
#define DEF_CAMERA_TYPES_H
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

#endif //!defined(DEF_CAMERA_TYPES_H)
