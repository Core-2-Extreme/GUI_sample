extern "C"
{
#include "system/util/httpc.h"
}

#if DEF_HTTPC_API_ENABLE
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "3ds.h"

extern "C"
{
#include "system/util/err_types.h"
#include "system/util/file.h"
#include "system/util/log.h"
#include "system/util/str.h"
#include "system/util/util_c.h"
}

#include "system/menu.hpp"


bool util_httpc_init = false;


static uint32_t Util_httpc_request(httpcContext* httpc_context, const char* url, HTTPC_RequestMethod method, uint8_t* post_data, uint32_t post_data_size);
static uint32_t Util_httpc_get_request(httpcContext* httpc_context, const char* url);
static uint32_t Util_httpc_post_request(httpcContext* httpc_context, const char* url, uint8_t* post_data, uint32_t post_data_size);
static void Util_httpc_get_response(httpcContext* httpc_context, uint16_t* status_code, Str_data* new_url, bool* redirected);
static uint32_t Util_httpc_download_data(httpcContext* httpc_context, uint8_t** data, uint32_t max_size, uint32_t* downloaded_size);
static void Util_httpc_close(httpcContext* httpc_context);
static uint32_t Util_httpc_save_data(httpcContext* httpc_context, uint32_t buffer_size, uint32_t* downloaded_size, const char* dir_path, const char* file_name);


uint32_t Util_httpc_init(uint32_t buffer_size)
{
	uint32_t result = DEF_ERR_OTHER;

	if(util_httpc_init)
		goto already_inited;

	if(buffer_size == 0)
		goto invalid_arg;

	result = httpcInit(buffer_size);
	if(result != DEF_SUCCESS)
	{
		DEF_LOG_RESULT(httpcInit, false, result);
		goto nintendo_api_failed;
	}

	util_httpc_init = true;
	return result;

	already_inited:
	return DEF_ERR_ALREADY_INITIALIZED;

	invalid_arg:
	return DEF_ERR_INVALID_ARG;

	nintendo_api_failed:
	return result;
}

void Util_httpc_exit(void)
{
	if(!util_httpc_init)
		return;

	util_httpc_init = false;
	httpcExit();
}

uint32_t Util_httpc_dl_data(const char* url, uint8_t** data, uint32_t max_size, uint32_t* downloaded_size,
uint16_t* status_code, uint16_t max_redirect, Str_data* last_url)
{
	uint16_t redirected = 0;
	uint32_t result = DEF_ERR_OTHER;
	httpcContext httpc_context = { 0, };
	Str_data current_url = { 0, };

	if(!util_httpc_init)
		goto not_inited;

	//downloaded_size, status_code and last_url can be NULL.
	if(!url || !data || max_size == 0)
		goto invalid_arg;

	if(downloaded_size)
		*downloaded_size = 0;
	if(status_code)
		*status_code = 0;

	result = Util_str_init(&current_url);
	if(result != DEF_SUCCESS)
	{
		DEF_LOG_RESULT(Util_str_init, false, result);
		goto api_failed;
	}

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

	result = Util_str_set(&current_url, url);
	if(result != DEF_SUCCESS)
	{
		DEF_LOG_RESULT(Util_str_set, false, result);
		goto api_failed;
	}

	while (true)
	{
		bool is_redirected = false;

		result = Util_httpc_get_request(&httpc_context, current_url.buffer);
		if(result != DEF_SUCCESS)
			goto api_failed;

		Util_httpc_get_response(&httpc_context, status_code, &current_url, &is_redirected);

		if (is_redirected && max_redirect > redirected)
		{
			if(last_url)
			{
				result = Util_str_set(last_url, current_url.buffer);
				if(result != DEF_SUCCESS)
				{
					DEF_LOG_RESULT(Util_str_set, false, result);
					goto api_failed;
				}
			}
			redirected++;
		}
		else
			is_redirected = false;

		if (!is_redirected)
		{
			result = Util_httpc_download_data(&httpc_context, data, max_size, downloaded_size);
			if(result != DEF_SUCCESS)
				goto api_failed;
		}

		Util_httpc_close(&httpc_context);

		if (!is_redirected)
			break;
	}

	return DEF_SUCCESS;

	not_inited:
	return DEF_ERR_NOT_INITIALIZED;

	invalid_arg:
	return DEF_ERR_INVALID_ARG;

	api_failed:
	Util_str_free(&current_url);
	if(last_url)
		Util_str_free(last_url);

	return result;
}

