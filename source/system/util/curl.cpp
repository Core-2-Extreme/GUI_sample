#include "definitions.hpp"

#if DEF_ENABLE_CURL_API
#include <stdbool.h>
#include <stdint.h>

#include "system/types.hpp"

#include "system/util/error_types.h"
#include "system/util/file.hpp"
#include "system/util/util.hpp"

extern "C"
{
#include "system/util/log.h"
#include "system/util/str.h"
}

extern "C"
{
#include "curl/curl.h"
}

//Include myself.
#include "system/util/curl.hpp"

bool util_curl_init = false;
uint32_t* util_curl_buffer = NULL;

typedef struct
{
	uint8_t* data;
	uint32_t max_size;
	uint32_t current_size;
	uint32_t* used_size;
	const char* file_name;
	const char* dir_path;
} Http_data;

typedef struct
{
	uint8_t* data;
	uint32_t upload_size;
	uint32_t offset;
	uint32_t* uploaded_size;
	void* user_data;
	int32_t (*callback)(void* buffer, uint32_t max_size, void* user_data);
} Upload_data;


static size_t Util_curl_write_callback(char* input_data, size_t size, size_t nmemb, void* user_data);
static size_t Util_curl_save_callback(char* input_data, size_t size, size_t nmemb, void* user_data);
static size_t Util_curl_read_callback(char* output_buffer, size_t size, size_t nitems, void* user_data);
//We can't get rid of this "int" because library uses "int" type as args.
static int Util_curl_seek_callback(void* user_data, curl_off_t offset, int origin);
static uint32_t Util_curl_request(CURL** curl_handle, const char* url, CURLoption method, uint16_t max_redirect, Upload_data* upload_data);
static uint32_t Util_curl_get_request(CURL** curl_handle, const char* url, uint16_t max_redirect);
static uint32_t Util_curl_post_request(CURL** curl_handle, const char* url, Upload_data* upload_data, uint16_t max_redirect);
static void Util_curl_get_response(CURL** curl_handle, uint16_t* status_code, Util_str* new_url);
static uint32_t Util_curl_download_data(CURL** curl_handle, Http_data* http_data);
static uint32_t Util_curl_save_data(CURL** curl_handle, Http_data* http_data);
static void Util_curl_close(CURL** curl_handle);
static uint32_t Util_curl_post_and_dl_data_internal(const char* url, uint8_t* post_data, uint32_t post_size, uint8_t** dl_data,
uint32_t max_dl_size, uint32_t* downloaded_size, uint32_t* uploaded_size, uint16_t* status_code, uint16_t max_redirect, Util_str* last_url,
int32_t (*read_callback)(void* buffer, uint32_t max_size, void* user_data), void* user_data);
static uint32_t Util_curl_post_and_save_data_internal(const char* url, uint8_t* post_data, uint32_t post_size, uint32_t buffer_size,
uint32_t* downloaded_size, uint32_t* uploaded_size, uint16_t* status_code, uint16_t max_redirect, Util_str* last_url, const char* dir_path,
const char* file_name, int32_t (*read_callback)(void* buffer, uint32_t max_size, void* user_data), void* user_data);


uint32_t Util_curl_init(uint32_t buffer_size)
{
	uint32_t result = DEF_ERR_OTHER;

	if(util_curl_init)
		goto already_inited;

	if(buffer_size < 0)
		goto invalid_arg;

	util_curl_buffer = (uint32_t*)__real_memalign(0x1000, buffer_size);
	if(!util_curl_buffer)
		goto out_of_memory;

	result = socInit(util_curl_buffer, buffer_size);
	if(result != DEF_SUCCESS)
	{
		DEF_LOG_RESULT(socInit, false, result);
		goto nintendo_api_failed;
	}

	util_curl_init = true;
	return result;

	already_inited:
	return DEF_ERR_ALREADY_INITIALIZED;

	invalid_arg:
	return DEF_ERR_INVALID_ARG;

	out_of_memory:
	return DEF_ERR_OUT_OF_MEMORY;

	nintendo_api_failed:
	free(util_curl_buffer);
	util_curl_buffer = NULL;
	return result;
}

