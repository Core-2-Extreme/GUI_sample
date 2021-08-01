#include "headers.hpp"

bool draw_sheet_texture_free[DEF_DRAW_MAX_NUM_OF_SPRITE_SHEETS];
double draw_frametime[20] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, };
std::string draw_part_text[2][1024];
C2D_Font system_fonts[4];
C3D_RenderTarget* screen[3];
C2D_SpriteSheet sheet_texture[DEF_DRAW_MAX_NUM_OF_SPRITE_SHEETS];
C2D_Image wifi_icon_image[9];
C2D_Image battery_level_icon_image[21];
C2D_Image battery_charge_icon_image[1];
C2D_Image eco_image[2];
std::string draw_japanese_kanji[3000];
std::string draw_simple_chinese[6300];
TickCounter draw_frame_time_timer;

double Draw_query_frametime(void)
{
	return draw_frametime[19];
}

double Draw_query_fps(void)
{
	double cache = 0;
	for(int i = 0; i < 20; i++)
		cache += draw_frametime[i];

	return 1000.0 / (cache / 20);
}

extern "C" void memcpy_asm_4b(u8*, u8*);

int Draw_convert_to_pos(int height, int width, int img_height, int img_width, int pixel_size)
{
	int pos = img_width * height;
	if(pos == 0)
		pos = img_width;

	pos -= (img_width - width) - img_width;
	return pos * pixel_size;
}

Result_with_string Draw_set_texture_data(Image_data* c2d_image, u8* buf, int pic_width, int pic_height, int tex_size_x, int tex_size_y, GPU_TEXCOLOR color_format)
{
	return Draw_set_texture_data(c2d_image, buf, pic_width, pic_height, 0, 0, tex_size_x, tex_size_y, color_format);
}

Result_with_string Draw_set_texture_data(Image_data* c2d_image, u8* buf, int pic_width, int pic_height, int parse_start_width, int parse_start_height, int tex_size_x, int tex_size_y, GPU_TEXCOLOR color_format)
{
	int x_max = 0;
	int y_max = 0;
	int increase_list_x[tex_size_x + 8]; //= { 4, 12, 4, 44, }
	int increase_list_y[tex_size_y + 8]; //= { 2, 6, 2, 22, 2, 6, 2, tex_size_x * 8 - 42, };
	int count[2] = { 0, 0, };
	int c3d_pos = 0;
	int c3d_offset = 0;
	int pixel_size = 0;
	Result_with_string result;

	if(color_format == GPU_RGB8)
		pixel_size = 3;
	else if(color_format == GPU_RGB565)
		pixel_size = 2;
	else
	{
		result.code = DEF_ERR_INVALID_ARG;
		result.string = DEF_ERR_INVALID_ARG_STR;
		return result;
	}

	for(int i = 0; i <= tex_size_x; i+=4)
	{
		increase_list_x[i] = 4 * pixel_size;
		increase_list_x[i + 1] = 12 * pixel_size;
		increase_list_x[i + 2] = 4 * pixel_size;
		increase_list_x[i + 3] = 44 * pixel_size;
	}
	for(int i = 0; i <= tex_size_y; i+=8)
	{
		increase_list_y[i] = 2 * pixel_size;
		increase_list_y[i + 1] = 6 * pixel_size;
		increase_list_y[i + 2] = 2 * pixel_size;
		increase_list_y[i + 3] = 22 * pixel_size;
		increase_list_y[i + 4] = 2 * pixel_size;
		increase_list_y[i + 5] = 6 * pixel_size;
		increase_list_y[i + 6] = 2 * pixel_size;
		increase_list_y[i + 7] = (tex_size_x * 8 - 42) * pixel_size;
	}

	if (parse_start_width > pic_width || parse_start_height > pic_height)
	{
		result.code = DEF_ERR_INVALID_ARG;
		result.string = DEF_ERR_INVALID_ARG_STR;
		return result;
	}
	
	y_max = pic_height - (u32)parse_start_height;
	x_max = pic_width - (u32)parse_start_width;
	if (tex_size_y < y_max)
		y_max = tex_size_y;
	if (tex_size_x < x_max)
		x_max = tex_size_x;

	c2d_image->subtex->width = (u16)x_max;
	c2d_image->subtex->height = (u16)y_max;
	c2d_image->subtex->left = 0.0;
	c2d_image->subtex->top = 1.0;
	c2d_image->subtex->right = x_max / (float)tex_size_x;
	c2d_image->subtex->bottom = 1.0 - y_max / (float)tex_size_y;
	c2d_image->c2d.subtex = c2d_image->subtex;

	if(pixel_size == 2)
	{
		for(int k = 0; k < y_max; k++)
		{
			for(int i = 0; i < x_max; i += 2)
			{
				memcpy_asm_4b(&(((u8*)c2d_image->c2d.tex->data)[c3d_pos + c3d_offset]), &(((u8*)buf)[Draw_convert_to_pos(k + parse_start_height, i + parse_start_width, pic_height, pic_width, pixel_size)]));
				c3d_pos += increase_list_x[count[0]];
				count[0]++;
			}
			count[0] = 0;
			c3d_pos = 0;
			c3d_offset += increase_list_y[count[1]];
			count[1]++;
		}
	}
	else if(pixel_size == 3)
	{
		for(int k = 0; k < y_max; k++)
		{
			for(int i = 0; i < x_max; i += 2)
			{
				memcpy_asm_4b(&(((u8*)c2d_image->c2d.tex->data)[c3d_pos + c3d_offset]), &(((u8*)buf)[Draw_convert_to_pos(k + parse_start_height, i + parse_start_width, pic_height, pic_width, pixel_size)]));
				memcpy(&(((u8*)c2d_image->c2d.tex->data)[c3d_pos + c3d_offset + 4]), &(((u8*)buf)[Draw_convert_to_pos(k + parse_start_height, i + parse_start_width, pic_height, pic_width, pixel_size) + 4]), 2);
				c3d_pos += increase_list_x[count[0]];
				count[0]++;
			}
			count[0] = 0;
			c3d_pos = 0;
			c3d_offset += increase_list_y[count[1]];
			count[1]++;
		}
	}

	C3D_TexFlush(c2d_image->c2d.tex);

	return result;
}

