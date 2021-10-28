#include "system/headers.hpp"

Handle util_safe_linear_memory_mutex = -1;

extern "C" void memcpy_asm(u8*, u8*, int);

Result_with_string Util_parse_file(std::string source_data, int num_of_items, std::string out_data[])
{
	Result_with_string result;

	size_t start_num = 0;
	size_t end_num = 0;
	std::string start_text;
	std::string end_text;

	for (int i = 0; i < num_of_items; i++)
	{
		start_text = "<" + std::to_string(i) + ">";
		start_num = source_data.find(start_text);
		end_text = "</" + std::to_string(i) + ">";
		end_num = source_data.find(end_text);

		if (end_num == std::string::npos || start_num == std::string::npos)
		{
			result.code = -1;
			result.string = "[Error] Failed to parse file. error pos : " + std::to_string(i) + " ";
			break;
		}

		start_num += start_text.length();
		end_num -= start_num;
		out_data[i] = source_data.substr(start_num, end_num);
	}

	return result;
}

std::string Util_convert_seconds_to_time(double input_seconds)
{
	int hours = 0;
	int minutes = 0;
	int seconds = 0;
	long count = 0;
	std::string time = "";

	if(std::isnan(input_seconds) || std::isinf(input_seconds))
		input_seconds = 0;
	
	for(count = 0; count < (int)input_seconds; count++)
	{
		if(seconds + 1 >= 60)
		{
			if(minutes + 1 >= 60)
			{
				seconds = 0;
				minutes = 0;
				hours++;
			}
			else
			{
				seconds = 0;
				minutes++;
			}
		}
		else
			seconds++;
	}

	if(hours != 0)
		time += std::to_string(hours) + ":";

	if(minutes < 10)
		time += "0" + std::to_string(minutes) + ":";
	else
		time += std::to_string(minutes) + ":";

	if(seconds < 10)
		time += "0" + std::to_string(seconds);
	else
		time += std::to_string(seconds);

	time += std::to_string(input_seconds - count + 1).substr(1, 2);
	return time;
}

std::string Util_encode_to_escape(std::string in_data)
{
	int string_length = in_data.length();
	std::string check;
	std::string return_data = "";

	for(int i = 0; i < string_length; i++)
	{
		check = in_data.substr(i, 1);
		if(check == "\n")
			return_data += "\\n";
		else if(check == "\u0022")
			return_data += "\\\u0022";
		else if(check == "\u005c")
			return_data += "\\\u005c";
		else
			return_data += in_data.substr(i, 1);
	}

	return return_data;
}

Result_with_string Util_load_msg(std::string file_name, std::string out_msg[], int num_of_msg)
{
	u8* fs_buffer = NULL;
	u32 read_size = 0;
	Result_with_string result;
	fs_buffer = (u8*)malloc(0x2000);
	if(fs_buffer == NULL)
	{
		result.code = DEF_ERR_OUT_OF_MEMORY;
		result.string = DEF_ERR_OUT_OF_MEMORY_STR;
		return result;
	}

	result = Util_file_load_from_rom(file_name, "romfs:/gfx/msg/", fs_buffer, 0x2000, &read_size);
	if (result.code != 0)
	{
		free(fs_buffer);
		return result;
	}

	result = Util_parse_file((char*)fs_buffer, num_of_msg, out_msg);
	if (result.code != 0)
	{
		free(fs_buffer);
		return result;
	}

	free(fs_buffer);
	return result;
}

void Util_init(void)
{
	svcCreateMutex(&util_safe_linear_memory_mutex, false);
}

void Util_exit(void)
{
	svcCloseHandle(util_safe_linear_memory_mutex);
}

void* Util_safe_linear_alloc(size_t size)
{
	void* pointer = NULL;

	svcWaitSynchronization(util_safe_linear_memory_mutex, U64_MAX);
	pointer = linearAlloc(size);
	svcReleaseMutex(util_safe_linear_memory_mutex);

	return pointer;
}

void* __attribute__((optimize("O0"))) Util_safe_linear_realloc(void* pointer, size_t size)
{
	void* new_ptr = NULL;
	u32 pointer_size = 0;

	if(size == 0)
	{
		Util_safe_linear_free(pointer);
		return pointer;
	}
	if(!pointer)
		return Util_safe_linear_alloc(size);

	new_ptr = Util_safe_linear_alloc(size);
	if(new_ptr)
	{
		svcWaitSynchronization(util_safe_linear_memory_mutex, U64_MAX);
		pointer_size = linearGetSize(pointer);
		svcReleaseMutex(util_safe_linear_memory_mutex);
		
		if(size > pointer_size)
			memcpy_asm((u8*)new_ptr, (u8*)pointer, pointer_size);
		else
			memcpy_asm((u8*)new_ptr, (u8*)pointer, size);

		Util_safe_linear_free(pointer);
	}
	return new_ptr;
}

void Util_safe_linear_free(void* pointer)
{
	svcWaitSynchronization(util_safe_linear_memory_mutex, U64_MAX);
	linearFree(pointer);
	svcReleaseMutex(util_safe_linear_memory_mutex);
}

u32 Util_check_free_linear_space(void)
{
	u32 space = 0;
	svcWaitSynchronization(util_safe_linear_memory_mutex, U64_MAX);
	space = linearSpaceFree();
	svcReleaseMutex(util_safe_linear_memory_mutex);
	return space;
}

u32 Util_check_free_ram(void)
{
	u8* malloc_check[2000];
	u32 count;

	for (int i = 0; i < 2000; i++)
		malloc_check[i] = NULL;

	for (count = 0; count < 2000; count++)
	{
		malloc_check[count] = (u8*)malloc(0x186A0);// 100KB
		if (malloc_check[count] == NULL)
			break;
	}

	for (u32 i = 0; i <= count; i++)
		free(malloc_check[i]);

	return count * 100 * 1024;//return free B
}
