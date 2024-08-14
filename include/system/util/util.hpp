#if !defined(DEF_UTIL_HPP)
#define DEF_UTIL_HPP
#include <stdbool.h>
#include <stdint.h>
#include <string>

/**
 * @brief Convert [\\n], ["] and [\\\\] to escape expression.
 * @param in_data (in) Input text.
 * @return Converted text.
 * @note Thread safe
*/
std::string Util_encode_to_escape(std::string in_data);

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

#endif //!defined(DEF_UTIL_HPP)