void Draw_c2d_image_set_filter(Image_data* c2d_image, bool filter)
{
	if(filter)
		C3D_TexSetFilter(c2d_image->c2d.tex, GPU_LINEAR, GPU_LINEAR);
	else
		C3D_TexSetFilter(c2d_image->c2d.tex, GPU_NEAREST, GPU_NEAREST);
}

Result_with_string Draw_c2d_image_init(Image_data* c2d_image, int tex_size_x, int tex_size_y, GPU_TEXCOLOR color_format)
{
	Result_with_string result;

	c2d_image->subtex = (Tex3DS_SubTexture*)linearAlloc(sizeof(Tex3DS_SubTexture*));
	c2d_image->c2d.tex = (C3D_Tex*)linearAlloc(sizeof(C3D_Tex*));
	if(c2d_image->subtex == NULL || c2d_image->c2d.tex == NULL)
	{
		linearFree(c2d_image->subtex);
		linearFree(c2d_image->c2d.tex);
		c2d_image->subtex = NULL;
		c2d_image->c2d.tex = NULL;
		result.code = DEF_ERR_OUT_OF_LINEAR_MEMORY;
		result.string = DEF_ERR_OUT_OF_LINEAR_MEMORY_STR;
	}
	c2d_image->c2d.subtex = c2d_image->subtex;

	if (!C3D_TexInit(c2d_image->c2d.tex, (u16)tex_size_x, (u16)tex_size_y, color_format))
	{
		result.code = DEF_ERR_OUT_OF_LINEAR_MEMORY;
		result.string = DEF_ERR_OUT_OF_LINEAR_MEMORY_STR;
		return result;
	}

	C3D_TexSetFilter(c2d_image->c2d.tex, GPU_LINEAR, GPU_LINEAR);
	c2d_image->c2d.tex->border = 0xFFFFFF;
	C3D_TexSetWrap(c2d_image->c2d.tex, GPU_CLAMP_TO_EDGE, GPU_CLAMP_TO_EDGE);


	return result;
}

void Draw_c2d_image_free(Image_data c2d_image)
{
	linearFree(c2d_image.c2d.tex->data);
	linearFree(c2d_image.c2d.tex);
	linearFree(c2d_image.subtex);
	c2d_image.c2d.tex->data = NULL;
	c2d_image.c2d.tex = NULL;
	c2d_image.c2d.subtex = NULL;
	c2d_image.subtex = NULL;
}

