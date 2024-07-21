#if !defined(DEF_SWKBD_HPP)
#define DEF_SWKBD_HPP
#include <stdbool.h>
#include <stdint.h>
#include "system/util/str_types.h"
#include "system/util/swkbd_types.h"

#if DEF_ENABLE_SWKBD_API

/**
 * @brief Initialize a software keyboard.
 * @param type (in) Software keyboard type.
 * @param valid_type (in) Accepted input type.
 * @param button_type (in) Buttons to display.
 * @param max_length (in) Max input length in number of characters, NOT in bytes.
 * @param hint_text (in) Hint text.
 * @param init_text (in) Initial text.
 * @param password_mode (in) Password mode.
 * @param feature (in) Software keyboard feature.
 * @return On success DEF_SUCCESS, on failure DEF_ERR_*.
 * @warning Thread dangerous (untested)
*/
uint32_t Util_swkbd_init(Util_swkbd_type type, Util_swkbd_acceptable_input valid_type, Util_swkbd_display_button button_type,
uint32_t max_length, Util_str* hint_text, Util_str* init_text, Util_swkbd_password_mode password_mode, Util_swkbd_features_bit features);

/**
 * @brief Set dictionary word.
 * @param first_spell (in) Array for word's spell.
 * @param full_spell (in) Array for word's full spell.
 * @param num_of_word (in) Number of word.
 * @return On success DEF_SUCCESS, on failure DEF_ERR_*.
 * @warning Thread dangerous (untested)
*/
uint32_t Util_swkbd_set_dic_word(Util_str* first_spell, Util_str* full_spell, uint16_t num_of_word);

/**
 * @brief Launch software keyboard.
 * @param out_data (out) Pointer for user input text.
 * @param pressed_button (out) Pressed button, can be NULL.
 * @return On success DEF_SUCCESS, on failure DEF_ERR_*.
 * @warning Call it only from rendering thread.
 * @warning Thread dangerous (untested)
*/
uint32_t Util_swkbd_launch(Util_str* out_data, Util_swkbd_button* pressed_button);

/**
 * @brief Uninitialize a software keyboard.
 * Do nothing if swkbd api is not initialized.
 * @warning Thread dangerous (untested)
*/
void Util_swkbd_exit(void);

#else

#define Util_swkbd_init(...) Util_return_result_with_string(var_disabled_result)
#define Util_swkbd_set_dic_word(...) Util_return_result_with_string(var_disabled_result)
#define Util_swkbd_launch(...) Util_return_result_with_string(var_disabled_result)
#define Util_swkbd_exit()

#endif //DEF_ENABLE_SWKBD_API

#endif //!defined(DEF_SWKBD_HPP)
