#include "definitions.hpp"
#include "system/types.hpp"

#include "system/util/explorer.hpp"
#include "system/util/log.hpp"
#include "system/util/util.hpp"
extern "C"
{
#include "system/util/str.h"
}

//Include myself.
#include "system/util/file.hpp"


static Result_with_string Util_file_load_from_file_with_range_internal(std::string&& file_name, std::string&& dir_path, uint8_t** read_data, int max_size, uint64_t read_offset, uint32_t* read_size);
static Result_with_string Util_file_load_from_rom_internal(std::string&& file_name, std::string&& dir_path, uint8_t** read_data, int max_size, uint32_t* read_size);


Result_with_string Util_file_save_to_file(std::string file_name, std::string dir_path, uint8_t* write_data, int size, bool delete_old_file)
{
	uint16_t* utf16_dir_path = NULL;
	uint16_t* utf16_path = NULL;
	uint32_t written_size = 0;
	uint64_t offset = 0;
	ssize_t utf_out_size = 0;
	std::string path = "";
	Handle handle = 0;
	FS_Archive archive = 0;
	Result_with_string result;

	if(file_name == "" || dir_path == "" || !write_data || size <= 0)
		goto invalid_arg;

	path = dir_path + file_name;
	utf16_dir_path = (uint16_t*)malloc(4096 + 2);
	utf16_path = (uint16_t*)malloc(4096 + 2);
	if(!utf16_dir_path || !utf16_path)
		goto out_of_memory;

	utf_out_size = utf8_to_utf16(utf16_dir_path, (uint8_t*)dir_path.c_str(), 2048);
	utf16_dir_path[(utf_out_size < 0 ? 0 : utf_out_size)] = 0x00;//Add a null terminator.

	utf_out_size = utf8_to_utf16(utf16_path, (uint8_t*)path.c_str(), 2048);
	utf16_path[(utf_out_size < 0 ? 0 : utf_out_size)] = 0x00;//Add a null terminator.

	result.code = FSUSER_OpenArchive(&archive, ARCHIVE_SDMC, fsMakePath(PATH_EMPTY, ""));
	if(result.code != 0)
	{
		result.error_description = "[Error] FSUSER_OpenArchive() failed. ";
		goto nintendo_api_failed;
	}

	result.code = FSUSER_CreateDirectory(archive, fsMakePath(PATH_UTF16, utf16_dir_path), FS_ATTRIBUTE_DIRECTORY);
	if (result.code != 0 && result.code != 0xC82044BE)//#0xC82044BE directory already exist
	{
		result.error_description = "[Error] FSUSER_CreateDirectory() failed. ";
		goto nintendo_api_failed;
	}

	if (delete_old_file)
		FSUSER_DeleteFile(archive, fsMakePath(PATH_UTF16, utf16_path));

	result.code = FSUSER_CreateFile(archive, fsMakePath(PATH_UTF16, utf16_path), FS_ATTRIBUTE_ARCHIVE, 0);
	if (result.code != 0 && result.code != 0xC82044BE)//#0xC82044BE file already exist
	{
		result.error_description = "[Error] FSUSER_CreateFile() failed. ";
		goto nintendo_api_failed;
	}

	result.code = FSUSER_OpenFile(&handle, archive, fsMakePath(PATH_UTF16, utf16_path), FS_OPEN_WRITE, FS_ATTRIBUTE_ARCHIVE);
	if (result.code != 0)
	{
		result.error_description = "[Error] FSUSER_OpenFile() failed. ";
		goto nintendo_api_failed;
	}

	if (!delete_old_file)
	{
		result.code = FSFILE_GetSize(handle, &offset);
		if (result.code != 0)
		{
			result.error_description = "[Error] FSFILE_GetSize() failed. ";
			goto nintendo_api_failed;
		}
	}

	result.code = FSFILE_Write(handle, &written_size, offset, write_data, size, FS_WRITE_FLUSH);
	if (result.code != 0)
	{
		result.error_description = "[Error] FSFILE_Write() failed. ";
		goto nintendo_api_failed;
	}

	free(utf16_dir_path);
	free(utf16_path);
	utf16_dir_path = NULL;
	utf16_path = NULL;
	FSFILE_Close(handle);
	FSUSER_CloseArchive(archive);

	return result;

	invalid_arg:
	result.code = DEF_ERR_INVALID_ARG;
	result.string = DEF_ERR_INVALID_ARG_STR;
	return result;

	out_of_memory:
	free(utf16_dir_path);
	free(utf16_path);
	utf16_dir_path = NULL;
	utf16_path = NULL;
	result.code = DEF_ERR_OUT_OF_MEMORY;
	result.string = DEF_ERR_OUT_OF_MEMORY_STR;
	return result;

	nintendo_api_failed:
	free(utf16_dir_path);
	free(utf16_path);
	utf16_dir_path = NULL;
	utf16_path = NULL;
	FSFILE_Close(handle);
	FSUSER_CloseArchive(archive);
	result.string = DEF_ERR_NINTENDO_RETURNED_NOT_SUCCESS_STR;
	return result;
}