void Util_curl_exit(void)
{
	if(!util_curl_init)
		return;

	util_curl_init = false;
	socExit();
	free(util_curl_buffer);
	util_curl_buffer = NULL;
}

uint32_t Util_curl_dl_data(const char* url, uint8_t** data, uint32_t max_size, uint32_t* downloaded_size,
uint16_t* status_code, uint16_t max_redirect, Util_str* last_url)
{
	uint32_t dummy = 0;
	uint32_t result = DEF_ERR_OTHER;
	Http_data http_data = { 0, };
	CURL* curl_handle = NULL;

	if(!util_curl_init)
		goto not_inited;

	//downloaded_size, status_code and last_url can be NULL.
	if(!url || !data || max_size == 0)
		goto invalid_arg;

	if(downloaded_size)
		*downloaded_size = 0;
	if(status_code)
		*status_code = 0;

	if(last_url)
	{
		result = Util_str_init(last_url);
		if(result != DEF_SUCCESS)
		{
			DEF_LOG_RESULT(Util_str_init, false, result);
			goto api_failed;
		}

		result = Util_str_set(last_url, url);
		if(result != DEF_SUCCESS)
		{
			DEF_LOG_RESULT(Util_str_set, false, result);
			goto api_failed;
		}
	}

	curl_handle = curl_easy_init();
	if(!curl_handle)
	{
		DEF_LOG_RESULT(curl_easy_init, false, DEF_ERR_CURL_RETURNED_NOT_SUCCESS);
		goto curl_api_failed;
	}

	http_data.max_size = max_size;
	http_data.used_size = (downloaded_size ? downloaded_size : &dummy);
	result = Util_curl_get_request(&curl_handle, url, max_redirect);
	if(result != DEF_SUCCESS)
		goto api_failed;

	result = Util_curl_download_data(&curl_handle, &http_data);
	if(result != DEF_SUCCESS)
		goto api_failed;

	*data = (uint8_t*)http_data.data;

	Util_curl_get_response(&curl_handle, status_code, last_url);

	Util_curl_close(&curl_handle);

	return DEF_SUCCESS;

	not_inited:
	return DEF_ERR_NOT_INITIALIZED;

	invalid_arg:
	return DEF_ERR_INVALID_ARG;

	curl_api_failed:
	return DEF_ERR_CURL_RETURNED_NOT_SUCCESS;

	api_failed:
	if(last_url)
		Util_str_free(last_url);

	return result;
}

uint32_t Util_curl_save_data(const char* url, uint32_t buffer_size, uint32_t* downloaded_size, uint16_t* status_code,
uint16_t max_redirect, Util_str* last_url, const char* dir_path, const char* file_name)
{
	uint32_t dummy = 0;
	uint32_t result = DEF_ERR_OTHER;
	Http_data http_data = { 0, };
	CURL* curl_handle = NULL;

	if(!util_curl_init)
		goto not_inited;

	//downloaded_size, status_code and last_url can be NULL.
	if(!url || buffer_size == 0 || !dir_path || !file_name)
		goto invalid_arg;

	if(downloaded_size)
		*downloaded_size = 0;
	if(status_code)
		*status_code = 0;

	if(last_url)
	{
		result = Util_str_init(last_url);
		if(result != DEF_SUCCESS)
		{
			DEF_LOG_RESULT(Util_str_init, false, result);
			goto api_failed;
		}

		result = Util_str_set(last_url, url);
		if(result != DEF_SUCCESS)
		{
			DEF_LOG_RESULT(Util_str_set, false, result);
			goto api_failed;
		}
	}

	curl_handle = curl_easy_init();
	if(!curl_handle)
	{
		DEF_LOG_RESULT(curl_easy_init, false, DEF_ERR_CURL_RETURNED_NOT_SUCCESS);
		goto curl_api_failed;
	}

	http_data.max_size = buffer_size;
	http_data.used_size = (downloaded_size ? downloaded_size : &dummy);
	http_data.dir_path = dir_path;
	http_data.file_name = file_name;
	result = Util_curl_get_request(&curl_handle, url, max_redirect);
	if(result != 0)
		goto api_failed;

	result = Util_curl_save_data(&curl_handle, &http_data);
	if(result != 0)
		goto api_failed;

	Util_curl_get_response(&curl_handle, status_code, last_url);

	Util_curl_close(&curl_handle);

	return DEF_SUCCESS;

	not_inited:
	return DEF_ERR_NOT_INITIALIZED;

	invalid_arg:
	return DEF_ERR_INVALID_ARG;

	curl_api_failed:
	return DEF_ERR_CURL_RETURNED_NOT_SUCCESS;

	api_failed:
	if(last_url)
		Util_str_free(last_url);

	return result;
}

