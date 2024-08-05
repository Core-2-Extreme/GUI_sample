#if !defined(DEF_HTTPC_HPP)
#define DEF_HTTPC_HPP
#include <stdbool.h>
#include <stdint.h>
#include "system/util/httpc_types.h"
#include "system/util/str_types.h"

#if DEF_HTTPC_API_ENABLE

/**
 * @brief Initialize a httpc api.
 * @param buffer_size Internal buffer size used by post request.
 * @return On success DEF_SUCCESS, on failure DEF_ERR_* or Nintendo API's error.
 * @warning Thread dangerous (untested)
*/
uint32_t Util_httpc_init(uint32_t buffer_size);

/**
 * @brief Uninitialize a httpc API.
 * Do nothing if httpc api is not initialized.
 * @warning Thread dangerous (untested)
*/
void Util_httpc_exit(void);

/**
 * @brief Make a HTTP get request.
 * @param url (in) URL.
 * @param data (out) Pointer for response data, the pointer will be allocated up to max_size
 * depends on server response.
 * @param max_size (in) Max download size.
 * @param downloaded_size (out) Actuall downloaded size, can be NULL.
 * @param status_code (out) HTTP status code, can be NULL.
 * @param max_redirect (in) Maximum number of redirect (0 to disallow redirect).
 * @param last_url (out) Last url (data contains response of last url), can be NULL.
 * @return On success DEF_SUCCESS, on failure DEF_ERR_* or Nintendo API's error.
 * @note Thread safe
*/
uint32_t Util_httpc_dl_data(const char* url, uint8_t** data, uint32_t max_size, uint32_t* downloaded_size,
uint16_t* status_code, uint16_t max_redirect, Str_data* last_url);

/**
 * @brief Make a HTTP get request and save response to SD card.
 * @param url (in) URL.
 * @param buffer_size (in) Internal work buffer size.
 * @param downloaded_size (out) Actuall downloaded size, can be NULL.
 * @param status_code (out) HTTP status code, can be NULL.
 * @param max_redirect (in) Maximum number of redirect (0 to disallow redirect).
 * @param last_url (out) Last url (response of last url will be saved), can be NULL.
 * @param dir_path (in) Directory path.
 * @param file_name (in) File name if file already exist, the file will be overwritten.
 * @return On success DEF_SUCCESS, on failure DEF_ERR_* or Nintendo API's error.
 * @note Thread safe
*/
uint32_t Util_httpc_save_data(const char* url, uint32_t buffer_size, uint32_t* downloaded_size, uint16_t* status_code,
uint16_t max_redirect, Str_data* last_url, const char* dir_path, const char* file_name);

/**
 * @brief Make a HTTP post request.
 * @param url (in) URL.
 * @param post_data (in) Pointer for post data.
 * @param post_size (in) Post data size.
 * @param dl_data (out) Pointer for response data, the pointer will be allocated up to max_size
 * depends on server response.
 * @param max_dl_size (in) Max download size.
 * @param downloaded_size (out) Actuall downloaded size, can be NULL.
 * @param status_code (out) HTTP status code, can be NULL.
 * @param max_redirect (in) Maximum number of redirect (0 to disallow redirect).
 * @param last_url (out) Last url (dl_data contains response of last url), can be NULL.
 * @return On success DEF_SUCCESS, on failure DEF_ERR_* or Nintendo API's error.
 * @note Thread safe
*/
uint32_t Util_httpc_post_and_dl_data(const char* url, uint8_t* post_data, uint32_t post_size, uint8_t** dl_data, uint32_t max_dl_size,
uint32_t* downloaded_size, uint16_t* status_code, uint16_t max_redirect, Str_data* last_url);

/**
 * @brief Make a HTTP post request and save response to SD card.
 * @param url (in) URL.
 * @param post_data (in) Pointer for post data.
 * @param post_size (in) Post data size.
 * @param buffer_size (in) Internal work buffer size.
 * @param downloaded_size (out) Actuall downloaded size, can be NULL.
 * @param status_code (out) HTTP status code, can be NULL.
 * @param max_redirect (in) Maximum number of redirect (0 to disallow redirect).
 * @param last_url (out) Last url (response of last url will be saved), can be NULL.
 * @param dir_path (in) Directory path.
 * @param file_name (in) File name if file already exist, the file will be overwritten.
 * @return On success DEF_SUCCESS, on failure DEF_ERR_* or Nintendo API's error.
 * @note Thread safe
*/
uint32_t Util_httpc_post_and_save_data(const char* url, uint8_t* post_data, uint32_t post_size, uint32_t buffer_size, uint32_t* downloaded_size,
uint16_t* status_code, uint16_t max_redirect, Str_data* last_url, const char* dir_path, const char* file_name);

#else

#define Util_httpc_init(...) DEF_ERR_DISABLED
#define Util_httpc_exit()
#define Util_httpc_dl_data(...) DEF_ERR_DISABLED
#define Util_httpc_save_data(...) DEF_ERR_DISABLED
#define Util_httpc_post_and_dl_data(...) DEF_ERR_DISABLED
#define Util_httpc_post_and_save_data(...) DEF_ERR_DISABLED

#endif //DEF_HTTPC_API_ENABLE

#endif //!defined(DEF_HTTPC_HPP)