Result_with_string Util_file_load_from_file(std::string file_name, std::string dir_path, uint8_t** read_data, int max_size)
{
	uint32_t read_size = 0;
	return Util_file_load_from_file_with_range_internal(std::move(file_name), std::move(dir_path), read_data, max_size, 0, &read_size);
}

Result_with_string Util_file_load_from_file(std::string file_name, std::string dir_path, uint8_t** read_data, int max_size, uint32_t* read_size)
{
	return Util_file_load_from_file_with_range_internal(std::move(file_name), std::move(dir_path), read_data, max_size, 0, read_size);
}

Result_with_string Util_file_load_from_file_with_range(std::string file_name, std::string dir_path, uint8_t** read_data, int max_size, uint64_t read_offset, uint32_t* read_size)
{
	return Util_file_load_from_file_with_range_internal(std::move(file_name), std::move(dir_path), read_data, max_size, read_offset, read_size);
}

Result_with_string Util_file_load_from_rom(std::string file_name, std::string dir_path, uint8_t** read_data, int max_size)
{
	uint32_t read_size = 0;
	return Util_file_load_from_rom_internal(std::move(file_name), std::move(dir_path), read_data, max_size, &read_size);
}

Result_with_string Util_file_load_from_rom(std::string file_name, std::string dir_path, uint8_t** read_data, int max_size, uint32_t* read_size)
{
	return Util_file_load_from_rom_internal(std::move(file_name), std::move(dir_path), read_data, max_size, read_size);
}

Result_with_string Util_file_delete_file(std::string file_name, std::string dir_path)
{
	uint16_t* utf16_path = NULL;
	ssize_t utf_out_size = 0;
	std::string path = "";
	FS_Archive archive = 0;
	Result_with_string result;

	if(file_name == "" || dir_path == "")
		goto invalid_arg;

	path = dir_path + file_name;
	utf16_path = (uint16_t*)malloc(4096 + 2);
	if(!utf16_path)
		goto out_of_memory;

	utf_out_size = utf8_to_utf16(utf16_path, (uint8_t*)path.c_str(), 2048);
	utf16_path[(utf_out_size < 0 ? 0 : utf_out_size)] = 0x00;//Add a null terminator.

	result.code = FSUSER_OpenArchive(&archive, ARCHIVE_SDMC, fsMakePath(PATH_EMPTY, ""));
	if (result.code != 0)
	{
		result.error_description = "[Error] FSUSER_OpenArchive() failed. ";
		goto nintendo_api_failed;
	}

	result.code = FSUSER_DeleteFile(archive, fsMakePath(PATH_UTF16, utf16_path));
	if (result.code != 0)
	{
		result.error_description = "[Error] FSUSER_DeleteFile() failed. ";
		goto nintendo_api_failed;
	}

	free(utf16_path);
	utf16_path = NULL;
	FSUSER_CloseArchive(archive);

	return result;

	invalid_arg:
	result.code = DEF_ERR_INVALID_ARG;
	result.string = DEF_ERR_INVALID_ARG_STR;
	return result;

	out_of_memory:
	result.code = DEF_ERR_OUT_OF_MEMORY;
	result.string = DEF_ERR_OUT_OF_MEMORY_STR;
	return result;

	nintendo_api_failed:
	free(utf16_path);
	utf16_path = NULL;
	FSUSER_CloseArchive(archive);
	result.string = DEF_ERR_NINTENDO_RETURNED_NOT_SUCCESS_STR;
	return result;
}

