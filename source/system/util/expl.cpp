extern "C"
{
#include "system/util/expl.h"
}

#if DEF_EXPL_API_ENABLE
#include <ctype.h>
#include <stdbool.h>
#include <stdint.h>

#include <citro2d.h>

#include "system/menu.hpp"
#include "system/variables.hpp"

extern "C"
{
#include "system/draw/draw.h"
#include "system/util/err_types.h"
#include "system/util/file.h"
#include "system/util/hid.h"
#include "system/util/log.h"
#include "system/util/str.h"
#include "system/util/util_c.h"
}

extern "C"
{
#define DEF_EXPL_SORT_TYPE_UNDEFINED		(uint8_t)(0)	//Unknown.
#define DEF_EXPL_SORT_TYPE_SPECIAL_CHAR		(uint8_t)(1)	//Other than 0-9,a-z,A-Z.
#define DEF_EXPL_SORT_TYPE_NUMBER			(uint8_t)(2)	//0-9.
#define DEF_EXPL_SORT_TYPE_ALPHABET			(uint8_t)(3)	//a-z or A-Z.


typedef struct
{
	uint32_t size[DEF_EXPL_MAX_FILES];
	Str_data name[DEF_EXPL_MAX_FILES];
	Expl_file_type type[DEF_EXPL_MAX_FILES];
} Util_expl_file;

typedef struct
{
	uint32_t size;
	Str_data name;
	Expl_file_type type;
} Util_expl_file_compare;


static void Util_expl_generate_file_type_string(Expl_file_type type, Str_data* type_string);
static int Util_expl_compare_name(const void* a, const void* b);
static void Util_expl_read_dir_callback(void);


void (*util_expl_callback)(Str_data*, Str_data*) = NULL;
void (*util_expl_cancel_callback)(void) = NULL;
bool util_expl_read_dir_request = false;
bool util_expl_show_flag = false;
bool util_expl_scroll_mode = false;
bool util_expl_init = false;
uint32_t util_expl_num_of_file = 0;
uint32_t util_expl_check_file_size_index = 0;
double util_expl_y_offset = 0.0;
double util_expl_selected_file_num = 0.0;
Str_data util_expl_current_dir = { 0, };
Draw_image_data util_expl_file_button[16] = { 0, };
Util_expl_file util_expl_files = { 0, };


uint32_t Util_expl_init(void)
{
	uint32_t result = DEF_ERR_OTHER;

	if(util_expl_init)
		goto already_inited;

	util_expl_callback = NULL;
	util_expl_cancel_callback = NULL;
	util_expl_read_dir_request = false;
	util_expl_show_flag = false;
	util_expl_scroll_mode = false;
	util_expl_num_of_file = 0;
	util_expl_check_file_size_index = 0;
	util_expl_y_offset = 0.0;
	util_expl_selected_file_num = 0.0;

	result = Util_str_init(&util_expl_current_dir);
	if(result != DEF_SUCCESS)
	{
		DEF_LOG_RESULT(Util_str_init, false, result);
		goto other;
	}

	result = Util_str_set(&util_expl_current_dir, "/");
	if(result != DEF_SUCCESS)
	{
		DEF_LOG_RESULT(Util_str_set, false, result);
		goto other;
	}

	for(uint32_t i = 0; i < DEF_EXPL_MAX_FILES; i++)
	{
		util_expl_files.size[i] = 0;
		util_expl_files.type[i] = EXPL_FILE_TYPE_NONE;

		result = Util_str_init(&util_expl_files.name[i]);
		if(result != DEF_SUCCESS)
		{
			DEF_LOG_RESULT(Util_str_init, false, result);
			goto other;
		}
	}

	for(uint8_t i = 0; i < 16; i++)
		util_expl_file_button[i].c2d = var_square_image[0];

	if(!Menu_add_worker_thread_callback(Util_expl_read_dir_callback))
	{
		result = DEF_ERR_OTHER;
		DEF_LOG_RESULT(Menu_add_worker_thread_callback, false, result);
		goto other;
	}

	util_expl_init = true;
	return DEF_SUCCESS;

	already_inited:
	return DEF_ERR_ALREADY_INITIALIZED;

	other:
	Util_expl_exit();
	return result;
}

void Util_expl_exit(void)
{
	if(!util_expl_init)
		return;

	util_expl_init = false;
	Menu_remove_worker_thread_callback(Util_expl_read_dir_callback);

	Util_str_free(&util_expl_current_dir);
	for(uint32_t i = 0; i < DEF_EXPL_MAX_FILES; i++)
		Util_str_free(&util_expl_files.name[i]);
}

uint32_t Util_expl_query_current_dir(Str_data* dir_name)
{
	uint32_t result = DEF_ERR_OTHER;

	if(!util_expl_init)
		goto not_inited;

	if(!dir_name)
		goto invalid_arg;

	result = Util_str_init(dir_name);
	if(result != DEF_SUCCESS)
	{
		DEF_LOG_RESULT(Util_str_init, false, result);
		goto other;
	}

	//Copy directory name.
	result = Util_str_set(dir_name, util_expl_current_dir.buffer);
	if(result != DEF_SUCCESS)
	{
		DEF_LOG_RESULT(Util_str_set, false, result);
		goto other;
	}

	return DEF_SUCCESS;

	not_inited:
	return DEF_ERR_NOT_INITIALIZED;

	invalid_arg:
	return DEF_ERR_INVALID_ARG;

	other:
	return result;
}

uint32_t Util_expl_query_num_of_file(void)
{
	if(!util_expl_init)
		return 0;

	return util_expl_num_of_file;
}

uint32_t Util_expl_query_current_file_index(void)
{
	if(!util_expl_init)
		return DEF_EXPL_INVALID_INDEX;

	return (uint32_t)util_expl_selected_file_num + (uint32_t)util_expl_y_offset;
}

uint32_t Util_expl_query_file_name(uint32_t index, Str_data* file_name)
{
	uint32_t result = DEF_ERR_OTHER;

	if(!util_expl_init)
		goto not_inited;

	if(index >= DEF_EXPL_MAX_FILES || !file_name)
		goto invalid_arg;

	result = Util_str_init(file_name);
	if(result != DEF_SUCCESS)
	{
		DEF_LOG_RESULT(Util_str_init, false, result);
		goto other;
	}

	//Copy file name.
	result = Util_str_set(file_name, util_expl_files.name[index].buffer);
	if(result != DEF_SUCCESS)
	{
		DEF_LOG_RESULT(Util_str_set, false, result);
		goto other;
	}

	return DEF_SUCCESS;

	not_inited:
	return DEF_ERR_NOT_INITIALIZED;

	invalid_arg:
	return DEF_ERR_INVALID_ARG;

	other:
	return result;
}

uint32_t Util_expl_query_size(uint32_t index)
{
	if(!util_expl_init)
		return 0;
	else if (index < DEF_EXPL_MAX_FILES)
		return util_expl_files.size[index];
	else
		return 0;
}

Expl_file_type Util_expl_query_type(uint32_t index)
{
	if(!util_expl_init)
		return EXPL_FILE_TYPE_NONE;
	if (index < DEF_EXPL_MAX_FILES)
		return util_expl_files.type[index];
	else
		return EXPL_FILE_TYPE_NONE;
}

bool Util_expl_query_show_flag(void)
{
	if(!util_expl_init)
		return false;

	return util_expl_show_flag;
}

void Util_expl_set_callback(void (*callback)(Str_data*, Str_data*))
{
	if(!util_expl_init)
		return;

	util_expl_callback = callback;
}

void Util_expl_set_cancel_callback(void (*callback)(void))
{
	if(!util_expl_init)
		return;

	util_expl_cancel_callback = callback;
}

void Util_expl_set_current_dir(Str_data* dir_name)
{
	if(!util_expl_init)
		return;

	if(!Util_str_has_data(dir_name))
		return;

	Util_str_set(&util_expl_current_dir, dir_name->buffer);
	util_expl_read_dir_request = true;
}

void Util_expl_set_show_flag(bool flag)
{
	if(!util_expl_init)
		return;

	util_expl_show_flag = flag;
	if(flag)
		util_expl_read_dir_request = true;
}

void Util_expl_draw(void)
{
	Draw_image_data background = { 0, };
	uint32_t color = DEF_DRAW_BLACK;

	background.c2d = var_square_image[0];

	if(!util_expl_init)
	{
		Draw_texture(&background, DEF_DRAW_AQUA, 10.0, 20.0, 300.0, 190.0);
		Draw_c("Explorer api is not initialized.\nPress A to close.", 12.5, 30.0, 0.45, 0.45, DEF_DRAW_RED);
		return;
	}

	Draw_texture(&background, DEF_DRAW_AQUA, 10.0, 20.0, 300.0, 190.0);
	Draw_c("A : OK, B : Back, Y : Close, ↑↓→← : Move", 12.5, 185.0, 0.425, 0.425, DEF_DRAW_BLACK);
	Draw(&util_expl_current_dir, 12.5, 195.0, 0.45, 0.45, DEF_DRAW_BLACK);

	for (uint8_t i = 0; i < 16; i++)
	{
		Str_data message = { 0, };
		Str_data type = { 0, };
		uint32_t index = (i + (uint32_t)util_expl_y_offset);

		if(Util_str_init(&message) != DEF_SUCCESS
		|| Util_str_init(&type) != DEF_SUCCESS)
		{
			Util_str_free(&message);
			Util_str_free(&type);
			continue;
		}

		Util_expl_generate_file_type_string(util_expl_files.type[index], &type);

		Draw_texture(&util_expl_file_button[i], util_expl_file_button[i].selected ? DEF_DRAW_GREEN : DEF_DRAW_AQUA, 10, 20 + (i * 10), 290, 10);
		if(util_expl_files.type[index] & EXPL_FILE_TYPE_DIR)
			Util_str_format(&message, "%s (%s)", util_expl_files.name[index].buffer, type.buffer);
		else
		{
			float size = util_expl_files.size[index];

			if(size < 1000)
				Util_str_format(&message, "%s(%" PRIu32 "B) (%s)", util_expl_files.name[index].buffer, (uint32_t)size, type.buffer);
			else
			{
				size /= 1000.0;
				if(size < 1000)
					Util_str_format(&message, "%s(%.1fKB) (%s)", util_expl_files.name[index].buffer, size, type.buffer);
				else
				{
					size /= 1000.0;
					if(size < 1000)
						Util_str_format(&message, "%s(%.1fMB) (%s)", util_expl_files.name[index].buffer, size, type.buffer);
					else
					{
						size /= 1000.0;
						Util_str_format(&message, "%s(%.1fGB) (%s)", util_expl_files.name[index].buffer, size, type.buffer);
					}
				}
			}
		}
		Util_str_free(&type);

		Draw(&message, 12.5, 19 + (i * 10), 0.425, 0.425, i == (uint8_t)util_expl_selected_file_num ? DEF_DRAW_RED : color);
		Util_str_free(&message);
	}
}

void Util_expl_main(Hid_info key)
{
	if(!util_expl_init)
	{
		if (key.p_a)
		{
			util_expl_show_flag = false;
			var_need_reflesh = true;
		}
		return;
	}

	if (key.p_y)
	{
		if(util_expl_cancel_callback)
			util_expl_cancel_callback();

		util_expl_show_flag = false;
		var_need_reflesh = true;
	}
	else if (!util_expl_read_dir_request)
	{
		if(util_expl_scroll_mode)
		{
			if (key.touch_y_move > 0)
			{
				if ((util_expl_y_offset + (key.touch_y_move * var_scroll_speed / 8)) < util_expl_num_of_file - 15)
					util_expl_y_offset += (key.touch_y_move * var_scroll_speed / 8);

				var_need_reflesh = true;
			}
			else if (key.touch_y_move < 0)
			{
				if ((util_expl_y_offset + (key.touch_y_move * var_scroll_speed / 8)) > -1.0)
					util_expl_y_offset += (key.touch_y_move * var_scroll_speed / 8);

				var_need_reflesh = true;
			}
		}
		else
		{
			for (uint8_t i = 0; i < 16; i++)
			{
				if(Util_hid_is_pressed(key, util_expl_file_button[i]) && util_expl_num_of_file > (i + (uint32_t)util_expl_y_offset))
				{
					util_expl_file_button[i].selected = true;
					var_need_reflesh = true;
				}
				else if (key.p_a || (Util_hid_is_released(key, util_expl_file_button[i]) && util_expl_file_button[i].selected))
				{
					if (key.p_a || i == util_expl_selected_file_num)
					{
						bool is_root_dir = false;
						uint32_t selected_index = (util_expl_y_offset + util_expl_selected_file_num);
						Str_data dir = { 0, };

						Util_expl_query_current_dir(&dir);
						if(Util_str_has_data(&dir) && strcmp(dir.buffer, "/") == 0)
							is_root_dir = true;

						if (selected_index == 0 && !is_root_dir)
						{
							//Back to parent directory.
							char* last_slash_pos = strrchr(dir.buffer, '/');

							if(last_slash_pos)
							{
								//Remove last slash first.
								uint32_t new_length = (last_slash_pos - dir.buffer);

								Util_str_resize(&dir, new_length);

								last_slash_pos = strrchr(dir.buffer, '/');
								if(last_slash_pos)
								{
									//Then remove until next slash.
									new_length = (last_slash_pos - dir.buffer) + 1;
									Util_str_resize(&dir, new_length);
								}
							}
							else
								Util_str_set(&dir, "/");

							Util_str_set(&util_expl_current_dir, dir.buffer);
							util_expl_y_offset = 0.0;
							util_expl_selected_file_num = 0.0;
							util_expl_read_dir_request = true;
						}
						else if (util_expl_files.type[selected_index] & EXPL_FILE_TYPE_DIR)
						{
							//Go to selected sub directory.
							Util_str_format_append(&dir, "%s/", util_expl_files.name[selected_index].buffer);

							Util_str_set(&util_expl_current_dir, dir.buffer);
							util_expl_y_offset = 0.0;
							util_expl_selected_file_num = 0.0;
							util_expl_read_dir_request = true;
						}
						else
						{
							//Notify file selection.
							Str_data file = { 0, };

							Util_expl_query_file_name(selected_index, &file);

							if(Util_str_has_data(&file) && Util_str_has_data(&dir) && util_expl_callback)
								util_expl_callback(&file, &dir);

							Util_str_free(&file);
							util_expl_show_flag = false;
							var_need_reflesh = true;
						}

						Util_str_free(&dir);

						break;
					}
					else
					{
						if (util_expl_num_of_file > (i + (uint32_t)util_expl_y_offset))
							util_expl_selected_file_num = i;

						var_need_reflesh = true;
					}
				}
				else if(!Util_hid_is_held(key, util_expl_file_button[i]) && util_expl_file_button[i].selected)
				{
					util_expl_file_button[i].selected = false;
					util_expl_scroll_mode = true;
					var_need_reflesh = true;
				}
			}
		}
		if (key.p_b)
		{
			bool is_root_dir = false;
			Str_data dir = { 0, };

			Util_expl_query_current_dir(&dir);
			if(Util_str_has_data(&dir) && strcmp(dir.buffer, "/") == 0)
				is_root_dir = true;

			if (!is_root_dir)
			{
				if(Util_str_has_data(&dir))
				{
					//Back to parent directory.
					char* last_slash_pos = strrchr(dir.buffer, '/');

					if(last_slash_pos)
					{
						//Remove last slash first.
						uint32_t new_length = (last_slash_pos - dir.buffer);

						Util_str_resize(&dir, new_length);

						last_slash_pos = strrchr(dir.buffer, '/');
						if(last_slash_pos)
						{
							//Then remove until next slash.
							new_length = (last_slash_pos - dir.buffer) + 1;
							Util_str_resize(&dir, new_length);
						}
					}
					else
						Util_str_set(&dir, "/");
				}
				else
					Util_str_set(&dir, "/");

				Util_str_set(&util_expl_current_dir, dir.buffer);
				util_expl_y_offset = 0.0;
				util_expl_selected_file_num = 0.0;
				util_expl_read_dir_request = true;
				var_need_reflesh = true;
			}

			Util_str_free(&dir);
		}
		else if (key.p_d_down || key.h_d_down || key.p_c_down || key.h_c_down || key.p_d_right || key.h_d_right || key.p_c_right || key.h_c_right)
		{
			if ((util_expl_selected_file_num + 1.0) < 16.0 && (util_expl_selected_file_num + 1.0) < util_expl_num_of_file)
			{
				if (key.p_d_down || key.h_d_down || key.p_c_down || key.h_c_down)
					util_expl_selected_file_num += 0.125;
				else if (key.p_d_right || key.h_d_right || key.p_c_right || key.h_c_right)
					util_expl_selected_file_num += 1.0;
			}
			else if ((util_expl_y_offset + util_expl_selected_file_num + 1.0) < util_expl_num_of_file)
			{
				if (key.p_d_down || key.h_d_down || key.p_c_down || key.h_c_down)
					util_expl_y_offset += 0.125;
				else if (key.p_d_right || key.h_d_right || key.p_c_right || key.h_c_right)
					util_expl_y_offset += 1.0;
			}
			var_need_reflesh = true;
		}
		else if (key.p_d_up || key.h_d_up || key.p_c_up || key.h_c_up || key.p_d_left || key.h_d_left || key.p_c_left || key.h_c_left)
		{
			if ((util_expl_selected_file_num - 1.0) > -1.0)
			{
				if (key.p_d_up || key.h_d_up || key.p_c_up || key.h_c_up)
					util_expl_selected_file_num -= 0.125;
				else if (key.p_d_left || key.h_d_left || key.p_c_left || key.h_c_left)
					util_expl_selected_file_num -= 1.0;
			}
			else if ((util_expl_y_offset - 1.0) > -1.0)
			{
				if (key.p_d_up || key.h_d_up || key.p_c_up || key.h_c_up)
					util_expl_y_offset -= 0.125;
				else if (key.p_d_left || key.h_d_left || key.p_c_left || key.h_c_left)
					util_expl_y_offset -= 1.0;
			}
			var_need_reflesh = true;
		}

		if(!key.p_touch && !key.h_touch)
		{
			for(uint8_t i = 0; i < 16; i++)
			{
				if(util_expl_file_button[i].selected)
					var_need_reflesh = true;

				util_expl_file_button[i].selected = false;
			}
			util_expl_scroll_mode = false;
		}

		if (util_expl_selected_file_num <= -1)
			util_expl_selected_file_num = 0;
		else if (util_expl_selected_file_num >= 16)
			util_expl_selected_file_num = 15;
		else if (util_expl_selected_file_num >= util_expl_num_of_file)
			util_expl_selected_file_num = util_expl_num_of_file - 1;
		if (util_expl_y_offset <= -1)
			util_expl_y_offset = 0;
		else if (util_expl_y_offset + util_expl_selected_file_num >= util_expl_num_of_file)
			util_expl_y_offset = util_expl_num_of_file - 16;
	}
}

static void Util_expl_generate_file_type_string(Expl_file_type type, Str_data* type_string)
{
	if(type == EXPL_FILE_TYPE_NONE)
	{
		if(Util_str_add(type_string, "unknown,") != DEF_SUCCESS)
			return;
	}
	if(type & EXPL_FILE_TYPE_FILE)
	{
		if(Util_str_add(type_string, "file,") != DEF_SUCCESS)
			return;
	}
	if(type & EXPL_FILE_TYPE_DIR)
	{
		if(Util_str_add(type_string, "dir,") != DEF_SUCCESS)
			return;
	}
	if(type & EXPL_FILE_TYPE_READ_ONLY)
	{
		if(Util_str_add(type_string, "read only,") != DEF_SUCCESS)
			return;
	}
	if(type & EXPL_FILE_TYPE_HIDDEN)
	{
		if(Util_str_add(type_string, "hidden,") != DEF_SUCCESS)
			return;
	}

	if(type_string->length > 0)//Remove last comma.
		Util_str_resize(type_string, (type_string->length - 1));
}

static int Util_expl_compare_name(const void* a, const void* b)
{
	Util_expl_file_compare* file_a = (Util_expl_file_compare*)a;
	Util_expl_file_compare* file_b = (Util_expl_file_compare*)b;
	bool is_a_dir = (file_a->type & EXPL_FILE_TYPE_DIR);
	bool is_b_dir = (file_b->type & EXPL_FILE_TYPE_DIR);

	if((is_a_dir && is_b_dir)
	|| (!is_a_dir && !is_b_dir))
	{
		//Both elements have the same type, compare name.
		int32_t result = 0;
		uint32_t loop = Util_max(file_a->name.length, file_b->name.length);

		for(uint32_t i = 0; i < loop; i++)
		{
			char char_a = '\u0000';
			char char_b = '\u0000';
			uint8_t a_type = DEF_EXPL_SORT_TYPE_UNDEFINED;
			uint8_t b_type = DEF_EXPL_SORT_TYPE_UNDEFINED;

			if(i < file_a->name.length)
				char_a = file_a->name.buffer[i];
			if(i < file_b->name.length)
				char_b = file_b->name.buffer[i];

			if(char_a == char_b)
				continue;

			if(char_a >= '0' && char_a <= '9')
				a_type = DEF_EXPL_SORT_TYPE_NUMBER;
			else if((char_a >= 'a' && char_a <= 'z') || (char_a >= 'A' && char_a <= 'Z'))
				a_type = DEF_EXPL_SORT_TYPE_ALPHABET;
			else
				a_type = DEF_EXPL_SORT_TYPE_SPECIAL_CHAR;

			if(char_b >= '0' && char_b <= '9')
				b_type = DEF_EXPL_SORT_TYPE_NUMBER;
			else if((char_b >= 'a' && char_b <= 'z') || (char_b >= 'A' && char_b <= 'Z'))
				b_type = DEF_EXPL_SORT_TYPE_ALPHABET;
			else
				b_type = DEF_EXPL_SORT_TYPE_SPECIAL_CHAR;

			if(a_type == DEF_EXPL_SORT_TYPE_NUMBER && b_type == DEF_EXPL_SORT_TYPE_NUMBER)
			{
				//Both characters are numbers, just compare with ASCII values.
				result = ((int16_t)char_a - (int16_t)char_b);
				break;
			}
			else if(a_type == DEF_EXPL_SORT_TYPE_ALPHABET && b_type == DEF_EXPL_SORT_TYPE_ALPHABET)
			{
				//Both characters are alphabets, compare with ASCII values after lowering them.
				result = ((int16_t)tolower(char_a) - (int16_t)tolower(char_b));
				break;
			}
			else if(a_type == DEF_EXPL_SORT_TYPE_SPECIAL_CHAR && b_type == DEF_EXPL_SORT_TYPE_SPECIAL_CHAR)
			{
				//Both characters are special characters, just compare with ASCII values.
				result = ((int16_t)char_a - (int16_t)char_b);
				break;
			}
			else
			{
				//Both characters have the different type.
				//Special charcters should go first, then numbers, finally alphabets.
				if(a_type == DEF_EXPL_SORT_TYPE_SPECIAL_CHAR)
					result = -1;
				else if(b_type == DEF_EXPL_SORT_TYPE_SPECIAL_CHAR)
					result = 1;
				else if(a_type == DEF_EXPL_SORT_TYPE_NUMBER)
					result = -1;
				else if(b_type == DEF_EXPL_SORT_TYPE_NUMBER)
					result = 1;
				else if(a_type == DEF_EXPL_SORT_TYPE_ALPHABET)
					result = -1;
				else if(b_type == DEF_EXPL_SORT_TYPE_ALPHABET)
					result = 1;

				break;
			}
		}

		return result;
	}
	else
	{
		if(is_a_dir)//Directories should go first.
			return -1;
		else//Files should go after directories.
			return 1;
	}
}

static void Util_expl_read_dir_callback(void)
{
	if (util_expl_init)
	{
		if (util_expl_read_dir_request)
		{
			bool is_root_dir = false;
			uint32_t detected_files = 0;
			uint32_t result = DEF_ERR_OTHER;
			Util_expl_file files = { 0, };
			Util_expl_file_compare sort_cache[DEF_EXPL_MAX_FILES] = { 0, };

			var_need_reflesh = true;
			for (uint32_t i = 0; i < DEF_EXPL_MAX_FILES; i++)
			{
				Util_str_clear(&util_expl_files.name[i]);
				util_expl_files.type[i] = EXPL_FILE_TYPE_NONE;
				util_expl_files.size[i] = 0;
			}

			if(strcmp(util_expl_current_dir.buffer, "/") == 0)
				is_root_dir = true;

			if (!is_root_dir)
			{
				Util_str_set(&util_expl_files.name[0], ".. (Move to parent directory)");
				util_expl_files.type[0] = EXPL_FILE_TYPE_DIR;
			}

			//Read files in directory.
			DEF_LOG_RESULT_SMART(result, Util_file_read_dir(util_expl_current_dir.buffer, &detected_files, files.name, files.type, DEF_EXPL_MAX_FILES), (result == DEF_SUCCESS), result);

			if (result == DEF_SUCCESS)
			{
				//Non-root directory has a directory named "Go to parent directory".
				uint8_t offset = (is_root_dir ? 0 : 1);
				uint32_t loop = 0;

				for(uint32_t i = 0; i < detected_files; i++)
				{
					sort_cache[i].name = files.name[i];
					sort_cache[i].type = files.type[i];
				}

				qsort(sort_cache, detected_files, sizeof(Util_expl_file_compare), Util_expl_compare_name);

				loop = (uint32_t)Util_min((offset + detected_files), DEF_EXPL_MAX_FILES);
				for(uint32_t i = offset, source_index = 0; i < loop; i++)
				{
					Util_str_set(&util_expl_files.name[i], sort_cache[source_index].name.buffer);
					util_expl_files.type[i] = sort_cache[source_index].type;
					source_index++;
				}

				util_expl_num_of_file = (detected_files + offset);
			}
			else
				util_expl_num_of_file = 1;

			for(uint32_t i = 0; i < DEF_EXPL_MAX_FILES; i++)
				Util_str_free(&files.name[i]);

			util_expl_check_file_size_index = 0;
			var_need_reflesh = true;
			util_expl_read_dir_request = false;
		}
		else if(util_expl_check_file_size_index < util_expl_num_of_file)
		{
			while(util_expl_check_file_size_index < util_expl_num_of_file)
			{
				if(util_expl_files.type[util_expl_check_file_size_index] & EXPL_FILE_TYPE_FILE || util_expl_files.type[util_expl_check_file_size_index] & EXPL_FILE_TYPE_NONE)
				{
					uint64_t file_size = 0;
					uint32_t result = DEF_ERR_OTHER;

					result = Util_file_check_file_size(util_expl_files.name[util_expl_check_file_size_index].buffer, util_expl_current_dir.buffer, &file_size);
					if (result == DEF_SUCCESS)
					{
						util_expl_files.size[util_expl_check_file_size_index] = file_size;
						var_need_reflesh = true;
					}
					else
						DEF_LOG_RESULT(Util_file_check_file_size, (result == DEF_SUCCESS), result);

					util_expl_check_file_size_index++;
					//Don't check all files once as it locks worker thread too long.
					break;
				}
				else
					util_expl_check_file_size_index++;
			}
		}
	}
}
}
#endif //DEF_EXPL_API_ENABLE
