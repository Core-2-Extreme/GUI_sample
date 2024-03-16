#ifndef _STR_H
#define _STR_H
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>

#define DEF_STR_INITIAL_CAPACITY 16

typedef struct
{
	uint8_t sequencial_id;	//Used to detect string buffer changes.
	uint32_t capacity;		//Current buffer capacity (without NULL terminator, so (capacity + 1) bytes are allocated).
	uint32_t length;		//Current string length (without NULL terminator).
	char* buffer;		    //String buffer.
} Util_str;

/**
 * @brief Initialize a string struct.
 * @param string Pointer for target struct.
 * @return On success DEF_SUCCESS, on failure DEF_ERR_*.
 * @note Thread safe
*/
uint32_t Util_str_init(Util_str* string);

/**
 * @brief Free a string struct.
 * @param string Pointer for target struct.
 * @return On success DEF_SUCCESS, on failure DEF_ERR_*.
 * @note Thread safe
*/
void Util_str_free(Util_str* string);

/**
 * @brief Clear string data.
 * @param string Pointer for target struct.
 * @return On success DEF_SUCCESS, on failure DEF_ERR_*.
 * @note Thread safe
*/
uint32_t Util_str_clear(Util_str* string);

/**
 * @brief Set string data.
 * @param string Pointer for target struct.
 * @param source_string Pointer for source string.
 * @return On success DEF_SUCCESS, on failure DEF_ERR_*.
 * @note Thread safe
*/
uint32_t Util_str_set(Util_str* string, const char* source_string);

/**
 * @brief Add (append) string data.
 * @param string Pointer for target struct.
 * @param source_string Pointer for source string to add.
 * @return On success DEF_SUCCESS, on failure DEF_ERR_*.
 * @note Thread safe
*/
uint32_t Util_str_add(Util_str* string, const char* source_string);

/**
 * @brief Set string data with format.
 * @param string Pointer for target struct.
 * @param format_string Pointer for format string.
 * @param additional_parameters Additional parameters for format_string.
 * @return On success DEF_SUCCESS, on failure DEF_ERR_*.
 * @note Thread safe
*/
uint32_t Util_str_format(Util_str* string, const char* format_string, ...);

/**
 * @brief va_list version of Util_str_format().
 * @param string Pointer for target struct.
 * @param format_string Pointer for format string.
 * @param args Additional parameters for format_string.
 * @return On success DEF_SUCCESS, on failure DEF_ERR_*.
 * @note Thread safe
*/
uint32_t Util_str_vformat(Util_str* string, const char* format_string, va_list args);

/**
 * @brief Same as Util_str_format() except this will append text instead of overwrite old one.
 * @param string Pointer for target struct.
 * @param format_string Pointer for format string.
 * @param additional_parameters Additional parameters for format_string.
 * @return On success DEF_SUCCESS, on failure DEF_ERR_*.
 * @note Thread safe
*/
uint32_t Util_str_format_append(Util_str* string, const char* format_string, ...);

/**
 * @brief va_list version of Util_str_format_append().
 * @param string Pointer for target struct.
 * @param format_string Pointer for format string.
 * @param args Additional parameters for format_string.
 * @return On success DEF_SUCCESS, on failure DEF_ERR_*.
 * @note Thread safe
*/
uint32_t Util_str_vformat_append(Util_str* string, const char* format_string, va_list args);

/**
 * @brief Resize string buffer size.
 * @note If new_capacity is less than current string length,
 * string data WILL BE TRUNCATED to length of new_capacity.
 * @param string Pointer for target struct.
 * @param new_capacity New buffer capacity.
 * @return On success DEF_SUCCESS, on failure DEF_ERR_*.
 * @note Thread safe
*/
uint32_t Util_str_resize(Util_str* string, uint32_t new_capacity);

/**
 * @brief Check if struct is valid (so that safe to access the buffer).
 * @note If string is empty (buffer is allocated but string length is 0)
 * this will return true.
 * @param string Pointer for target struct.
 * @return True if struct is valid, otherwise false.
 * @note Thread safe
*/
bool Util_str_is_valid(Util_str* string);

/**
 * @brief Check if struct is valid and contains at least 1 character.
 * @note If string is empty (buffer is allocated but string length is 0)
 * this will return false.
 * @param string Pointer for target struct.
 * @return True if struct is valid AND contains at least 1 character, otherwise false.
 * @note Thread safe
*/
bool Util_str_has_data(Util_str* string);

#endif //_STR_H