Result_with_string Util_file_check_file_size(std::string file_name, std::string dir_path, uint64_t* file_size)
{
	uint16_t* utf16_path = NULL;
	ssize_t utf_out_size = 0;
	std::string path = "";
	Handle handle = 0;
	FS_Archive archive = 0;
	Result_with_string result;

	if(file_name == "" || dir_path == "" || !file_size)
		goto invalid_arg;

	path = dir_path + file_name;
	utf16_path = (uint16_t*)malloc(4096 + 2);
	if(!utf16_path)
		goto out_of_memory;

	utf_out_size = utf8_to_utf16(utf16_path, (uint8_t*)path.c_str(), 2048);
	utf16_path[(utf_out_size < 0 ? 0 : utf_out_size)] = 0x00;//Add a null terminator.

	result.code = FSUSER_OpenArchive(&archive, ARCHIVE_SDMC, fsMakePath(PATH_EMPTY, ""));
	if (result.code != 0)
	{
		result.error_description = "[Error] FSUSER_OpenArchive() failed. ";
		goto nintendo_api_failed;
	}

	result.code = FSUSER_OpenFile(&handle, archive, fsMakePath(PATH_UTF16, utf16_path), FS_OPEN_READ, FS_ATTRIBUTE_ARCHIVE);
	if (result.code != 0)
	{
		result.error_description = "[Error] FSUSER_OpenFile() failed. ";
		goto nintendo_api_failed;
	}

	result.code = FSFILE_GetSize(handle, file_size);
	if (result.code != 0)
	{
		result.error_description = "[Error] FSFILE_GetSize() failed. ";
		goto nintendo_api_failed;
	}

	free(utf16_path);
	utf16_path = NULL;
	FSFILE_Close(handle);
	FSUSER_CloseArchive(archive);

	return result;

	invalid_arg:
	result.code = DEF_ERR_INVALID_ARG;
	result.string = DEF_ERR_INVALID_ARG_STR;
	return result;

	out_of_memory:
	result.code = DEF_ERR_OUT_OF_MEMORY;
	result.string = DEF_ERR_OUT_OF_MEMORY_STR;
	return result;

	nintendo_api_failed:
	free(utf16_path);
	utf16_path = NULL;
	FSFILE_Close(handle);
	FSUSER_CloseArchive(archive);
	result.string = DEF_ERR_NINTENDO_RETURNED_NOT_SUCCESS_STR;
	return result;
}

