#include "definitions.hpp"

#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "system/system_definitions.hpp"

#include "system/util/error_types.h"

//Include myself.
#include "system/util/str.h"


static uint32_t Util_str_get_optimal_buffer_capacity(Util_str* string);
static uint32_t Util_str_vformat_internal(Util_str* string, bool is_append, const char* format_string, va_list args);


uint32_t Util_str_init(Util_str* string)
{
	if(!string)
		goto invalid_arg;

	//Init struct.
	Util_str_free(string);

	string->buffer = (char*)malloc(DEF_STR_INITIAL_CAPACITY + 1);
	if(!string->buffer)
		goto out_of_memory;

	string->capacity = DEF_STR_INITIAL_CAPACITY;
	Util_str_clear(string);
	string->sequencial_id = 0;

	return DEF_SUCCESS;

	invalid_arg:
	return DEF_ERR_INVALID_ARG;

	out_of_memory:
	return DEF_ERR_OUT_OF_MEMORY;
}

void Util_str_free(Util_str* string)
{
	if(!string)
		return;

	string->sequencial_id = 0;
	string->capacity = 0;
	string->length = 0;
	free(string->buffer);
	string->buffer = NULL;
}

uint32_t Util_str_clear(Util_str* string)
{
	if(!Util_str_is_valid(string))
		goto invalid_arg;

	string->buffer[0] = 0x00;//NULL terminator.
	string->length = 0;
	string->sequencial_id++;

	//Don't waste too much memory.
	Util_str_resize(string, Util_str_get_optimal_buffer_capacity(string));

	return DEF_SUCCESS;

	invalid_arg:
	return DEF_ERR_INVALID_ARG;
}

uint32_t Util_str_set(Util_str* string, const char* source_string)
{
	uint32_t source_string_length = 0;
	uint32_t result = DEF_ERR_OTHER;

	if(!Util_str_is_valid(string) || !source_string)
		goto invalid_arg;

	source_string_length = strlen(source_string);

	if(source_string_length == 0)//User wanted to empty string data.
		return Util_str_clear(string);

	if(source_string_length > string->capacity)
	{
		//Source string is too large, try to enlarge our string buffer.
		//We need more buffer, try to enlarge it.
		result = Util_str_resize(string, source_string_length);
		if(result != DEF_SUCCESS)
			goto error_other;
	}

	memcpy(string->buffer, source_string, source_string_length);
	string->buffer[source_string_length] = 0x00;//NULL terminator.
	string->length = source_string_length;
	string->sequencial_id++;

	//Don't waste too much memory.
	Util_str_resize(string, Util_str_get_optimal_buffer_capacity(string));

	return DEF_SUCCESS;

	invalid_arg:
	return DEF_ERR_INVALID_ARG;

	error_other:
	return result;
}

uint32_t Util_str_add(Util_str* string, const char* source_string)
{
	uint32_t new_length = 0;
	uint32_t source_string_length = 0;
	uint32_t result = DEF_ERR_OTHER;

	if(!Util_str_is_valid(string) || !source_string)
		goto invalid_arg;

	source_string_length = strlen(source_string);

	if(source_string_length == 0)
		goto invalid_arg;

	new_length = string->length + source_string_length;
	if(new_length > string->capacity)
	{
		//We need more buffer, try to enlarge it.
		result = Util_str_resize(string, new_length);
		if(result != DEF_SUCCESS)
			goto error_other;
	}

	memcpy((string->buffer + string->length), source_string, source_string_length);
	string->buffer[new_length] = 0x00;//NULL terminator.
	string->length = new_length;
	string->sequencial_id++;

	//Don't waste too much memory.
	Util_str_resize(string, Util_str_get_optimal_buffer_capacity(string));

	return DEF_SUCCESS;

	invalid_arg:
	return DEF_ERR_INVALID_ARG;

	error_other:
	return result;
}

uint32_t Util_str_format(Util_str* string, const char* format_string, ...)
{
	uint32_t result = DEF_ERR_OTHER;
	va_list args;

	va_start(args, format_string);
	result = Util_str_vformat_internal(string, false, format_string, args);
	va_end(args);

	return result;
}