uint32_t Util_httpc_save_data(const char* url, uint32_t buffer_size, uint32_t* downloaded_size, uint16_t* status_code,
uint16_t max_redirect, Str_data* last_url, const char* dir_path, const char* file_name)
{
	uint16_t redirected = 0;
	uint32_t result = DEF_ERR_OTHER;
	httpcContext httpc_context = { 0, };
	Str_data current_url = { 0, };

	if(!util_httpc_init)
		goto not_inited;

	//downloaded_size, status_code and last_url can be NULL.
	if(!url || buffer_size == 0 || !dir_path || !file_name)
		goto invalid_arg;

	if(downloaded_size)
		*downloaded_size = 0;
	if(status_code)
		*status_code = 0;

	result = Util_str_init(&current_url);
	if(result != DEF_SUCCESS)
	{
		DEF_LOG_RESULT(Util_str_init, false, result);
		goto api_failed;
	}

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

	result = Util_str_set(&current_url, url);
	if(result != DEF_SUCCESS)
	{
		DEF_LOG_RESULT(Util_str_set, false, result);
		goto api_failed;
	}

	while (true)
	{
		bool is_redirected = false;

		result = Util_httpc_get_request(&httpc_context, current_url.buffer);
		if(result != DEF_SUCCESS)
			goto api_failed;

		Util_httpc_get_response(&httpc_context, status_code, &current_url, &is_redirected);

		if (is_redirected && max_redirect > redirected)
		{
			if(last_url)
			{
				result = Util_str_set(last_url, current_url.buffer);
				if(result != DEF_SUCCESS)
				{
					DEF_LOG_RESULT(Util_str_set, false, result);
					goto api_failed;
				}
			}
			redirected++;
		}
		else
			is_redirected = false;

		if (!is_redirected)
		{
			result = Util_httpc_save_data(&httpc_context, buffer_size, downloaded_size, dir_path, file_name);
			if(result != DEF_SUCCESS)
				goto api_failed;
		}

		Util_httpc_close(&httpc_context);

		if (!is_redirected)
			break;
	}

	return DEF_SUCCESS;

	not_inited:
	return DEF_ERR_NOT_INITIALIZED;

	invalid_arg:
	return DEF_ERR_INVALID_ARG;

	api_failed:
	Util_str_free(&current_url);
	if(last_url)
		Util_str_free(last_url);

	return result;
}

uint32_t Util_httpc_post_and_dl_data(const char* url, uint8_t* post_data, uint32_t post_size, uint8_t** dl_data, uint32_t max_dl_size,
uint32_t* downloaded_size, uint16_t* status_code, uint16_t max_redirect, Str_data* last_url)
{
	bool post = true;
	uint16_t redirected = 0;
	uint32_t result = DEF_ERR_OTHER;
	httpcContext httpc_context = { 0, };
	Str_data current_url = { 0, };

	if(!util_httpc_init)
		goto not_inited;

	//downloaded_size, status_code and last_url can be NULL.
	if(!url || !post_data || post_size == 0 || !dl_data || max_dl_size == 0)
		goto invalid_arg;

	if(downloaded_size)
		*downloaded_size = 0;
	if(status_code)
		*status_code = 0;

	result = Util_str_init(&current_url);
	if(result != DEF_SUCCESS)
	{
		DEF_LOG_RESULT(Util_str_init, false, result);
		goto api_failed;
	}

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

	result = Util_str_set(&current_url, url);
	if(result != DEF_SUCCESS)
	{
		DEF_LOG_RESULT(Util_str_set, false, result);
		goto api_failed;
	}

	while (true)
	{
		bool is_redirected = false;

		if(post)
		{
			result = Util_httpc_post_request(&httpc_context, current_url.buffer, post_data, post_size);
			post = false;
		}
		else
			result = Util_httpc_get_request(&httpc_context, current_url.buffer);

		if(result != DEF_SUCCESS)
			goto api_failed;

		Util_httpc_get_response(&httpc_context, status_code, &current_url, &is_redirected);

		if (is_redirected && max_redirect > redirected)
		{
			if(last_url)
			{
				result = Util_str_set(last_url, current_url.buffer);
				if(result != DEF_SUCCESS)
				{
					DEF_LOG_RESULT(Util_str_set, false, result);
					goto api_failed;
				}
			}
			redirected++;
		}
		else
			is_redirected = false;

		if (!is_redirected)
		{
			result = Util_httpc_download_data(&httpc_context, dl_data, max_dl_size, downloaded_size);
			if(result != DEF_SUCCESS)
				goto api_failed;
		}

		Util_httpc_close(&httpc_context);

		if (!is_redirected)
			break;
	}

	return DEF_SUCCESS;

	not_inited:
	return DEF_ERR_NOT_INITIALIZED;

	invalid_arg:
	return DEF_ERR_INVALID_ARG;

	api_failed:
	Util_str_free(&current_url);
	if(last_url)
		Util_str_free(last_url);

	return result;
}