uint32_t Util_curl_post_and_dl_data(const char* url, uint8_t* post_data, uint32_t post_size, uint8_t** dl_data, uint32_t max_dl_size,
uint32_t* downloaded_size, uint32_t* uploaded_size, uint16_t* status_code, uint16_t max_redirect, Util_str* last_url)
{
	return Util_curl_post_and_dl_data_internal(url, post_data, post_size, dl_data, max_dl_size, downloaded_size,
	uploaded_size, status_code, max_redirect, last_url, NULL, NULL);
}

uint32_t Util_curl_post_and_dl_data_with_callback(const char* url, uint8_t* post_data, uint32_t post_size, uint8_t** dl_data, uint32_t max_dl_size,
uint32_t* downloaded_size, uint32_t* uploaded_size, uint16_t* status_code, uint16_t max_redirect, Util_str* last_url,
int32_t (*read_callback)(void* buffer, uint32_t max_size, void* user_data), void* user_data)
{
	return Util_curl_post_and_dl_data_internal(url, NULL, 0, dl_data, max_dl_size, downloaded_size,
	uploaded_size, status_code, max_redirect, last_url, read_callback, user_data);
}

uint32_t Util_curl_post_and_save_data(const char* url, uint8_t* post_data, uint32_t post_size, uint32_t buffer_size, uint32_t* downloaded_size,
uint32_t* uploaded_size, uint16_t* status_code, Util_str* last_url, uint16_t max_redirect, const char* dir_path, const char* file_name)
{
	return Util_curl_post_and_save_data_internal(url, post_data, post_size, buffer_size, downloaded_size,
	uploaded_size, status_code, max_redirect, last_url, dir_path, file_name, NULL, NULL);
}

uint32_t Util_curl_post_and_save_data_with_callback(const char* url, uint8_t* post_data, uint32_t post_size, uint32_t buffer_size,
uint32_t* downloaded_size, uint32_t* uploaded_size, uint16_t* status_code, uint16_t max_redirect, Util_str* last_url, const char* dir_path,
const char* file_name, int32_t (*read_callback)(void* buffer, uint32_t max_size, void* user_data), void* user_data)
{
	return Util_curl_post_and_save_data_internal(url, NULL, 0, buffer_size, downloaded_size, uploaded_size,
	status_code, max_redirect, last_url, dir_path, file_name, read_callback, user_data);
}

static size_t Util_curl_write_callback(char* input_data, size_t size, size_t nmemb, void* user_data)
{
	uint32_t buffer_size = 0;
	uint32_t input_size = (size * nmemb);
	uint32_t allocated_size = 0;
	Http_data* http_data = (Http_data*)user_data;

	if(!user_data)
		return -1;

	//Out of memory
	if((*http_data->used_size) + input_size > http_data->max_size)
		goto error;

	//Need to realloc memory
	if((*http_data->used_size) + input_size > http_data->current_size)
	{
		buffer_size = ((http_data->max_size > (http_data->current_size + 0x40000)) ? (http_data->current_size + 0x40000) : http_data->max_size);

		http_data->data = (uint8_t*)Util_safe_linear_realloc(http_data->data, buffer_size);
		if (!http_data->data)
			goto error;

		allocated_size = buffer_size - http_data->current_size;
		memset(http_data->data + http_data->current_size, 0x0, allocated_size);
		http_data->current_size = buffer_size;
	}

	memcpy(http_data->data + (*http_data->used_size), input_data, input_size);
	*http_data->used_size += input_size;

	return input_size;

	error:
	Util_safe_linear_free(http_data->data);
	http_data->data = NULL;
	*http_data->used_size = 0;
	http_data->current_size = 0;
	http_data->max_size = 0;
	return -1;
}

