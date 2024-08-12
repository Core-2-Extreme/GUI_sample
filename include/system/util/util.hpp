#if !defined(DEF_UTIL_HPP)
#define DEF_UTIL_HPP
#include <stdbool.h>
#include <stdint.h>
#include <string>
#include "3ds.h"
#include "system/util/str_types.h"

extern "C" void* __real_malloc(size_t size);
extern "C" void* __real_realloc(void* ptr, size_t size);
extern "C" void __real_free(void* ptr);
extern "C" void __real__free_r(struct _reent *r, void* ptr);
extern "C" void* __real_memalign(size_t alignment, size_t size);
extern "C" Result __real_APT_SetAppCpuTimeLimit(uint32_t percent);
extern "C" Result __real_APT_GetAppCpuTimeLimit(uint32_t* percent);

/**
 * @brief Initialize util API.
 * @return On success DEF_SUCCESS, on failure DEF_ERR_*.
 * @warning Thread dangerous (untested)
*/
uint32_t Util_init(void);

/**
 * @brief Uninitialize util API.
 * Do nothing if util api is not initialized.
 * @warning Thread dangerous (untested)
*/
void Util_exit(void);

/**
 * @brief Parse a file.
 * @param source_data (in) Text data.
 * @param expected_items (in) Expected elements.
 * @param out_data (out) Array for parsed data.
 * @return On success DEF_SUCCESS, on failure DEF_ERR_*.
 * @note Thread safe
*/
uint32_t Util_parse_file(const char* source_data, uint32_t expected_items, Str_data* out_data);

/**
 * @brief Convert seconds to time (hh:mm:ss.ms).
 * @param input_sseconds (in) Seconds.
 * @param time_string (out) Formatted string (hh:mm:ss.ms).
 * @return On success DEF_SUCCESS, on failure DEF_ERR_*.
 * @note Thread safe
*/
uint32_t Util_convert_seconds_to_time(double input_seconds, Str_data* time_string);

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
 * @return On success DEF_SUCCESS, on failure DEF_ERR_*.
 * @note Thread safe
*/
uint32_t Util_load_msg(const char* file_name, Str_data* out_msg, uint32_t num_of_msg);

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
 * @return On success DEF_SUCCESS, on failure DEF_ERR_*.
 * @warning Thread dangerous (untested)
*/
uint32_t Util_safe_linear_alloc_init(void);

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
uint32_t Util_check_free_linear_space(void);

/**
 * @brief Check free memory size.
 * @return Free memory size.
 * @warning Thread dangerous (untested)
*/
uint32_t Util_check_free_ram(void);

/**
 * @brief Check max allowed core #1 usage.
 * @return Max allowed core #1 usage.
 * @warning Thread dangerous (untested)
*/
uint32_t Util_get_core_1_max(void);

/**
 * @brief Sleep thread.
 * @param ns (in) Time to sleep in us.
 * @note Thread safe
*/
void Util_sleep(uint64_t us);

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

#endif //!defined(DEF_UTIL_HPP)