Result_with_string Util_file_check_file_exist(std::string file_name, std::string dir_path)
{
	uint16_t* utf16_path = NULL;
	ssize_t utf_out_size = 0;
	std::string path = "";
	Handle handle = 0;
	FS_Archive archive = 0;
	Result_with_string result;

	if(file_name == "" || dir_path == "")
		goto invalid_arg;

	path = dir_path + file_name;
	utf16_path = (uint16_t*)malloc(4096 + 2);
	if(!utf16_path)
		goto out_of_memory;

	utf_out_size = utf8_to_utf16(utf16_path, (uint8_t*)path.c_str(), 2048);
	utf16_path[(utf_out_size < 0 ? 0 : utf_out_size)] = 0x00;//Add a null terminator.

	result.code = FSUSER_OpenArchive(&archive, ARCHIVE_SDMC, fsMakePath(PATH_EMPTY, ""));
	if (result.code != 0)
	{
		result.error_description = "[Error] FSUSER_OpenArchive() failed. ";
		goto nintendo_api_failed;
	}

	result.code = FSUSER_OpenFile(&handle, archive, fsMakePath(PATH_UTF16, utf16_path), FS_OPEN_READ, FS_ATTRIBUTE_ARCHIVE);
	if (result.code != 0)
	{
		result.error_description = "[Error] FSUSER_OpenFile() failed. ";
		goto nintendo_api_failed;
	}

	FSFILE_Close(handle);
	FSUSER_CloseArchive(archive);

	free(utf16_path);
	utf16_path = NULL;
	return result;

	invalid_arg:
	result.code = DEF_ERR_INVALID_ARG;
	result.string = DEF_ERR_INVALID_ARG_STR;
	return result;

	out_of_memory:
	result.code = DEF_ERR_OUT_OF_MEMORY;
	result.string = DEF_ERR_OUT_OF_MEMORY_STR;
	return result;

	nintendo_api_failed:
	free(utf16_path);
	utf16_path = NULL;
	FSFILE_Close(handle);
	FSUSER_CloseArchive(archive);
	result.string = DEF_ERR_NINTENDO_RETURNED_NOT_SUCCESS_STR;
	return result;
}

uint32_t Util_file_read_dir(Util_str* dir_path, uint32_t* detected, Util_str* file_name, Expl_file_type* type, uint32_t array_length)
{
	uint16_t* utf16_dir_path = NULL;
	uint32_t count = 0;
	uint32_t read_entry = 0;
	uint32_t read_entry_count = 1;
	uint32_t result = DEF_ERR_OTHER;
	char* utf8_file_name = NULL;
	ssize_t utf_out_size = 0;
	FS_DirectoryEntry fs_entry = { 0,};
	Handle handle = 0;
	FS_Archive archive = 0;

	if(!Util_str_has_data(dir_path) || !detected || !file_name || !type)
		goto invalid_arg;

	for(uint32_t i = 0; i < array_length; i++)
	{
		if(Util_str_init(&file_name[i]) != DEF_SUCCESS)
			goto out_of_memory;

		type[i] = EXPL_FILE_TYPE_NONE;
	}
	*detected = 0;

	utf16_dir_path = (uint16_t*)malloc(4096 + 2);
	utf8_file_name = (char*)malloc(256 + 1);
	if(!utf16_dir_path || !utf8_file_name)
		goto out_of_memory;

	utf_out_size = utf8_to_utf16(utf16_dir_path, (uint8_t*)dir_path->buffer, 2048);
	utf16_dir_path[(utf_out_size < 0 ? 0 : utf_out_size)] = 0x00;//Add a null terminator.

	result = FSUSER_OpenArchive(&archive, ARCHIVE_SDMC, fsMakePath(PATH_EMPTY, ""));
	if (result != DEF_SUCCESS)
	{
		DEF_LOG_RESULT(FSUSER_OpenArchive, false, result);
		goto nintendo_api_failed;
	}

	result = FSUSER_OpenDirectory(&handle, archive, fsMakePath(PATH_UTF16, utf16_dir_path));
	if (result != DEF_SUCCESS)
	{
		DEF_LOG_RESULT(FSUSER_OpenDirectory, false, result);
		goto nintendo_api_failed;
	}

	while (true)
	{
		if(count >= array_length)
			goto out_of_memory;

		result = FSDIR_Read(handle, &read_entry, read_entry_count, (FS_DirectoryEntry*)&fs_entry);
		if(result != DEF_SUCCESS)
		{
			DEF_LOG_RESULT(FSDIR_Read, false, result);
			goto nintendo_api_failed;
		}

		if (read_entry == 0)
			break;

		utf_out_size = utf16_to_utf8((uint8_t*)utf8_file_name, fs_entry.name, 256);
		utf8_file_name[(utf_out_size < 0 ? 0 : utf_out_size)] = 0x00;//Add a null terminator.

		if(Util_str_set(&file_name[count], utf8_file_name) != DEF_SUCCESS)
			goto out_of_memory;

		if (fs_entry.attributes & FS_ATTRIBUTE_HIDDEN)
			type[count] = (type[count] | EXPL_FILE_TYPE_HIDDEN);
		if (fs_entry.attributes & FS_ATTRIBUTE_DIRECTORY)
			type[count] = (type[count] | EXPL_FILE_TYPE_DIR);
		if (fs_entry.attributes & FS_ATTRIBUTE_ARCHIVE)
			type[count] = (type[count] | EXPL_FILE_TYPE_FILE);
		if (fs_entry.attributes & FS_ATTRIBUTE_READ_ONLY)
			type[count] = (type[count] | EXPL_FILE_TYPE_READ_ONLY);

		count++;
		*detected = count;
	}

	free(utf8_file_name);
	free(utf16_dir_path);
	utf8_file_name = NULL;
	utf16_dir_path = NULL;
	FSDIR_Close(handle);
	FSUSER_CloseArchive(archive);
	return DEF_SUCCESS;

	invalid_arg:
	return DEF_ERR_INVALID_ARG;

	out_of_memory:
	free(utf8_file_name);
	free(utf16_dir_path);
	utf8_file_name = NULL;
	utf16_dir_path = NULL;
	FSDIR_Close(handle);
	FSUSER_CloseArchive(archive);
	for(uint32_t i = 0; i < array_length; i++)
		Util_str_free(&file_name[i]);

	return DEF_ERR_OUT_OF_MEMORY;

	nintendo_api_failed:
	free(utf8_file_name);
	free(utf16_dir_path);
	utf8_file_name = NULL;
	utf16_dir_path = NULL;
	FSDIR_Close(handle);
	FSUSER_CloseArchive(archive);
	for(uint32_t i = 0; i < array_length; i++)
		Util_str_free(&file_name[i]);

	return result;
}

