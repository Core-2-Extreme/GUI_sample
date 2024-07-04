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
	bool p_a;
	bool p_b;
	bool p_x;
	bool p_y;
	bool p_c_up;
	bool p_c_down;
	bool p_c_left;
	bool p_c_right;
	bool p_d_up;
	bool p_d_down;
	bool p_d_left;
	bool p_d_right;
	bool p_l;
	bool p_r;
	bool p_zl;
	bool p_zr;
	bool p_start;
	bool p_select;
	bool p_cs_up;
	bool p_cs_down;
	bool p_cs_left;
	bool p_cs_right;
	bool p_touch;
	bool p_any;
	//Is button held.
	bool h_a;
	bool h_b;
	bool h_x;
	bool h_y;
	bool h_c_up;
	bool h_c_down;
	bool h_c_left;
	bool h_c_right;
	bool h_d_up;
	bool h_d_down;
	bool h_d_left;
	bool h_d_right;
	bool h_l;
	bool h_r;
	bool h_zl;
	bool h_zr;
	bool h_start;
	bool h_select;
	bool h_cs_up;
	bool h_cs_down;
	bool h_cs_left;
	bool h_cs_right;
	bool h_touch;
	bool h_any;
	//Is button released.
	bool r_a;
	bool r_b;
	bool r_x;
	bool r_y;
	bool r_c_up;
	bool r_c_down;
	bool r_c_left;
	bool r_c_right;
	bool r_d_up;
	bool r_d_down;
	bool r_d_left;
	bool r_d_right;
	bool r_l;
	bool r_r;
	bool r_zl;
	bool r_zr;
	bool r_start;
	bool r_select;
	bool r_cs_up;
	bool r_cs_down;
	bool r_cs_left;
	bool r_cs_right;
	bool r_touch;
	bool r_any;
	//CPAD and touch position.
	int cpad_x;
	int cpad_y;
	int touch_x;
	int touch_y;
	int touch_x_move;
	int touch_y_move;
	int held_time;
	//Timestamp for this data.
	uint64_t ts;
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