void Draw(std::string text, float x, float y, float text_size_x, float text_size_y, int abgr8888)
{
	bool reverse = false;
	bool found = false;
	bool font_loaded[2] = { Exfont_is_loaded_system_font(0), Exfont_is_loaded_system_font(1), };//JPN, CHN
	float width = 0, height = 0, original_x, y_offset;
	int previous_num = -3;
	int memcmp_result = -1;
	int count = 0;
	int characters = 0;
	int font_num_list[2][1024];
	std::string sample[8] = { "\u0000", "\u000A", "\u4DFF", "\uA000", "\u312F", "\u3190", "\uABFF", "\uD7B0", };
	C2D_Text c2d_text;
	C2D_TextBuf c2d_buf;
	original_x = x;
	c2d_buf = C2D_TextBufNew(4096);

	Exfont_text_parse(text, draw_part_text[0], 1023, &characters);
	Exfont_text_parse(Exfont_text_sort(draw_part_text[0], 1023), draw_part_text[0], 1023, &characters);

	for (int i = 0; i < characters; i++)
	{
		reverse = false;
		if (memcmp((void*)draw_part_text[0][i].c_str(), (void*)sample[0].c_str(), 0x1) == 0)
		{
			font_num_list[0][i] = -2;
			break;
		}
		else if (memcmp((void*)draw_part_text[0][i].c_str(), (void*)sample[1].c_str(), 0x1) == 0)
		{
			font_num_list[0][i] = -1;
			continue;
		}

		if((memcmp((void*)draw_part_text[0][i].c_str(), (void*)sample[2].c_str(), 0x3) > 0 && memcmp((void*)draw_part_text[0][i].c_str(), (void*)sample[3].c_str(), 0x3) < 0)
		|| (memcmp((void*)draw_part_text[0][i].c_str(), (void*)sample[4].c_str(), 0x3) > 0 && memcmp((void*)draw_part_text[0][i].c_str(), (void*)sample[5].c_str(), 0x3) < 0)
		|| (memcmp((void*)draw_part_text[0][i].c_str(), (void*)sample[6].c_str(), 0x3) > 0 && memcmp((void*)draw_part_text[0][i].c_str(), (void*)sample[7].c_str(), 0x3) < 0))
		{
			if(memcmp((void*)draw_part_text[0][i].c_str(), (void*)sample[2].c_str(), 0x3) > 0 && memcmp((void*)draw_part_text[0][i].c_str(), (void*)sample[3].c_str(), 0x3) < 0)
			{
				found = false;
				reverse = false;
				memcmp_result = 1;

				if(font_loaded[0])
				{
					for(int s = 0;;)
					{
						if(!reverse)
							s += 100;
						else
							s--;

						if((s < 0 || s > 3000) && reverse)
							break;
						else if(s > 3000)
						{
							reverse = true;
							s = 3000;
						}
						else
							memcmp_result = memcmp((void*)draw_part_text[0][i].c_str(), (void*)draw_japanese_kanji[s].c_str(), 3);

						if(memcmp_result == 0)
						{
							font_num_list[0][i] = 0; //JPN
							found = true;
							break;
						}
						else if(memcmp_result < 0)
							reverse = true;
					}
				}

				if(!found)
				{
					reverse = false;
					memcmp_result = 1;

					if(font_loaded[1])
					{
						for(int s = 0;;)
						{
							if(!reverse)
								s += 100;
							else
								s--;

							if((s < 0 || s > 6300) && reverse)
								break;
							else if(s > 6300)
							{
								reverse = true;
								s = 6300;
							}
							else
								memcmp_result = memcmp((void*)draw_part_text[0][i].c_str(), (void*)draw_simple_chinese[s].c_str(), 3);

							if(memcmp_result == 0)
							{
								font_num_list[0][i] = 1; //CHN
								found = true;
								break;
							}
							else if(memcmp_result < 0)
								reverse = true;
						}
					}
				}

				if(!found)
				  font_num_list[0][i] = 3; //TWN
			}
			else
				font_num_list[0][i] = 2; //KOR
		}
		else
			font_num_list[0][i] = 4;
	}

	draw_part_text[1][0] = "";
	previous_num = font_num_list[0][0];
	for (int i = 0; i < characters; i++)
	{
		if(font_num_list[0][i] == -2)
		{
			font_num_list[1][count + 1] = font_num_list[0][i];
			break;
		}
		else if(previous_num != font_num_list[0][i] || font_num_list[0][i] == -1)
		{
			count++;
			draw_part_text[1][count] = "";
		}

		draw_part_text[1][count] += draw_part_text[0][i];
		font_num_list[1][count] = font_num_list[0][i];
		previous_num = font_num_list[0][i];
	}

	for (int i = 0; i <= count; i++)
	{
		if (font_num_list[1][i] == -2)
			break;
		else if (font_num_list[1][i] == -1)
		{
			y += 20.0 * text_size_y;
			x = original_x;
			continue;
		}

		if(!Exfont_is_loaded_external_font(0) || (font_num_list[1][i] >= 0 && font_num_list[1][i] <= 3))
		{
			if(!Exfont_is_loaded_external_font(0))
				system_fonts[font_num_list[1][i]] = 0;

			C2D_TextBufClear(c2d_buf);
			if(font_num_list[1][i] == 1)
				y_offset = 3 * text_size_y;
			else if(font_num_list[1][i] == 3)
				y_offset = 5 * text_size_y;
			else
				y_offset = 0;

			C2D_TextFontParse(&c2d_text, system_fonts[font_num_list[1][i]], c2d_buf, draw_part_text[1][i].c_str());
			C2D_TextOptimize(&c2d_text);
			C2D_TextGetDimensions(&c2d_text, text_size_x, text_size_y, &width, &height);
			C2D_DrawText(&c2d_text, C2D_WithColor, x, y + y_offset, 0.0, text_size_x, text_size_y, abgr8888);
			x += width;
		}
		else if(font_num_list[1][i] == 4)
		{
			Exfont_draw_external_fonts(draw_part_text[1][i], x, y, text_size_x * 1.56, text_size_y * 1.56, abgr8888, &width, &height);
			x += width;
		}
	}
	C2D_TextBufDelete(c2d_buf);
}