static Result_with_string Util_file_load_from_file_with_range_internal(std::string&& file_name, std::string&& dir_path, uint8_t** read_data, int max_size, uint64_t read_offset, uint32_t* read_size)
{
	uint16_t* utf16_path = NULL;
	uint32_t max_read_size = 0;
	uint64_t file_size = 0;
	ssize_t utf_out_size = 0;
	std::string path = "";
	Handle handle = 0;
	FS_Archive archive = 0;
	Result_with_string result;

	if(file_name == "" || dir_path == "" || !read_data || max_size <= 0 || !read_size)
		goto invalid_arg;

	path = dir_path + file_name;
	utf16_path = (uint16_t*)malloc(4096 + 2);
	if(!utf16_path)
		goto out_of_memory;

	utf_out_size = utf8_to_utf16(utf16_path, (uint8_t*)path.c_str(), 2048);
	utf16_path[(utf_out_size < 0 ? 0 : utf_out_size)] = 0x00;//Add a null terminator.

	result.code = FSUSER_OpenArchive(&archive, ARCHIVE_SDMC, fsMakePath(PATH_EMPTY, ""));
	if (result.code != 0)
	{
		result.error_description = "[Error] FSUSER_OpenArchive() failed. ";
		goto nintendo_api_failed;
	}

	result.code = FSUSER_OpenFile(&handle, archive, fsMakePath(PATH_UTF16, utf16_path), FS_OPEN_READ, FS_ATTRIBUTE_ARCHIVE);
	if (result.code != 0)
	{
		result.error_description = "[Error] FSUSER_OpenFile() failed. ";
		goto nintendo_api_failed;
	}

	result.code = FSFILE_GetSize(handle, &file_size);
	if (result.code != 0)
	{
		result.error_description = "[Error] FSFILE_GetSize() failed. ";
		goto nintendo_api_failed;
	}

	max_read_size = (file_size - read_offset) > (uint64_t)max_size ? max_size : (file_size - read_offset);
	Util_safe_linear_free(*read_data);
	*read_data = (uint8_t*)Util_safe_linear_alloc(max_read_size + 1);
	if(!*read_data)
		goto out_of_memory;

	result.code = FSFILE_Read(handle, read_size, read_offset, *read_data, max_read_size);
	if (result.code != 0)
	{
		result.error_description = "[Error] FSFILE_Read() failed. ";
		goto nintendo_api_failed;
	}

	(*read_data)[*read_size] = 0x00;//Add a null terminator.

	free(utf16_path);
	utf16_path = NULL;
	FSFILE_Close(handle);
	FSUSER_CloseArchive(archive);

	return result;

	invalid_arg:
	result.code = DEF_ERR_INVALID_ARG;
	result.string = DEF_ERR_INVALID_ARG_STR;
	return result;

	out_of_memory:
	free(utf16_path);
	Util_safe_linear_free(*read_data);
	utf16_path = NULL;
	*read_data = NULL;
	FSFILE_Close(handle);
	FSUSER_CloseArchive(archive);
	result.code = DEF_ERR_OUT_OF_MEMORY;
	result.string = DEF_ERR_OUT_OF_MEMORY_STR;
	return result;

	nintendo_api_failed:
	free(utf16_path);
	Util_safe_linear_free(*read_data);
	utf16_path = NULL;
	*read_data = NULL;
	FSFILE_Close(handle);
	FSUSER_CloseArchive(archive);
	result.string = DEF_ERR_NINTENDO_RETURNED_NOT_SUCCESS_STR;
	return result;
}

