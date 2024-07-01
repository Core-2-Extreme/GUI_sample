#if !defined(ERROR_HPP)
#define ERROR_HPP

#include "system/types.hpp"
#include "system/util/hid.hpp"

extern "C"
{
#define DEF_ERR_NO_RESULT_CODE		(uint32_t)(1234567890)	//No result codes available.

/**
 * @brief Initialize a error api.
 * @return On success DEF_SUCCESS, on failure DEF_ERR_*.
 * @warning Thread dangerous (untested)
*/
uint32_t Util_err_init(void);

/**
 * @brief Uninitialize a error API.
 * Do nothing if error api is not initialized.
 * @warning Thread dangerous (untested)
*/
void Util_err_exit(void);

/**
 * @brief Query error show flag.
 * Always return false if error api is not initialized.
 * @return Internal error show flag.
 * @warning Thread dangerous (untested)
*/
bool Util_err_query_error_show_flag(void);

/**
 * @brief Set error message.
 * Do nothing if error api is not initialized.
 * @param summary (in) Error summary.
 * @param description (in) Error description.
 * @param location (in) Error location.
 * @param error_code (in) Error code.
 * @warning Thread dangerous (untested)
*/
void Util_err_set_error_message(const char* summary, const char* description, const char* location, uint32_t error_code);

/**
 * @brief Set error show flag.
 * Do nothing if error api is not initialized.
 * @param flag (in) When true, internal error show flag will be set to true otherwise set to false.
 * @warning Thread dangerous (untested)
*/
void Util_err_set_error_show_flag(bool flag);

/**
 * @brief Clear error message.
 * Do nothing if error api is not initialized.
 * @warning Thread dangerous (untested)
*/
void Util_err_clear_error_message(void);

/**
 * @brief Save error message to SD card.
 * After saving error message, error show flag will be set to false.
 * Do nothing if error api is not initialized.
 * @warning Thread dangerous (untested)
*/
void Util_err_save_error(void);

/**
 * @brief Process user input for Util_err_draw().
 * @param key (in) key info returned by Util_hid_query_key_state().
 * @warning Thread dangerous (untested)
*/
void Util_err_main(Hid_info key);

/**
 * @brief Draw error message.
 * @warning Thread dangerous (untested)
 * @warning Call it from only drawing thread.
*/
void Util_err_draw(void);
}
#endif //!defined(ERROR_HPP)