int Draw_get_free_sheet_num(void)
{
	for(int i = 0; i < DEF_DRAW_MAX_NUM_OF_SPRITE_SHEETS; i++)
	{
		if(draw_sheet_texture_free[i])
			return i;
	}
	return -1;
}

Result_with_string Draw_load_texture(std::string file_name, int sheet_map_num, C2D_Image return_image[], int start_num, int num_of_array)
{
	size_t num_of_images;
	bool function_fail = false;
	Result_with_string result;

	sheet_texture[sheet_map_num] = C2D_SpriteSheetLoad(file_name.c_str());
	if (sheet_texture[sheet_map_num] == NULL)
	{
		result.code = DEF_ERR_OTHER;
		result.string = "[Error] Couldn't load texture file : " + file_name + " ";
		function_fail = true;
	}
	else
		draw_sheet_texture_free[sheet_map_num] = false;

	if (!function_fail)
	{
		num_of_images = C2D_SpriteSheetCount(sheet_texture[sheet_map_num]);
		if ((int)num_of_images < num_of_array)
		{
			result.code = DEF_ERR_OTHER;
			result.string = "[Error] num of arry " + std::to_string(num_of_array) + " is bigger than spritesheet has num of image(s) " + std::to_string(num_of_images) + " ";
			function_fail = true;
		}
	}

	if (!function_fail)
	{
		for (int i = 0; i <= (num_of_array - 1); i++)
			return_image[start_num + i] = C2D_SpriteSheetGetImage(sheet_texture[sheet_map_num], i);
	}
	return result;
}

void Draw_touch_pos(void)
{
	Hid_info key;
	Util_hid_query_key_state(&key);
	if(key.p_touch || key.h_touch)
		Draw("●", key.touch_x, key.touch_y, 0.20, 0.20, DEF_DRAW_RED);
}