static size_t Util_curl_save_callback(char* input_data, size_t size, size_t nmemb, void* user_data)
{
	uint32_t input_size = (size * nmemb);
	uint32_t input_data_offset = 0;
	uint32_t result = DEF_ERR_OTHER;
	Http_data* http_data = (Http_data*)user_data;

	if(!user_data)
		return -1;

	*http_data->used_size += input_size;

	//If libcurl buffer size is bigger than our buffer size, save it directly without buffering
	if(input_size > http_data->max_size)
	{
		result = Util_file_save_to_file(http_data->file_name, http_data->dir_path, (uint8_t*)input_data, input_size, false);
		http_data->current_size = 0;
		if(result == DEF_SUCCESS)
			return (size * nmemb);
		else
			return -1;
	}
	//If we run out of buffer, save it
	else if(http_data->current_size + input_size > http_data->max_size)
	{
		memcpy(http_data->data + http_data->current_size, input_data, http_data->max_size - http_data->current_size);
		input_data_offset = http_data->max_size - http_data->current_size;

		http_data->current_size += input_data_offset;
		input_size = input_size - input_data_offset;

		result = Util_file_save_to_file(http_data->file_name, http_data->dir_path, http_data->data, http_data->current_size, false);
		http_data->current_size = 0;
	}
	else
		result = DEF_SUCCESS;

	memcpy(http_data->data + http_data->current_size, input_data + input_data_offset, input_size);
	http_data->current_size += input_size;

	if(result == DEF_SUCCESS)
		return (size * nmemb);
	else
		return -1;
}

static size_t Util_curl_read_callback(char* output_buffer, size_t size, size_t nitems, void* user_data)
{
	uint32_t buffer_size = (size * nitems);
	int32_t copy_size = 0;
	int32_t read_size = 0;
	Upload_data* upload_data = (Upload_data*)user_data;

	if(!user_data)
		return -1;

	//if call back is provided, use it
	if(upload_data->callback)
	{
		read_size = upload_data->callback(output_buffer, buffer_size, upload_data->user_data);
		if(upload_data->uploaded_size)
			*upload_data->uploaded_size += read_size;

		return read_size;
	}

	//EOF
	if(upload_data->upload_size - upload_data->offset <= 0)
		return 0;

	//if buffer size is smaller than available post data size
	if(buffer_size < upload_data->upload_size - upload_data->offset)
		copy_size = buffer_size;
	else
		copy_size = upload_data->upload_size - upload_data->offset;

	memcpy(output_buffer, upload_data->data + upload_data->offset, copy_size);
	if(upload_data->uploaded_size)
		*upload_data->uploaded_size += copy_size;

	upload_data->offset += copy_size;

	return copy_size;
}

//We can't get rid of this "int" because library uses "int" type as args.
static int Util_curl_seek_callback(void* user_data, curl_off_t offset, int origin)
{
	Upload_data* upload_data = (Upload_data*)user_data;

	if(!user_data)
		return CURL_SEEKFUNC_FAIL;

	if(origin == SEEK_SET)
		upload_data->offset = offset;
	else
		return CURL_SEEKFUNC_FAIL;

	return CURL_SEEKFUNC_OK;
}

