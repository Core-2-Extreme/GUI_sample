#ifndef UTIL_HPP
#define UTIL_HPP

#include "system/types.hpp"

extern "C" void* __real_malloc(size_t size);
extern "C" void* __real_realloc(void* ptr, size_t size);
extern "C" void __real_free(void* ptr);
extern "C" void __real__free_r(struct _reent *r, void* ptr);
extern "C" void* __real_memalign(size_t alignment, size_t size);
extern "C" Result __real_APT_SetAppCpuTimeLimit(u32 percent);
extern "C" Result __real_APT_GetAppCpuTimeLimit(u32* percent);

/**
 * @brief Initialize util API.
 * @return On success DEF_SUCCESS,
 * on failure DEF_ERR_* or Nintendo API's error.
 * @warning Thread dangerous (untested)
*/
Result_with_string Util_init(void);

/**
 * @brief Uninitialize util API.
 * Do nothing if util api is not initialized.
 * @warning Thread dangerous (untested)
*/
void Util_exit(void);

/**
 * @brief Get watch usage.
 * @param handle (in) Watch handle to check.
 * Always return 0 if util api is not initialized.
 * @return Number of watched variables.
 * @note Thread safe
*/
u32 Util_get_watch_usage(Watch_handle handle);

/**
 * @brief Get total watch usage.
 * Always return 0 if util api is not initialized.
 * @return Number of total watched variables.
 * @note Thread safe
*/
u32 Util_get_watch_total_usage(void);

/**
 * @brief Add a variable to watch list.
 * Do nothing if util api is not initialized.
 * @param handle (in) Watch handle to link with.
 * @param variable (in) Pointer for variable to add to watch list.
 * @param length (in) Data length to watch.
 * @return On success DEF_SUCCESS,
 * on failure DEF_ERR_*.
 * @note Thread safe
*/
Result_with_string Util_add_watch(Watch_handle handle, void* variable, u32 length);

/**
 * @brief Remove a variable frin watch list.
 * Do nothing if util api is not initialized.
 * @param handle (in) Watch handle to search.
 * @param variable (in) Pointer for variable to remove from watch list.
 * @note Thread safe
*/
void Util_remove_watch(Watch_handle handle, void* variable);

/**
 * @brief Check if watched values were changed since last call of this function.
 * Always return false if util api is not initialized.
 * @param handles (in) Watch handle to check (bit field).
 * @return True if values were changed otherwise false.
 * @note Thread safe
*/
bool Util_is_watch_changed(Watch_handle_bit handles);

/**
 * @brief Parse a file.
 * @param source_data (in) Text data.
 * @param expected_items (in) Expected elements.
 * @param out_data (out) Array for parsed data.
 * @return On success DEF_SUCCESS,
 * on failure DEF_ERR_*.
 * @note Thread safe
*/
Result_with_string Util_parse_file(std::string source_data, int expected_items, std::string out_data[]);

/**
 * @brief Convert seconds to time (hh:mm:ss.ms).
 * @param input_sseconds (in) Seconds.
 * @return Converted time (hh:mm:ss.ms).
 * @note Thread safe
*/
std::string Util_convert_seconds_to_time(double input_seconds);

/**
 * @brief Convert [\\n], ["] and [\\\\] to escape expression.
 * @param in_data (in) Input text.
 * @return Converted text.
 * @note Thread safe
*/
std::string Util_encode_to_escape(std::string in_data);

/**
 * @brief Load a message.
 * @param file_name (in) File name in romfs:/gfx/msg/.
 * @param out_msg (out) Array for parsed message.
 * @param num_of_msg (in) Number of message.
 * @return On success DEF_SUCCESS,
 * on failure DEF_ERR_*.
 * @note Thread safe
*/
Result_with_string Util_load_msg(std::string file_name, std::string out_msg[], int num_of_msg);

/**
 * @brief Encode to base64.
 * @param source (in) Source data.
 * @param size (in) Source data size.
 * @return Encoded string,
 * @note Thread safe
*/
std::string Util_encode_to_base64(char* source, int size);

/**
 * @brief Decode from base64.
 * @param source (in) Source string.
 * @return Decoded string,
 * @note Thread safe
*/
std::string Util_decode_from_base64(std::string source);

/**
 * @brief Initialize a safe linear alloc API.
 * @return On success DEF_SUCCESS,
 * on failure DEF_ERR_* or Nintendo API's error.
 * @warning Thread dangerous (untested)
*/
Result_with_string Util_safe_linear_alloc_init(void);

/**
 * @brief Uninitialize a safe linear alloc API.
 * Do nothing if safe linear alloc api is not initialized.
 * @warning Thread dangerous (untested)
*/
void Util_safe_linear_alloc_exit(void);

/**
 * @brief Linear alloc.
 * Always return NULL if safe linear alloc api is not initialized.
 * @param size (in) Memory size (in byte).
 * @return On success pointer, on failure NULL.
 * @note Thread safe
*/
void* Util_safe_linear_alloc(size_t size);

/**
 * @brief Linear align.
 * Always return NULL if safe linear alloc api is not initialized.
 * @param alignment (in) Alignment.
 * @param size (in) Memory size (in byte).
 * @return On success pointer, on failure NULL.
 * @note Thread safe
*/
void* Util_safe_linear_align(size_t alignment, size_t size);

/**
 * @brief Linear realloc.
 * Always return NULL if safe linear alloc api is not initialized.
 * @param pointer (in) Old pointer.
 * @param size (in) New memory size (in byte).
 * @return On success pointer, on failure NULL.
 * @note Thread safe
*/
void* Util_safe_linear_realloc(void* pointer, size_t size);

/**
 * @brief Free linear memory.
 * Do nothing if safe linear alloc api is not initialized.
 * @note Thread safe
*/
void Util_safe_linear_free(void* pointer);

/**
 * @brief Check free linear memory size.
 * Always return 0 if safe linear alloc api is not initialized.
 * @return Free linear memory size.
 * @note Thread safe
*/
u32 Util_check_free_linear_space(void);

/**
 * @brief Check free memory size.
 * @return Free memory size.
 * @warning Thread dangerous (untested)
*/
u32 Util_check_free_ram(void);

/**
 * @brief Check max allowed core #1 usage.
 * @return Max allowed core #1 usage.
 * @warning Thread dangerous (untested)
*/
u32 Util_get_core_1_max(void);

/**
 * @brief Sleep thread.
 * @param ns (in) Time to sleep in us.
 * @note Thread safe
*/
void Util_sleep(s64 us);

/**
 * @brief Compare values and return minimum value.
 * @param value_0 (in) Value to compare.
 * @param value_1 (in) Value to compare.
 * @return Minimum value.
 * @note Thread safe
*/
long Util_min(long value_0, long value_1);

/**
 * @brief Compare values and return maximum value.
 * @param value_0 (in) Value to compare.
 * @param value_1 (in) Value to compare.
 * @return Maximum value.
 * @note Thread safe
*/
long Util_max(long value_0, long value_1);

/**
 * @brief To prevent 'statement has no effect' warning in unused functions.
*/
bool Util_return_bool(bool value);
/**
 * @brief To prevent 'statement has no effect' warning in unused functions.
*/
int Util_return_int(int value);
/**
 * @brief To prevent 'statement has no effect' warning in unused functions.
*/
double Util_return_double(double value);
/**
 * @brief To prevent 'statement has no effect' warning in unused functions.
*/
std::string Util_return_string(std::string string);
/**
 * @brief To prevent 'statement has no effect' warning in unused functions.
*/
Result_with_string Util_return_result_with_string(Result_with_string value);

#endif