void Draw_top_ui(void)
{
	Draw_texture(var_square_image[0], DEF_DRAW_BLACK, 0.0, 0.0, 400.0, 15.0);
	Draw_texture(wifi_icon_image[var_wifi_signal], DEF_DRAW_NO_COLOR, 360.0, 0.0, 15.0, 15.0);
	Draw_texture(battery_level_icon_image[var_battery_level_raw / 5], DEF_DRAW_NO_COLOR, 315.0, 0.0, 30.0, 15.0);
	Draw_texture(eco_image[var_eco_mode], DEF_DRAW_NO_COLOR, 345.0, 0.0, 15.0, 15.0);
	if (var_battery_charge)
		Draw_texture(battery_charge_icon_image[0], DEF_DRAW_NO_COLOR, 295.0, 0.0, 20.0, 15.0);
	Draw(var_status, 0.0, 0.0, 0.45, 0.45, DEF_DRAW_GREEN);
	Draw(std::to_string(var_battery_level_raw), 322.5, 1.25, 0.4, 0.4, DEF_DRAW_BLACK);

	if (var_debug_mode)
		Draw_debug_info();
}

void Draw_bot_ui(void)
{
	Draw_texture(var_square_image[0], DEF_DRAW_BLACK, 0.0, 225.0, 320.0, 15.0);
	Draw("▽", 155.0, 220.0, 0.75, 0.75, DEF_DRAW_WHITE);
}

void Draw_texture(C2D_Image image, float x, float y, float x_size, float y_size)
{
	Draw_texture(image, DEF_DRAW_NO_COLOR, x, y, x_size, y_size);
}

void Draw_texture(C2D_Image image, int abgr8888, float x, float y, float x_size, float y_size)
{
	C2D_ImageTint tint;
	C2D_DrawParams c2d_parameter =
	{
		{
			x,
			y,
			x_size,
			y_size
		},
		{
			0,
			0
		},
		0.0f,
		0.0f
	};

	if (!(image.tex == NULL))
	{
		if(abgr8888 == DEF_DRAW_NO_COLOR)
			C2D_DrawImage(image, &c2d_parameter, NULL);
		else
		{
			C2D_PlainImageTint(&tint, abgr8888, true);
			C2D_DrawImage(image, &c2d_parameter, &tint);
		}
	}
}

void Draw_line(float x_0, float y_0, int abgr8888_0, float x_1, float y_1, int abgr8888_1, float width)
{
	C2D_DrawRectangle(0, 0, 0, 0, 0, 0x0, 0x0, 0x0, 0x0);
	//magic C2D_DrawLine() won't work without calling C2D_DrawRectangle()
	C2D_DrawLine(x_0, y_0, abgr8888_0, x_1, y_1, abgr8888_1, width, 0);
}

void Draw_debug_info(void)
{
	int color = DEF_DRAW_BLACK;
	Hid_info key;
	Util_hid_query_key_state(&key);

	if (var_night_mode)
		color = DEF_DRAW_WHITE;

	Draw_texture(var_square_image[0], DEF_DRAW_WEAK_BLUE, 0.0, 20.0, 70.0, 140.0);
	Draw_texture(var_square_image[0], DEF_DRAW_WEAK_BLUE, 0.0, 160.0, 110.0, 60.0);
	Draw("A　 p: " + std::to_string(key.p_a) + " h: " + std::to_string(key.h_a), 0.0, 20.0, 0.4, 0.4, color);
	Draw("B　 p: " + std::to_string(key.p_b) + " h: " + std::to_string(key.h_b), 0.0, 30.0, 0.4, 0.4, color);
	Draw("X　 p: " + std::to_string(key.p_x) + " h: " + std::to_string(key.h_x), 0.0, 40.0, 0.4, 0.4, color);
	Draw("Y　 p: " + std::to_string(key.p_y) + " h: " + std::to_string(key.h_y), 0.0, 50.0, 0.4, 0.4, color);
	Draw("L　 p: " + std::to_string(key.p_l) + " h: " + std::to_string(key.h_l), 0.0, 60.0, 0.4, 0.4, color);
	Draw("R　 p: " + std::to_string(key.p_r) + " h: " + std::to_string(key.h_r), 0.0, 70.0, 0.4, 0.4, color);
	Draw("C↓ p: " + std::to_string(key.p_c_down) + " h: " + std::to_string(key.h_c_down), 0.0, 80.0, 0.4, 0.4, color);
	Draw("C→ p: " + std::to_string(key.p_c_right) + " h: " + std::to_string(key.h_c_right), 0.0, 90.0, 0.4, 0.4, color);
	Draw("C↑ p: " + std::to_string(key.p_c_up) + " h: " + std::to_string(key.h_c_up), 0.0, 100.0, 0.4, 0.4, color);
	Draw("C← p: " + std::to_string(key.p_c_left) + " h: " + std::to_string(key.h_c_left), 0.0, 110.0, 0.4, 0.4, color);
	Draw("D↓ p: " + std::to_string(key.p_d_down) + " h: " + std::to_string(key.h_d_down), 0.0, 120.0, 0.4, 0.4, color);
	Draw("D→ p: " + std::to_string(key.p_d_right) + " h: " + std::to_string(key.h_d_right), 0.0, 130.0, 0.4, 0.4, color);
	Draw("D↑ p: " + std::to_string(key.p_d_up) + " h: " + std::to_string(key.h_d_up), 0.0, 140.0, 0.4, 0.4, color);
	Draw("D← p: " + std::to_string(key.p_d_left) + " h: " + std::to_string(key.h_d_left), 0.0, 150.0, 0.4, 0.4, color);
	Draw("touch x: " + std::to_string(key.touch_x) + ", y: " + std::to_string(key.touch_y), 0.0, 160.0, 0.4, 0.4, color);
	Draw("CPU: " + std::to_string(C3D_GetProcessingTime()).substr(0, 5) + "ms", 0.0, 170.0, 0.4, 0.4, color);
	Draw("GPU: " + std::to_string(C3D_GetDrawingTime()).substr(0, 5) + "ms", 0.0, 180.0, 0.4, 0.4, color);
	Draw("Frametime: " + std::to_string(draw_frametime[19]).substr(0, 6) + "ms", 0.0, 190.0, 0.4, 0.4, color);
	Draw("RAM: " + std::to_string(var_free_ram / 1000.0).substr(0, 5) + " MB", 0.0, 200.0, 0.4, 0.4, color);
	Draw("linear RAM: " + std::to_string(var_free_linear_ram / 1000.0 / 1000.0).substr(0, 5) +" MB", 0.0, 210.0, 0.4, 0.4, color);
}