static uint32_t Util_curl_request(CURL** curl_handle, const char* url, CURLoption method, uint16_t max_redirect, Upload_data* upload_data)
{
	uint32_t result = DEF_ERR_OTHER;

	result = curl_easy_setopt(*curl_handle, CURLOPT_URL, url);
	if (result != CURLE_OK)
	{
		DEF_LOG_RESULT(curl_easy_setopt_URL, false, result);
		goto curl_api_failed;
	}

	if(method == CURLOPT_HTTPPOST)
		result = curl_easy_setopt(*curl_handle, CURLOPT_HTTPPOST, 1);
	else
		result = curl_easy_setopt(*curl_handle, CURLOPT_HTTPGET, 1);

	if (result != CURLE_OK)
	{
		if(method == CURLOPT_HTTPPOST)
			DEF_LOG_RESULT(curl_easy_setopt_HTTPPOST, false, result);
		else
			DEF_LOG_RESULT(curl_easy_setopt_HTTPGET, false, result);

		goto curl_api_failed;
	}

	result = curl_easy_setopt(*curl_handle, CURLOPT_SSL_VERIFYPEER, 0);
	if (result != CURLE_OK)
	{
		DEF_LOG_RESULT(curl_easy_setopt_SSL_VERIFYPEER, false, result);
		goto curl_api_failed;
	}

	result = curl_easy_setopt(*curl_handle, CURLOPT_FOLLOWLOCATION, (max_redirect > 0));
	if (result != CURLE_OK)
	{
		DEF_LOG_RESULT(curl_easy_setopt_FOLLOWLOCATION, false, result);
		goto curl_api_failed;
	}

	result = curl_easy_setopt(*curl_handle, CURLOPT_MAXREDIRS, max_redirect);
	if (result != CURLE_OK)
	{
		DEF_LOG_RESULT(curl_easy_setopt_MAXREDIRS, false, result);
		goto curl_api_failed;
	}

	result = curl_easy_setopt(*curl_handle, CURLOPT_USERAGENT, DEF_HTTP_USER_AGENT);
	if (result != CURLE_OK)
	{
		DEF_LOG_RESULT(curl_easy_setopt_USERAGENT, false, result);
		goto curl_api_failed;
	}

	result = curl_easy_setopt(*curl_handle, CURLOPT_BUFFERSIZE, 1024 * 128);
	if (result != CURLE_OK)
	{
		DEF_LOG_RESULT(curl_easy_setopt_BUFFERSIZE, false, result);
		goto curl_api_failed;
	}

	if(method == CURLOPT_HTTPPOST)
	{
		result = curl_easy_setopt(*curl_handle, CURLOPT_POSTFIELDS, NULL);
		if (result != CURLE_OK)
		{
			DEF_LOG_RESULT(curl_easy_setopt_POSTFIELDS, false, result);
			goto curl_api_failed;
		}

		/*result = curl_easy_setopt(curl_handle, CURLOPT_POSTFIELDSIZE, upload_data->upload_size);
		if (result != CURLE_OK)
		{
			DEF_LOG_RESULT(curl_easy_setopt_POSTFIELDSIZE, false, result);
			goto curl_api_failed;
		}*/

		result = curl_easy_setopt(*curl_handle, CURLOPT_READFUNCTION, Util_curl_read_callback);
		if (result != CURLE_OK)
		{
			DEF_LOG_RESULT(curl_easy_setopt_READFUNCTION, false, result);
			goto curl_api_failed;
		}

		result = curl_easy_setopt(*curl_handle, CURLOPT_READDATA, (void*)upload_data);
		if (result != CURLE_OK)
		{
			DEF_LOG_RESULT(curl_easy_setopt_READDATA, false, result);
			goto curl_api_failed;
		}

		result = curl_easy_setopt(*curl_handle, CURLOPT_SEEKFUNCTION, Util_curl_seek_callback);
		if (result != CURLE_OK)
		{
			DEF_LOG_RESULT(curl_easy_setopt_SEEKFUNCTION, false, result);
			goto curl_api_failed;
		}

		result = curl_easy_setopt(*curl_handle, CURLOPT_SEEKDATA, (void*)upload_data);
		if (result != CURLE_OK)
		{
			DEF_LOG_RESULT(curl_easy_setopt_SEEKDATA, false, result);
			goto curl_api_failed;
		}
	}

	return result;

	curl_api_failed:
	Util_curl_close(curl_handle);
	return DEF_ERR_CURL_RETURNED_NOT_SUCCESS;
}

