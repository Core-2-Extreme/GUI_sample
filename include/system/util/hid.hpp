#if !defined(HID_HPP)
#define HID_HPP

#include <stdbool.h>
#include <stdint.h>

#include "system/draw/draw.hpp"

extern "C"
{
typedef struct
{
	//Is button pressed.
	bool p_a = false;
	bool p_b = false;
	bool p_x = false;
	bool p_y = false;
	bool p_c_up = false;
	bool p_c_down = false;
	bool p_c_left = false;
	bool p_c_right = false;
	bool p_d_up = false;
	bool p_d_down = false;
	bool p_d_left = false;
	bool p_d_right = false;
	bool p_l = false;
	bool p_r = false;
	bool p_zl = false;
	bool p_zr = false;
	bool p_start = false;
	bool p_select = false;
	bool p_cs_up = false;
	bool p_cs_down = false;
	bool p_cs_left = false;
	bool p_cs_right = false;
	bool p_touch = false;
	bool p_any = false;
	//Is button held.
	bool h_a = false;
	bool h_b = false;
	bool h_x = false;
	bool h_y = false;
	bool h_c_up = false;
	bool h_c_down = false;
	bool h_c_left = false;
	bool h_c_right = false;
	bool h_d_up = false;
	bool h_d_down = false;
	bool h_d_left = false;
	bool h_d_right = false;
	bool h_l = false;
	bool h_r = false;
	bool h_zl = false;
	bool h_zr = false;
	bool h_start = false;
	bool h_select = false;
	bool h_cs_up = false;
	bool h_cs_down = false;
	bool h_cs_left = false;
	bool h_cs_right = false;
	bool h_touch = false;
	bool h_any = false;
	//Is button released.
	bool r_a = false;
	bool r_b = false;
	bool r_x = false;
	bool r_y = false;
	bool r_c_up = false;
	bool r_c_down = false;
	bool r_c_left = false;
	bool r_c_right = false;
	bool r_d_up = false;
	bool r_d_down = false;
	bool r_d_left = false;
	bool r_d_right = false;
	bool r_l = false;
	bool r_r = false;
	bool r_zl = false;
	bool r_zr = false;
	bool r_start = false;
	bool r_select = false;
	bool r_cs_up = false;
	bool r_cs_down = false;
	bool r_cs_left = false;
	bool r_cs_right = false;
	bool r_touch = false;
	bool r_any = false;
	//CPAD and touch position.
	int cpad_x = 0;
	int cpad_y = 0;
	int touch_x = 0;
	int touch_y = 0;
	int touch_x_move = 0;
	int touch_y_move = 0;
	int held_time = 0;
	//Timestamp for this data.
	uint64_t ts = 0;
} Hid_info;

/**
 * @brief Initialize a hid api.
 * @return On success DEF_SUCCESS, on failure DEF_ERR_* or Nintendo API's error.
 * @warning Thread dangerous (untested)
*/
uint32_t Util_hid_init(void);

/**
 * @brief Uninitialize a error API.
 * Do nothing if hid api is not initialized.
 * @warning Thread dangerous (untested)
*/
void Util_hid_exit(void);

/**
 * @brief Check whether image is pressed.
 * Always return false if hid api is not initialized.
 * @param hid_state (in) Key info returned by Util_hid_query_key_state().
 * @param image (in) Image data.
 * @return Whether image is pressed.
 * @note Thread safe
*/
bool Util_hid_is_pressed(Hid_info hid_state, Draw_image_data image);

/**
 * @brief Check whether image is held.
 * Always return false if hid api is not initialized.
 * @param hid_state (in) Key info returned by Util_hid_query_key_state().
 * @param image (in) Image data.
 * @return Whether image is held.
 * @note Thread safe
*/
bool Util_hid_is_held(Hid_info hid_state, Draw_image_data image);

/**
 * @brief Check whether image is released.
 * Always return false if hid api is not initialized.
 * @param hid_state (in) Key info returned by Util_hid_query_key_state().
 * @param image (in) Image data.
 * @return Whether image is released.
 * @note Thread safe
*/
bool Util_hid_is_released(Hid_info hid_state, Draw_image_data image);

/**
 * @brief Query current key state.
 * @param out_key_state (out) Pointer for key info.
 * @return On success DEF_SUCCESS, on failure DEF_ERR_*.
 * @note Thread safe
*/
uint32_t Util_hid_query_key_state(Hid_info* out_key_state);

/**
 * @brief Add hid callback.
 * Always return false if hid api is not initialized.
 * @param callback (in) Pointer for callback function.
 * @return On success true,
 * on failure false.
 * @note Thread safe
*/
bool Util_hid_add_callback(void (*callback)(void));

/**
 * @brief Remove hid callback.
 * Do nothing if hid api is not initialized.
 * @param callback (in) Pointer for callback function.
 * @note Thread safe
*/
void Util_hid_remove_callback(void (*callback)(void));
}
#endif //!defined(HID_HPP)