uint32_t Util_httpc_post_and_save_data(const char* url, uint8_t* post_data, uint32_t post_size, uint32_t buffer_size, uint32_t* downloaded_size,
uint16_t* status_code, uint16_t max_redirect, Str_data* last_url, const char* dir_path, const char* file_name)
{
	bool post = true;
	uint16_t redirected = 0;
	uint32_t result = DEF_ERR_OTHER;
	httpcContext httpc_context = { 0, };
	Str_data current_url = { 0, };

	if(!util_httpc_init)
		goto not_inited;

	//downloaded_size, status_code and last_url can be NULL.
	if(!url || !post_data || post_size == 0 || buffer_size == 0 || !dir_path || !file_name)
		goto invalid_arg;

	if(downloaded_size)
		*downloaded_size = 0;
	if(status_code)
		*status_code = 0;

	result = Util_str_init(&current_url);
	if(result != DEF_SUCCESS)
	{
		DEF_LOG_RESULT(Util_str_init, false, result);
		goto api_failed;
	}

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

	result = Util_str_set(&current_url, url);
	if(result != DEF_SUCCESS)
	{
		DEF_LOG_RESULT(Util_str_set, false, result);
		goto api_failed;
	}

	while (true)
	{
		bool is_redirected = false;

		if(post)
		{
			result = Util_httpc_post_request(&httpc_context, current_url.buffer, post_data, post_size);
			post = false;
		}
		else
			result = Util_httpc_get_request(&httpc_context, current_url.buffer);

		if(result != DEF_SUCCESS)
			goto api_failed;

		Util_httpc_get_response(&httpc_context, status_code, &current_url, &is_redirected);

		if (is_redirected && max_redirect > redirected)
		{
			if(last_url)
			{
				result = Util_str_set(last_url, current_url.buffer);
				if(result != DEF_SUCCESS)
				{
					DEF_LOG_RESULT(Util_str_set, false, result);
					goto api_failed;
				}
			}
			redirected++;
		}
		else
			is_redirected = false;

		if (!is_redirected)
		{
			result = Util_httpc_save_data(&httpc_context, buffer_size, downloaded_size, dir_path, file_name);
			if(result != DEF_SUCCESS)
				goto api_failed;
		}

		Util_httpc_close(&httpc_context);

		if (!is_redirected)
			break;
	}

	return DEF_SUCCESS;

	not_inited:
	result = DEF_ERR_NOT_INITIALIZED;
	return DEF_ERR_NOT_INITIALIZED;

	invalid_arg:
	return DEF_ERR_INVALID_ARG;

	api_failed:
	Util_str_free(&current_url);
	if(last_url)
		Util_str_free(last_url);

	return result;
}