uint32_t Util_str_vformat(Util_str* string, const char* format_string, va_list args)
{
	return Util_str_vformat_internal(string, false, format_string, args);
}

uint32_t Util_str_format_append(Util_str* string, const char* format_string, ...)
{
	uint32_t result = DEF_ERR_OTHER;
	va_list args;

	va_start(args, format_string);
	result = Util_str_vformat_internal(string, true, format_string, args);
	va_end(args);

	return result;
}

uint32_t Util_str_vformat_append(Util_str* string, const char* format_string, va_list args)
{
	return Util_str_vformat_internal(string, true, format_string, args);
}

uint32_t Util_str_resize(Util_str* string, uint32_t new_capacity)
{
	char* temp_buffer = NULL;

	if(!Util_str_is_valid(string) || new_capacity == 0 || new_capacity == UINT32_MAX)
		goto invalid_arg;

	if(string->capacity == new_capacity)
		goto success;//Do nothing.

	//Try to resize buffer.
	temp_buffer = (char*)realloc(string->buffer, (new_capacity + 1));
	if(!temp_buffer)
		goto out_of_memory;

	//Update buffer information.
	string->buffer = temp_buffer;
	string->capacity = new_capacity;
	string->buffer[new_capacity] = 0x00;//NULL terminator.

	success:
	return DEF_SUCCESS;

	invalid_arg:
	return DEF_ERR_INVALID_ARG;

	out_of_memory:
	return DEF_ERR_OUT_OF_MEMORY;
}

bool Util_str_is_valid(Util_str* string)
{
	if(!string || !string->buffer || string->capacity == 0 || string->capacity == UINT32_MAX)
		return false;
	else
		return true;
}

bool Util_str_has_data(Util_str* string)
{
	if(!Util_str_is_valid(string))
		return false;
	else if(string->length == 0)
		return false;
	else
		return true;
}

static uint32_t Util_str_get_optimal_buffer_capacity(Util_str* string)
{
	uint32_t optimal_capacity = 0;

	if(!string)
		return optimal_capacity;

	//If we have too much buffer, resize it.
	if(string->length == 0)
		optimal_capacity = DEF_STR_INITIAL_CAPACITY;
	else if((string->capacity / 2) > string->length)
		optimal_capacity = (uint32_t)(string->length * 1.25);
	else
		optimal_capacity = string->capacity;

	return optimal_capacity;
}

static uint32_t Util_str_vformat_internal(Util_str* string, bool is_append, const char* format_string, va_list args)
{
	char* buffer = NULL;
	uint32_t remaining_capacity = 0;
	uint32_t old_length = 0;
	uint32_t new_length = 0;
	uint32_t result = DEF_ERR_OTHER;

	if(!Util_str_is_valid(string) || !format_string)
		goto invalid_arg;

	if(is_append)
		old_length = string->length;
	else
		old_length = 0;

	buffer = (string->buffer + old_length);
	remaining_capacity = (string->capacity - old_length) + 1;

	new_length = vsnprintf(buffer, remaining_capacity, format_string, args);
	new_length += old_length;

	if(new_length > string->capacity)
	{
		//We need more buffer, try to enlarge it.
		result = Util_str_resize(string, new_length);
		if(result != DEF_SUCCESS)
			goto error_other;

		//Update pointer (since realloc may change pointer)
		//and remaining size then retry.
		buffer = (string->buffer + old_length);
		remaining_capacity = (string->capacity - old_length) + 1;
		vsnprintf(buffer, remaining_capacity, format_string, args);
	}

	//NULL terminator was added by vsnprintf().
	string->length = new_length;
	string->sequencial_id++;

	//Don't waste too much memory.
	Util_str_resize(string, Util_str_get_optimal_buffer_capacity(string));

	return DEF_SUCCESS;

	invalid_arg:
	return DEF_ERR_INVALID_ARG;

	error_other:
	return result;
}
