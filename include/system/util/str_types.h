#if !defined(DEF_STR_TYPES_H)
#define DEF_STR_TYPES_H
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

#endif //!defined(DEF_STR_TYPES_H)