static uint32_t Util_httpc_request(httpcContext* httpc_context, const char* url, HTTPC_RequestMethod method, uint8_t* post_data, uint32_t post_data_size)
{
	uint32_t result = DEF_ERR_OTHER;

	if(method == HTTPC_METHOD_POST)
		result = httpcOpenContext(httpc_context, HTTPC_METHOD_POST, url, 0);
	else
		result = httpcOpenContext(httpc_context, HTTPC_METHOD_GET, url, 0);

	if (result != DEF_SUCCESS)
	{
		DEF_LOG_RESULT(httpcOpenContext, false, result);
		goto nintendo_api_failed;
	}

	result = httpcSetSSLOpt(httpc_context, SSLCOPT_DisableVerify);
	if (result != DEF_SUCCESS)
	{
		DEF_LOG_RESULT(httpcSetSSLOpt, false, result);
		goto nintendo_api_failed;
	}

	result = httpcSetKeepAlive(httpc_context, HTTPC_KEEPALIVE_ENABLED);
	if (result != DEF_SUCCESS)
	{
		DEF_LOG_RESULT(httpcSetKeepAlive, false, result);
		goto nintendo_api_failed;
	}

	result = httpcAddRequestHeaderField(httpc_context, "Connection", "Keep-Alive");
	if (result != DEF_SUCCESS)
	{
		DEF_LOG_RESULT(httpcAddRequestHeaderField, false, result);
		goto nintendo_api_failed;
	}

	result = httpcAddRequestHeaderField(httpc_context, "User-Agent", DEF_MENU_HTTP_USER_AGENT);
	if (result != DEF_SUCCESS)
	{
		DEF_LOG_RESULT(httpcAddRequestHeaderField, false, result);
		goto nintendo_api_failed;
	}

	if(method == HTTPC_METHOD_POST)
	{
		result = httpcAddPostDataRaw(httpc_context, (uint32_t*)post_data, post_data_size);
		if (result != DEF_SUCCESS)
		{
			DEF_LOG_RESULT(httpcAddPostDataRaw, false, result);
			goto nintendo_api_failed;
		}
	}

	result = httpcBeginRequest(httpc_context);
	if (result != DEF_SUCCESS)
	{
		DEF_LOG_RESULT(httpcBeginRequest, false, result);
		goto nintendo_api_failed;
	}

	return DEF_SUCCESS;

	nintendo_api_failed:
	Util_httpc_close(httpc_context);
	return result;
}

static uint32_t Util_httpc_get_request(httpcContext* httpc_context, const char* url)
{
	return Util_httpc_request(httpc_context, url, HTTPC_METHOD_GET, NULL, 0);
}

static uint32_t Util_httpc_post_request(httpcContext* httpc_context, const char* url, uint8_t* post_data, uint32_t post_data_size)
{
	return Util_httpc_request(httpc_context, url, HTTPC_METHOD_POST, post_data, post_data_size);
}

static void Util_httpc_get_response(httpcContext* httpc_context, uint16_t* status_code, Str_data* new_url, bool* redirected)
{
	uint32_t result = DEF_ERR_OTHER;
	char moved_url[4096] = { 0, };

	*redirected = false;

	if(status_code)
	{
		uint32_t out = 0;

		result = httpcGetResponseStatusCode(httpc_context, &out);
		if(result == DEF_SUCCESS)
			*status_code = (uint16_t)out;
		else
			*status_code = 0;
	}

	result = httpcGetResponseHeader(httpc_context, "location", moved_url, 0x1000);
	if (result == DEF_SUCCESS)
	{
		if(Util_str_set(new_url, moved_url) == DEF_SUCCESS)
			*redirected = true;
	}
	else
	{
		result = httpcGetResponseHeader(httpc_context, "Location", moved_url, 0x1000);
		if (result == DEF_SUCCESS)
		{
			if(Util_str_set(new_url, moved_url) == DEF_SUCCESS)
				*redirected = true;
		}
	}
}