static Result_with_string Util_file_load_from_rom_internal(std::string&& file_name, std::string&& dir_path, uint8_t** read_data, int max_size, uint32_t* read_size)
{
	size_t max_read_size = 0;
	uint64_t file_size = 0;
	std::string path = "";
	FILE* handle = 0;
	Result_with_string result;

	if(file_name == "" || dir_path == "" || !read_data || max_size <= 0 || !read_size)
		goto invalid_arg;

	path = dir_path + file_name;
	handle = fopen(path.c_str(), "rb");
	if (!handle)
	{
		result.error_description = "[Error] fopen() failed. ";
		goto fopen_failed;
	}

	if(fseek(handle, 0, SEEK_END) != 0)
	{
		result.error_description = "[Error] fseek() failed. ";
		goto fopen_failed;
	}

	file_size = ftell(handle);
	if(file_size <= 0)
	{
		result.error_description = "[Error] ftell() failed. ";
		goto fopen_failed;
	}

	rewind(handle);

	max_read_size = file_size > (uint64_t)max_size ? max_size : file_size;
	Util_safe_linear_free(*read_data);
	*read_data = (uint8_t*)Util_safe_linear_alloc(max_read_size + 1);
	if(!*read_data)
		goto out_of_memory;

	*read_size = fread(*read_data, 1, max_read_size, handle);
	if(*read_size != max_read_size)
	{
		result.error_description = "[Error] fread() failed. ";
		goto fopen_failed;
	}

	(*read_data)[*read_size] = 0x00;//Add a null terminator.

	fclose(handle);

	return result;

	invalid_arg:
	result.code = DEF_ERR_INVALID_ARG;
	result.string = DEF_ERR_INVALID_ARG_STR;
	return result;

	out_of_memory:
	fclose(handle);
	result.code = DEF_ERR_OUT_OF_MEMORY;
	result.string = DEF_ERR_OUT_OF_MEMORY_STR;
	return result;

	fopen_failed:
	Util_safe_linear_free(*read_data);
	*read_data = NULL;
	if(handle)
		fclose(handle);

	result.code = DEF_ERR_OTHER;
	result.string = DEF_ERR_OTHER_STR;
	return result;
}
