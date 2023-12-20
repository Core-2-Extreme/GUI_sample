#include "definitions.hpp"

#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "system/system_definitions.hpp"

//Include myself.
#include "system/util/string.h"


static uint32_t Util_string_get_optimal_buffer_capacity(Util_string* string);


uint32_t Util_string_init(Util_string* string)
{
	if(!string)
		goto invalid_arg;

	//Init struct.
	Util_string_free(string);

	string->buffer = (char*)malloc(DEF_STRING_INITIAL_CAPACITY + 1);
	if(!string->buffer)
		goto out_of_memory;

	string->capacity = DEF_STRING_INITIAL_CAPACITY;
	Util_string_clear(string);
	string->sequencial_id = 0;

	return DEF_SUCCESS;

	invalid_arg:
	return DEF_ERR_INVALID_ARG;

	out_of_memory:
	return DEF_ERR_OUT_OF_MEMORY;
}

void Util_string_free(Util_string* string)
{
	if(!string)
		return;

	string->sequencial_id = 0;
	string->capacity = 0;
	string->length = 0;
	free(string->buffer);
	string->buffer = NULL;
}

uint32_t Util_string_clear(Util_string* string)
{
	if(!Util_string_is_valid(string))
		goto invalid_arg;

	string->buffer[0] = 0x00;//NULL terminator.
	string->length = 0;
	string->sequencial_id++;

	//Don't waste too much memory.
	Util_string_resize(string, Util_string_get_optimal_buffer_capacity(string));

	return DEF_SUCCESS;

	invalid_arg:
	return DEF_ERR_INVALID_ARG;
}

uint32_t Util_string_set(Util_string* string, const char* source_string)
{
	uint32_t source_string_length = 0;
	uint32_t result = DEF_ERR_OTHER;

	if(!Util_string_is_valid(string) || !source_string)
		goto invalid_arg;

	source_string_length = strlen(source_string);

	if(source_string_length == 0)//User wanted to empty string data.
		return Util_string_clear(string);

	if(source_string_length > string->capacity)
	{
		//Source string is too large, try to enlarge our string buffer.
		//We need more buffer, try to enlarge it.
		result = Util_string_resize(string, source_string_length);
		if(result != DEF_SUCCESS)
			goto error_other;
	}

	memcpy(string->buffer, source_string, source_string_length);
	string->buffer[source_string_length] = 0x00;//NULL terminator.
	string->length = source_string_length;
	string->sequencial_id++;

	//Don't waste too much memory.
	Util_string_resize(string, Util_string_get_optimal_buffer_capacity(string));

	return DEF_SUCCESS;

	invalid_arg:
	return DEF_ERR_INVALID_ARG;

	error_other:
	return result;
}

uint32_t Util_string_add(Util_string* string, const char* source_string)
{
	uint32_t new_length = 0;
	uint32_t source_string_length = 0;
	uint32_t result = DEF_ERR_OTHER;

	if(!Util_string_is_valid(string) || !source_string)
		goto invalid_arg;

	source_string_length = strlen(source_string);

	if(source_string_length == 0)
		goto invalid_arg;

	new_length = string->length + source_string_length;
	if(new_length > string->capacity)
	{
		//We need more buffer, try to enlarge it.
		result = Util_string_resize(string, new_length);
		if(result != DEF_SUCCESS)
			goto error_other;
	}

	memcpy((string->buffer + string->length), source_string, source_string_length);
	string->buffer[new_length] = 0x00;//NULL terminator.
	string->length = new_length;
	string->sequencial_id++;

	//Don't waste too much memory.
	Util_string_resize(string, Util_string_get_optimal_buffer_capacity(string));

	return DEF_SUCCESS;

	invalid_arg:
	return DEF_ERR_INVALID_ARG;

	error_other:
	return result;
}

uint32_t Util_string_format(Util_string* string, const char* format_string, ...)
{
	uint32_t new_length = 0;
	uint32_t result = DEF_ERR_OTHER;
	va_list args;

	if(!Util_string_is_valid(string) || !format_string)
		goto invalid_arg;

	va_start(args, format_string);
	new_length = vsnprintf(string->buffer, (string->capacity + 1), format_string, args);

	if(new_length > string->capacity)
	{
		//We need more buffer, try to enlarge it.
		result = Util_string_resize(string, new_length);
		if(result == DEF_SUCCESS)
		{
			//Retry it.
			vsnprintf(string->buffer, (string->capacity + 1), format_string, args);
		}
		va_end(args);

		if(result != DEF_SUCCESS)
			goto error_other;
	}
	else
		va_end(args);

	//NULL terminator was added by vsnprintf().
	string->length = new_length;
	string->sequencial_id++;

	//Don't waste too much memory.
	Util_string_resize(string, Util_string_get_optimal_buffer_capacity(string));

	return DEF_SUCCESS;

	invalid_arg:
	return DEF_ERR_INVALID_ARG;

	error_other:
	return result;
}

uint32_t Util_string_resize(Util_string* string, uint32_t new_capacity)
{
	char* temp_buffer = NULL;

	if(!Util_string_is_valid(string) || new_capacity == 0 || new_capacity == UINT32_MAX)
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

bool Util_string_is_valid(Util_string* string)
{
	if(!string || !string->buffer || string->capacity == 0 || string->capacity == UINT32_MAX)
		return false;
	else
		return true;
}

bool Util_string_has_data(Util_string* string)
{
	if(!Util_string_is_valid(string))
		return false;
	else if(string->length == 0)
		return false;
	else
		return true;
}

static uint32_t Util_string_get_optimal_buffer_capacity(Util_string* string)
{
	uint32_t optimal_capacity = 0;

	if(!string)
		return optimal_capacity;

	//If we have too much buffer, resize it.
	if(string->length == 0)
		optimal_capacity = DEF_STRING_INITIAL_CAPACITY;
	else if((string->capacity / 2) > string->length)
		optimal_capacity = (uint32_t)(string->length * 1.25);
	else
		optimal_capacity = string->capacity;

	return optimal_capacity;
}