static uint32_t Util_curl_get_request(CURL** curl_handle, const char* url, uint16_t max_redirect)
{
	return Util_curl_request(curl_handle, url, CURLOPT_HTTPGET, max_redirect, NULL);
}

static uint32_t Util_curl_post_request(CURL** curl_handle, const char* url, Upload_data* upload_data, uint16_t max_redirect)
{
	return Util_curl_request(curl_handle, url, CURLOPT_HTTPPOST, max_redirect, upload_data);
}

static void Util_curl_get_response(CURL** curl_handle, uint16_t* status_code, Util_str* new_url)
{
	uint32_t result = DEF_ERR_OTHER;

	if(status_code)
	{
		uint32_t out = 0;

		result = curl_easy_getinfo(*curl_handle, CURLINFO_RESPONSE_CODE, &out);
		if(result == CURLE_OK)
			*status_code = (uint16_t)out;
		else
			*status_code = 0;
	}

	if(new_url)
	{
		char moved_url[4096] = { 0, };

		result = curl_easy_getinfo(*curl_handle, CURLINFO_EFFECTIVE_URL, moved_url);
		if(result == CURLE_OK)
			Util_str_set(new_url, moved_url);
	}
}

static uint32_t Util_curl_download_data(CURL** curl_handle, Http_data* http_data)
{
	uint32_t result = DEF_ERR_OTHER;
	char error_message[4096] = { 0, };

	result = curl_easy_setopt(*curl_handle, CURLOPT_WRITEFUNCTION, Util_curl_write_callback);
	if (result != CURLE_OK)
	{
		DEF_LOG_RESULT(curl_easy_setopt_WRITEFUNCTION, false, result);
		goto curl_api_failed;
	}

	result = curl_easy_setopt(*curl_handle, CURLOPT_WRITEDATA, (void*)http_data);
	if (result != CURLE_OK)
	{
		DEF_LOG_RESULT(curl_easy_setopt_WRITEDATA, false, result);
		goto curl_api_failed;
	}

	result = curl_easy_setopt(*curl_handle, CURLOPT_ERRORBUFFER, error_message);
	if (result != CURLE_OK)
	{
		DEF_LOG_RESULT(curl_easy_setopt_ERRORBUFFER, false, result);
		goto curl_api_failed;
	}

	result = curl_easy_perform(*curl_handle);
	if (result != CURLE_OK)
	{
		DEF_LOG_RESULT(curl_easy_perform, false, result);
		DEF_LOG_STRING(error_message);
		goto curl_api_failed;
	}
	return DEF_SUCCESS;

	curl_api_failed:
	Util_curl_close(curl_handle);
	Util_safe_linear_free(http_data->data);
	http_data->data = NULL;
	*http_data->used_size = 0;
	http_data->current_size = 0;
	http_data->max_size = 0;
	return DEF_ERR_CURL_RETURNED_NOT_SUCCESS;
}

static void Util_curl_close(CURL** curl_handle)
{
	if(!curl_handle)
		return;

	if(*curl_handle)
		curl_easy_cleanup(*curl_handle);

	*curl_handle = NULL;
}

