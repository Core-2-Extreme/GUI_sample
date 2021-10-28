#pragma once

/**
 * @brief Initialize a camera.
 * @param color_format (in) Output color format DEF_CAM_OUT_YUV422 or DEF_CAM_OUT_RGB565.
 * @return On success DEF_SUCCESS, 
 * on failure DEF_ERR_INVALID_ARG, DEF_ERR_ALREADY_INITIALIZED or Nintendo API's error will be returned.
 * @warning Thread dangerous
*/
Result_with_string Util_cam_init(int color_format);

/**
 * @brief Take a picture.
 * @param raw_data (out) Pointer for picture data, the pointer will be allocated inside of function.
 * @param width (out) Picture width.
 * @param height (out) Picture height.
 * @param shutter_sound (in) When true, shutter sound will be played after taking a picture.
 * @return On success DEF_SUCCESS, 
 * on failure DEF_ERR_NOT_INITIALIZED, DEF_ERR_OUT_OF_MEMORY or Nintendo API's error will be returned.
 * @warning Thread dangerous
*/
Result_with_string Util_cam_take_a_picture(u8** raw_data, int* width, int* height, bool shutter_sound);

/**
 * @brief Set picture resolution.
 * Available resolutions are : 640*480, 512*384, 400*240, 352*288, 320*240, 256*192, 176*144 and 160*120
 * @param width (in) Picture width.
 * @param height (in) Picture height.
 * @return On success DEF_SUCCESS, 
 * on failure DEF_ERR_NOT_INITIALIZED, DEF_ERR_INVALID_ARG or Nintendo API's error will be returned.
 * @warning Thread dangerous
*/
Result_with_string Util_cam_set_resolution(int width, int height);

/**
 * @brief Uninitialize a camera.
 * @warning Thread dangerous
*/
void Util_cam_exit(void);