Result_with_string Draw_load_kanji_samples(void)
{
	int characters = 0;
	u8* fs_buffer = (u8*)malloc(0x8000);
	u32 read_size = 0;
	Result_with_string result;

	memset((void*)fs_buffer, 0x0, 0x8000);
	result = Util_file_load_from_rom("kanji.txt", "romfs:/gfx/font/sample/", fs_buffer, 0x8000, &read_size);
	if(result.code == 0)
		Exfont_text_parse((char*)fs_buffer, draw_japanese_kanji, 3000, &characters);

	memset((void*)fs_buffer, 0x0, 0x8000);
	result = Util_file_load_from_rom("hanyu_s.txt", "romfs:/gfx/font/sample/", fs_buffer, 0x8000, &read_size);
	if(result.code == 0)
		Exfont_text_parse((char*)fs_buffer, draw_simple_chinese, 6300, &characters);

	free(fs_buffer);
	return result;
}

Result_with_string Draw_init(bool wide, bool _3d)
{
	Result_with_string result;
	gfxInitDefault();

	//gfxSet3D(_3d);
	//gfxSetWide(wide);
	gfxSet3D(false);
	gfxSetWide(false);

	if(wide)
		gfxSetWide(wide);
	else if(_3d)
		gfxSet3D(_3d);

	C3D_Init(C3D_DEFAULT_CMDBUF_SIZE);
	C2D_Init(C2D_DEFAULT_MAX_OBJECTS);
	C2D_Prepare();
	screen[0] = C2D_CreateScreenTarget(GFX_TOP, GFX_LEFT);
	screen[1] = C2D_CreateScreenTarget(GFX_BOTTOM, GFX_LEFT);
	screen[2] = C2D_CreateScreenTarget(GFX_TOP, GFX_RIGHT);

	C2D_TargetClear(screen[0], C2D_Color32f(0, 0, 0, 0));
	C2D_TargetClear(screen[1], C2D_Color32f(0, 0, 0, 0));
	C2D_TargetClear(screen[2], C2D_Color32f(0, 0, 0, 0));

	for (int i = 0; i < DEF_DRAW_MAX_NUM_OF_SPRITE_SHEETS; i++)
		draw_sheet_texture_free[i] = true;

	osTickCounterStart(&draw_frame_time_timer);

	result = Draw_load_texture("romfs:/gfx/draw/wifi_signal.t3x", 0, wifi_icon_image, 0, 9);
	if(result.code != 0)
		return result;

	result = Draw_load_texture("romfs:/gfx/draw/battery_level.t3x", 1, battery_level_icon_image, 0, 21);
	if(result.code != 0)
		return result;

	result = Draw_load_texture("romfs:/gfx/draw/battery_charge.t3x", 2, battery_charge_icon_image, 0, 1);
	if(result.code != 0)
		return result;

	result = Draw_load_texture("romfs:/gfx/draw/eco.t3x", 3, eco_image, 0, 2);
	if(result.code != 0)
		return result;

	result = Draw_load_texture("romfs:/gfx/draw/square.t3x", 4, var_square_image, 0, 1);
	if(result.code != 0)
		return result;

	result = Draw_load_kanji_samples();

	return result;
}