static uint32_t Util_httpc_download_data(httpcContext* httpc_context, uint8_t** data, uint32_t max_size, uint32_t* downloaded_size)
{
	uint8_t* new_buffer = NULL;
	uint32_t dl_size = 0;
	uint32_t remain_buffer_size = 0;
	uint32_t buffer_offset = 0;
	uint32_t buffer_size = 0;
	uint32_t result = DEF_ERR_OTHER;

	buffer_size = ((max_size > 0x40000) ? 0x40000 : max_size);
	free(*data);
	*data = (uint8_t*)linearAlloc(buffer_size);
	if (!*data)
		goto out_of_memory;

	if(downloaded_size)
		*downloaded_size = 0;

	remain_buffer_size = buffer_size;
	memset(*data, 0x0, remain_buffer_size);

	while(true)
	{
		result = httpcDownloadData(httpc_context, (*data) + buffer_offset, remain_buffer_size, &dl_size);
		if(downloaded_size)
			*downloaded_size += dl_size;

		buffer_offset += dl_size;
		if (result != DEF_SUCCESS)
		{
			if(result == HTTPC_RESULTCODE_DOWNLOADPENDING)
			{
				if(buffer_size >= max_size)
					goto out_of_memory;

				buffer_size = ((max_size > (buffer_size + 0x40000)) ? (buffer_size + 0x40000) : max_size);
				new_buffer = (uint8_t*)linearRealloc(*data, buffer_size);
				remain_buffer_size = buffer_size - buffer_offset;
				if(!new_buffer)
					goto out_of_memory;

				*data = new_buffer;
				memset((*data) + buffer_offset, 0x0, remain_buffer_size);
			}
			else
			{
				DEF_LOG_RESULT(httpcDownloadData, false, result);
				goto nintendo_api_failed;
			}
		}
		else
			break;
	}

	return DEF_SUCCESS;

	out_of_memory:
	free(*data);
	*data = NULL;
	Util_httpc_close(httpc_context);
	return DEF_ERR_OUT_OF_MEMORY;

	nintendo_api_failed:
	free(*data);
	*data = NULL;
	Util_httpc_close(httpc_context);
	return result;
}

static void Util_httpc_close(httpcContext* httpc_context)
{
	if(httpc_context)
		httpcCloseContext(httpc_context);

	httpc_context = NULL;
}

static uint32_t Util_httpc_save_data(httpcContext* httpc_context, uint32_t buffer_size, uint32_t* downloaded_size, const char* dir_path, const char* file_name)
{
	bool first = true;
	uint8_t* cache = NULL;
	uint32_t dl_size = 0;
	uint32_t result = DEF_ERR_OTHER;

	cache = (uint8_t*)linearAlloc(buffer_size);
	if (!cache)
		goto out_of_memory;

	if(downloaded_size)
		*downloaded_size = 0;

	while(true)
	{
		result = httpcDownloadData(httpc_context, cache, buffer_size, &dl_size);
		if(downloaded_size)
			*downloaded_size += dl_size;

		if (result != DEF_SUCCESS)
		{
			if(result == HTTPC_RESULTCODE_DOWNLOADPENDING)
			{
				result = Util_file_save_to_file(file_name, dir_path, cache, dl_size, first);
				first = false;
				if(result != DEF_SUCCESS)
				{
					DEF_LOG_RESULT(Util_file_save_to_file, false, result);
					goto api_failed;
				}
			}
			else
			{
				Util_file_delete_file(file_name, dir_path);

				DEF_LOG_RESULT(httpcDownloadData, false, result);
				goto nintendo_api_failed;
			}
		}
		else
		{
			if(dl_size > 0)
			{
				result = Util_file_save_to_file(file_name, dir_path, cache, dl_size, first);
				first = false;
				if(result != DEF_SUCCESS)
				{
					DEF_LOG_RESULT(Util_file_save_to_file, false, result);
					goto api_failed;
				}
			}
			break;
		}
	}

	free(cache);
	cache = NULL;
	return DEF_SUCCESS;

	out_of_memory:
	Util_httpc_close(httpc_context);
	return DEF_ERR_OUT_OF_MEMORY;

	nintendo_api_failed:
	api_failed:
	free(cache);
	cache = NULL;
	Util_httpc_close(httpc_context);
	return result;
}

#endif
