#if !defined(DEF_STR_TYPES_H)
#define DEF_STR_TYPES_H
#include <stdbool.h>
#include <stdint.h>

#define DEF_STR_INITIAL_CAPACITY			(uint32_t)(16)
#define DEF_STR_NEVER_NULL(str_ptr)			(const char*)(Util_str_is_valid(str_ptr) ? (str_ptr)->buffer : "")

typedef struct
{
	uint8_t sequencial_id;	//Used to detect string buffer changes.
	uint32_t capacity;		//Current buffer capacity (without NULL terminator, so (capacity + 1) bytes are allocated).
	uint32_t length;		//Current string length (without NULL terminator).
	char* buffer;		    //String buffer.
} Util_str;

#endif //!defined(DEF_STR_TYPES_H)