static uint32_t Util_curl_save_data(CURL** curl_handle, Http_data* http_data)
{
	uint32_t result = DEF_ERR_OTHER;
	char error_message[4096] = { 0, };

	http_data->data = (uint8_t*)Util_safe_linear_alloc(http_data->max_size);
	if(!http_data->data)
		goto out_of_memory;

	result = curl_easy_setopt(*curl_handle, CURLOPT_WRITEFUNCTION, Util_curl_save_callback);
	if (result != CURLE_OK)
	{
		DEF_LOG_RESULT(curl_easy_setopt_WRITEFUNCTION, false, result);
		goto curl_api_failed;
	}

	result = curl_easy_setopt(*curl_handle, CURLOPT_WRITEDATA, (void*)http_data);
	if (result != CURLE_OK)
	{
		DEF_LOG_RESULT(curl_easy_setopt_WRITEDATA, false, result);
		goto curl_api_failed;
	}

	result = curl_easy_setopt(*curl_handle, CURLOPT_ERRORBUFFER, error_message);
	if (result != CURLE_OK)
	{
		DEF_LOG_RESULT(curl_easy_setopt_ERRORBUFFER, false, result);
		goto curl_api_failed;
	}

	Util_file_delete_file(http_data->file_name, http_data->dir_path);

	result = curl_easy_perform(*curl_handle);
	if (result != CURLE_OK)
	{
		DEF_LOG_RESULT(curl_easy_perform, false, result);
		DEF_LOG_STRING(error_message);
		goto curl_api_failed;
	}

	Util_file_save_to_file(http_data->file_name, http_data->dir_path, http_data->data, http_data->current_size, false);

	Util_safe_linear_free(http_data->data);
	http_data->data = NULL;
	return result;

	out_of_memory:
	Util_curl_close(curl_handle);
	*http_data->used_size = 0;
	http_data->current_size = 0;
	http_data->max_size = 0;
	return DEF_ERR_OUT_OF_LINEAR_MEMORY;

	curl_api_failed:
	Util_curl_close(curl_handle);
	Util_safe_linear_free(http_data->data);
	http_data->data = NULL;
	*http_data->used_size = 0;
	http_data->current_size = 0;
	http_data->max_size = 0;
	return DEF_ERR_CURL_RETURNED_NOT_SUCCESS;
}

static uint32_t Util_curl_post_and_dl_data_internal(const char* url, uint8_t* post_data, uint32_t post_size, uint8_t** dl_data,
uint32_t max_dl_size, uint32_t* downloaded_size, uint32_t* uploaded_size, uint16_t* status_code, uint16_t max_redirect, Util_str* last_url,
int32_t (*read_callback)(void* buffer, uint32_t max_size, void* user_data), void* user_data)
{
	uint32_t dummy = 0;
	uint32_t result = DEF_ERR_OTHER;
	Http_data http_data = { 0, };
	Upload_data upload_data = { 0, };
	CURL* curl_handle = NULL;

	if(!util_curl_init)
		goto not_inited;

	//downloaded_size, uploaded_size, status_code, last_url and user_data can be NULL.
	if(!url || !dl_data || max_dl_size == 0)
		goto invalid_arg;

	if(read_callback)
	{
		//If callback is provided, post_data and post_size must be NULL and 0.
		if(post_data || post_size != 0)
			goto invalid_arg;
	}
	else
	{
		//If callback is NOT provided, post_data and post_size must NOT be NULL and 0.
		if(!post_data || post_size == 0)
			goto invalid_arg;
	}

	if(downloaded_size)
		*downloaded_size = 0;
	if(uploaded_size)
		*uploaded_size = 0;
	if(status_code)
		*status_code = 0;

	if(last_url)
	{
		result = Util_str_init(last_url);
		if(result != DEF_SUCCESS)
		{
			DEF_LOG_RESULT(Util_str_init, false, result);
			goto api_failed;
		}

		result = Util_str_set(last_url, url);
		if(result != DEF_SUCCESS)
		{
			DEF_LOG_RESULT(Util_str_set, false, result);
			goto api_failed;
		}
	}

	curl_handle = curl_easy_init();
	if(!curl_handle)
	{
		DEF_LOG_RESULT(curl_easy_init, false, DEF_ERR_CURL_RETURNED_NOT_SUCCESS);
		goto curl_api_failed;
	}

	http_data.max_size = max_dl_size;
	http_data.used_size = (downloaded_size ? downloaded_size : &dummy);
	upload_data.uploaded_size = uploaded_size;
	if(read_callback)
	{
		upload_data.callback = read_callback;
		upload_data.user_data = user_data;
	}
	else
	{
		upload_data.upload_size = post_size;
		upload_data.data = post_data;
	}
	result = Util_curl_post_request(&curl_handle, url, &upload_data, max_redirect);
	if(result != DEF_SUCCESS)
		goto api_failed;

	result = Util_curl_download_data(&curl_handle, &http_data);
	if(result != DEF_SUCCESS)
		goto api_failed;

	*dl_data = (uint8_t*)http_data.data;

	Util_curl_get_response(&curl_handle, status_code, last_url);

	Util_curl_close(&curl_handle);

	return DEF_SUCCESS;

	not_inited:
	return DEF_ERR_NOT_INITIALIZED;

	invalid_arg:
	return DEF_ERR_INVALID_ARG;

	curl_api_failed:
	return DEF_ERR_CURL_RETURNED_NOT_SUCCESS;

	api_failed:
	if(last_url)
		Util_str_free(last_url);

	return result;
}