void Draw_reinit(bool wide, bool _3d)
{
	C2D_Fini();
	C3D_Fini();

	//gfxSet3D(_3d);
	//gfxSetWide(wide);
	gfxSet3D(false);
	gfxSetWide(false);

	if(wide)
		gfxSetWide(wide);
	else if(_3d)
		gfxSet3D(_3d);

	C3D_Init(C3D_DEFAULT_CMDBUF_SIZE);
	C2D_Init(C2D_DEFAULT_MAX_OBJECTS);
	C2D_Prepare();
	screen[0] = C2D_CreateScreenTarget(GFX_TOP, GFX_LEFT);
	screen[1] = C2D_CreateScreenTarget(GFX_BOTTOM, GFX_LEFT);
	screen[2] = C2D_CreateScreenTarget(GFX_TOP, GFX_RIGHT);
}

Result_with_string Draw_load_system_font(int system_font_num)
{
	Result_with_string result;
	if (system_font_num == 0)
		system_fonts[0] = C2D_FontLoadSystem(CFG_REGION_JPN);
	else if (system_font_num == 1)
		system_fonts[1] = C2D_FontLoadSystem(CFG_REGION_CHN);
	else if (system_font_num == 2)
		system_fonts[2] = C2D_FontLoadSystem(CFG_REGION_KOR);
	else if (system_font_num == 3)
		system_fonts[3] = C2D_FontLoadSystem(CFG_REGION_TWN);
	else
	{
		result.code = DEF_ERR_INVALID_ARG;
		result.string = DEF_ERR_INVALID_ARG_STR;
		return result;
	}

	if(system_fonts[system_font_num] == NULL)
	{
		result.code = -1;
		result.string = "[Error] Couldn't load font file : " + std::to_string(system_font_num) + " ";
	}
	return result;
}

void Draw_free_system_font(int system_font_num)
{
	if (system_font_num >= 0 && system_font_num <= 3)
	{
		if (system_fonts[system_font_num] != NULL)
		{
			C2D_FontFree(system_fonts[system_font_num]);
			system_fonts[system_font_num] = NULL;
		}
	}
}

void Draw_free_texture(int sheet_map_num)
{
	if(sheet_map_num >= 0 && sheet_map_num < DEF_DRAW_MAX_NUM_OF_SPRITE_SHEETS)
	{
		if (sheet_texture[sheet_map_num] != NULL)
		{
			C2D_SpriteSheetFree(sheet_texture[sheet_map_num]);
			sheet_texture[sheet_map_num] = NULL;
		}
		draw_sheet_texture_free[sheet_map_num] = true;
	}
}

void Draw_exit(void)
{
	for (int i = 0; i < 128; i++)
		Draw_free_texture(i);
	for (int i = 0; i < 4; i++)
		Draw_free_system_font(i);

	C2D_Fini();
	C3D_Fini();
	gfxExit();
}

void Draw_frame_ready(void)
{
	C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
}

void Draw_screen_ready(int screen_num, int abgr8888)
{
	if (screen_num >= 0 && screen_num <= 2)
	{
		C2D_TargetClear(screen[screen_num], abgr8888);
		C2D_SceneBegin(screen[screen_num]);
	}
}

void Draw_apply_draw(void)
{
	C3D_FrameEnd(0);
	osTickCounterUpdate(&draw_frame_time_timer);
	draw_frametime[19] = osTickCounterRead(&draw_frame_time_timer);
	for(int i = 0; i < 19; i++)
		draw_frametime[i] = draw_frametime[i + 1];
}