static uint32_t Util_curl_post_and_save_data_internal(const char* url, uint8_t* post_data, uint32_t post_size, uint32_t buffer_size,
uint32_t* downloaded_size, uint32_t* uploaded_size, uint16_t* status_code, uint16_t max_redirect, Util_str* last_url, const char* dir_path,
const char* file_name, int32_t (*read_callback)(void* buffer, uint32_t max_size, void* user_data), void* user_data)
{
	uint32_t dummy = 0;
	uint32_t result = DEF_ERR_OTHER;
	Http_data http_data = { 0, };
	Upload_data upload_data = { 0, };
	CURL* curl_handle = NULL;

	if(!util_curl_init)
		goto not_inited;

	//downloaded_size, uploaded_size, status_code, last_url and user_data can be NULL.
	if(!url || buffer_size == 0 || !dir_path || !file_name)
		goto invalid_arg;

	if(read_callback)
	{
		//If callback is provided, post_data and post_size must be NULL and 0.
		if(post_data || post_size != 0)
			goto invalid_arg;
	}
	else
	{
		//If callback is NOT provided, post_data and post_size must NOT be NULL and 0.
		if(!post_data || post_size == 0)
			goto invalid_arg;
	}

	if(downloaded_size)
		*downloaded_size = 0;
	if(uploaded_size)
		*uploaded_size = 0;
	if(status_code)
		*status_code = 0;

	if(last_url)
	{
		result = Util_str_init(last_url);
		if(result != DEF_SUCCESS)
		{
			DEF_LOG_RESULT(Util_str_init, false, result);
			goto api_failed;
		}

		result = Util_str_set(last_url, url);
		if(result != DEF_SUCCESS)
		{
			DEF_LOG_RESULT(Util_str_set, false, result);
			goto api_failed;
		}
	}

	curl_handle = curl_easy_init();
	if(!curl_handle)
	{
		DEF_LOG_RESULT(curl_easy_init, false, DEF_ERR_CURL_RETURNED_NOT_SUCCESS);
		goto curl_api_failed;
	}

	http_data.max_size = buffer_size;
	http_data.used_size = (downloaded_size ? downloaded_size : &dummy);
	http_data.dir_path = dir_path;
	http_data.file_name = file_name;
	upload_data.uploaded_size = uploaded_size;
	if(read_callback)
	{
		upload_data.callback = read_callback;
		upload_data.user_data = user_data;
	}
	else
	{
		upload_data.upload_size = post_size;
		upload_data.data = post_data;
	}

	result = Util_curl_post_request(&curl_handle, url, &upload_data, max_redirect);
	if(result != DEF_SUCCESS)
		goto api_failed;

	result = Util_curl_save_data(&curl_handle, &http_data);
	if(result != DEF_SUCCESS)
		goto api_failed;

	Util_curl_get_response(&curl_handle, status_code, last_url);

	Util_curl_close(&curl_handle);

	return DEF_SUCCESS;

	not_inited:
	return DEF_ERR_NOT_INITIALIZED;

	invalid_arg:
	return DEF_ERR_INVALID_ARG;

	curl_api_failed:
	return DEF_ERR_CURL_RETURNED_NOT_SUCCESS;

	api_failed:
	if(last_url)
		Util_str_free(last_url);

	return result;
}

#endif
