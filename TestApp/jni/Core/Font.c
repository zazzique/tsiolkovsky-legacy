
//	Using http://www.angelcode.com/products/bmfont/


#include "Common.h"

#include "Files.h"
#include "FastMath.h"
#include "Vector.h"
#include "TextureManager.h"
#include "Render.h"
#include "Font.h"

#define MAX_FONTS_COUNT 8
#define MAX_TEXT_SIZE 1024
#define MAX_KERNINGS_COUNT 2048

#define FONT_PARSE_SEPARATORS " \n\r=\t"

enum
{
	FONT_PG_INFO,
	FONT_PG_COMMON,
	FONT_PG_PAGE,
	FONT_PG_CHARS,
	FONT_PG_CHAR,
	FONT_PG_KERNINGS,
	FONT_PG_KERNING
};

typedef struct _FontCharacter
{
	float x;
	float y;
	float width;
	float height;
	float x_offset;
	float y_offset;
	float x_advance;
	int kernings_count;
	int kernings_first_index;
} FontCharacter;

typedef struct _Font
{
	char name[32];
	char texture[32];
	float line_height;
	float scale_x;
	float scale_y;
	float default_scale;
	FontCharacter font_chars[256];
	U8 kerning_cache_chars[MAX_KERNINGS_COUNT];
	float kerning_cache_amounts[MAX_KERNINGS_COUNT];
	int kerning_table_size;
	float font_vertices[MAX_TEXT_SIZE * 8];
	U32 font_colors[MAX_TEXT_SIZE * 4];
	float font_tex_coords[MAX_TEXT_SIZE * 8];
	int char_count;
} Font;

Font fonts[MAX_FONTS_COUNT];
int fonts_count;

typedef struct _FontKerning
{
	char first_char;
	char second_char;
	float amount;
	
} FontKerning;

FontKerning kerning_table[MAX_KERNINGS_COUNT];

U16 font_index_array[MAX_TEXT_SIZE * 6];

U32 font_colors_presets[8] = { 0xff404040, 0xff20e0ff, 0xff0000ff, 0xff4040ff, 0xff4040ff, 0xff80caff, 0xff000000, 0xffffffff };

void Font_Init()
{
	fonts_count = 0;
	
	for (int i = 0; i < MAX_FONTS_COUNT; i++)
	{
		fonts[i].char_count = 0;
	}

	for (int i = 0; i < MAX_TEXT_SIZE; i++)
	{
		font_index_array[i * 6 + 0] = (i * 4) + 0;
		font_index_array[i * 6 + 1] = (i * 4) + 1;
		font_index_array[i * 6 + 2] = (i * 4) + 2;
		font_index_array[i * 6 + 3] = (i * 4) + 2;
		font_index_array[i * 6 + 4] = (i * 4) + 3;
		font_index_array[i * 6 + 5] = (i * 4) + 0;
	}
}


void Font_Add(char *font_name)
{
	FileHandler font_file;
	
	if (fonts_count >= MAX_FONTS_COUNT)
	{
		LogPrint("Error: too many fonts!\n");
		return;
	}
	
	if ((!tablet) && (pixel_scale <= 1.0f))
	{
		fonts[fonts_count].default_scale = 1.0f;
		
		if (!Files_OpenFileOfType(&font_file, font_name, "fnt"))
		{
			LogPrint("Error: font '%s' not found!\n", font_name);
			return;
		}
	}
	else
	{
		if (!tablet)
			fonts[fonts_count].default_scale = 0.5f;
		else
			fonts[fonts_count].default_scale = 1.0f;

		if (!Files_OpenFileOfType(&font_file, font_name, "fnthd"))
		{
			if (!tablet)
				fonts[fonts_count].default_scale = 1.0f;
			else
				fonts[fonts_count].default_scale = 2.0f;
			
			if (!Files_OpenFileOfType(&font_file, font_name, "fnt"))
			{
				LogPrint("Error: font '%s' not found!\n", font_name);
				return;
			}
		}
	}

	int parsing_group = FONT_PG_INFO;
	
	int char_id = 0;
	BOOL succeed = TRUE;
	
	int kernings_counter = 0;
	
	char *content = NULL;
	I32 content_size;

	Files_GetData(&font_file, (void **)&content, &content_size);

	if (content == NULL)
	{
		LogPrint("Error: font '%s' not found!\n", font_name);
		return;
	}
	
	content[content_size - 1] = '\0';
	
	char *cur_exp = NULL;
	cur_exp = strtok (content, FONT_PARSE_SEPARATORS);
	
	while (cur_exp != NULL)
	{
		cur_exp = strtok (NULL, FONT_PARSE_SEPARATORS);
		if (cur_exp == NULL)
			break;
		
		if (strcmp(cur_exp, "info") == 0)
		{
			parsing_group = FONT_PG_INFO;
			continue;
		}
		
		if (parsing_group == FONT_PG_INFO)
		{
			if (strcmp(cur_exp, "common") == 0)
			{
				parsing_group = FONT_PG_COMMON;
				continue;
			}
			
			if (strcmp(cur_exp, "unicode") == 0)
			{
				cur_exp = strtok (NULL, FONT_PARSE_SEPARATORS);
				if (atoi(cur_exp) != 0)
				{
					LogPrint("Error: unicode fonts are not supported!\n");
					succeed = FALSE;
					break;
				}
				continue;
			}
			cur_exp = strtok (NULL, FONT_PARSE_SEPARATORS);
		}
		
		if (parsing_group == FONT_PG_COMMON)
		{
			if (strcmp(cur_exp, "page") == 0)
			{
				parsing_group = FONT_PG_PAGE;
				continue;
			}
			
			if (strcmp(cur_exp, "lineHeight") == 0)
			{
				cur_exp = strtok (NULL, FONT_PARSE_SEPARATORS);
				fonts[fonts_count].line_height = (float)atof(cur_exp);
				continue;
			}
			if (strcmp(cur_exp, "scaleW") == 0)
			{
				cur_exp = strtok (NULL, FONT_PARSE_SEPARATORS);
				float div_tmp = (float)atof(cur_exp);
				if (div_tmp <= 0.0f)
					div_tmp = FLT_EPSILON;
				fonts[fonts_count].scale_x = 1.0f / div_tmp;
				continue;
			}
			if (strcmp(cur_exp, "scaleH") == 0)
			{
				cur_exp = strtok (NULL, FONT_PARSE_SEPARATORS);
				float div_tmp = (float)atof(cur_exp);
				if (div_tmp <= 0.0f)
					div_tmp = FLT_EPSILON;
				fonts[fonts_count].scale_y = 1.0f / div_tmp;
				continue;
			}
			if (strcmp(cur_exp, "pages") == 0)
			{
				cur_exp = strtok (NULL, FONT_PARSE_SEPARATORS);
				if (atoi(cur_exp) != 1)
				{
					LogPrint("Error: too many pages in font!\n");
					succeed = FALSE;
					break;
				}
				continue;
			}
			cur_exp = strtok (NULL, FONT_PARSE_SEPARATORS);
		}
		
		if (parsing_group == FONT_PG_PAGE)
		{
			if (strcmp(cur_exp, "kernings") == 0)
			{
				parsing_group = FONT_PG_KERNINGS;
				continue;
			}
			if (strcmp(cur_exp, "chars") == 0)
			{
				parsing_group = FONT_PG_CHARS;
				continue;
			}
			if (strcmp(cur_exp, "page") == 0)
			{
				continue;
			}
			
			if (strcmp(cur_exp, "file") == 0)
			{
				cur_exp = strtok (NULL, FONT_PARSE_SEPARATORS);
				strcpy(fonts[fonts_count].texture, cur_exp);
				continue;
			}
			cur_exp = strtok (NULL, FONT_PARSE_SEPARATORS);
		}
		
		if (parsing_group == FONT_PG_CHARS)
		{
			if (strcmp(cur_exp, "char") == 0)
			{
				parsing_group = FONT_PG_CHAR;
				continue;
			}
			cur_exp = strtok (NULL, FONT_PARSE_SEPARATORS);
		}
		
		if (parsing_group == FONT_PG_CHAR)
		{
			if (strcmp(cur_exp, "kernings") == 0)
			{
				parsing_group = FONT_PG_KERNINGS;
				continue;
			}
			
			if (strcmp(cur_exp, "char") == 0)
			{
				continue;
			}
			
			if (strcmp(cur_exp, "id") == 0)
			{
				cur_exp = strtok (NULL, FONT_PARSE_SEPARATORS);
				char_id = atoi(cur_exp);
				continue;
			}
			if (strcmp(cur_exp, "x") == 0)
			{
				cur_exp = strtok (NULL, FONT_PARSE_SEPARATORS);
				fonts[fonts_count].font_chars[char_id].x = (float)atof(cur_exp);
				continue;
			}
			if (strcmp(cur_exp, "y") == 0)
			{
				cur_exp = strtok (NULL, FONT_PARSE_SEPARATORS);
				fonts[fonts_count].font_chars[char_id].y = (float)atof(cur_exp);
				continue;
			}
			if (strcmp(cur_exp, "width") == 0)
			{
				cur_exp = strtok (NULL, FONT_PARSE_SEPARATORS);
				fonts[fonts_count].font_chars[char_id].width = (float)atof(cur_exp);
				continue;
			}
			if (strcmp(cur_exp, "height") == 0)
			{
				cur_exp = strtok (NULL, FONT_PARSE_SEPARATORS);
				fonts[fonts_count].font_chars[char_id].height = (float)atof(cur_exp);
				continue;
			}
			if (strcmp(cur_exp, "xoffset") == 0)
			{
				cur_exp = strtok (NULL, FONT_PARSE_SEPARATORS);
				fonts[fonts_count].font_chars[char_id].x_offset = (float)atof(cur_exp);
				continue;
			}
			if (strcmp(cur_exp, "yoffset") == 0)
			{
				cur_exp = strtok (NULL, FONT_PARSE_SEPARATORS);
				fonts[fonts_count].font_chars[char_id].y_offset = (float)atof(cur_exp);
				continue;
			}
			if (strcmp(cur_exp, "xadvance") == 0)
			{
				cur_exp = strtok (NULL, FONT_PARSE_SEPARATORS);
				fonts[fonts_count].font_chars[char_id].x_advance = (float)atof(cur_exp);
				continue;
			}
			cur_exp = strtok (NULL, FONT_PARSE_SEPARATORS);
		}
		
		if (parsing_group == FONT_PG_KERNINGS)
		{
			if (strcmp(cur_exp, "kerning") == 0)
			{
				parsing_group = FONT_PG_KERNING;
				kernings_counter ++;
				continue;
			}
			if (strcmp(cur_exp, "count") == 0)
			{
				cur_exp = strtok (NULL, FONT_PARSE_SEPARATORS);
				fonts[fonts_count].kerning_table_size = atoi(cur_exp);
				
				if (fonts[fonts_count].kerning_table_size >= MAX_KERNINGS_COUNT)
				{
					LogPrint("Error: too many kernings!\n");
					succeed = FALSE;
					break;
				}
				continue;
			}
			
			cur_exp = strtok (NULL, FONT_PARSE_SEPARATORS);
		}

		if (parsing_group == FONT_PG_KERNING)
		{
			if (strcmp(cur_exp, "chars") == 0)
			{
				parsing_group = FONT_PG_CHARS;
				continue;
			}
			
			if (strcmp(cur_exp, "kerning") == 0)
			{
				kernings_counter ++;
				continue;
			}
			
			if (strcmp(cur_exp, "first") == 0)
			{
				cur_exp = strtok (NULL, FONT_PARSE_SEPARATORS);
				kerning_table[kernings_counter - 1].first_char = atoi(cur_exp);
				continue;
			}
			if (strcmp(cur_exp, "second") == 0)
			{
				cur_exp = strtok (NULL, FONT_PARSE_SEPARATORS);
				kerning_table[kernings_counter - 1].second_char = atoi(cur_exp);
				continue;
			}
			if (strcmp(cur_exp, "amount") == 0)
			{
				cur_exp = strtok (NULL, FONT_PARSE_SEPARATORS);
				kerning_table[kernings_counter - 1].amount = (float)atof(cur_exp);
				continue;
			}
			cur_exp = strtok (NULL, FONT_PARSE_SEPARATORS);
		}
	}
	
	int kerning_index_in_cache = 0;
	int kernings_in_char_count = 0;
	
	for (int i = 0; i < 256; i++)
	{
		kernings_in_char_count = 0;
		
		for (int j = 0; j < kernings_counter; j++)
		{
			if (kerning_table[j].first_char == i)
			{
				if (kernings_in_char_count == 0)
				{
					fonts[fonts_count].font_chars[i].kernings_first_index = kerning_index_in_cache;
				}
				
				kernings_in_char_count ++;
				
				fonts[fonts_count].font_chars[i].kernings_count = kernings_in_char_count;
				
				fonts[fonts_count].kerning_cache_chars[kerning_index_in_cache] = kerning_table[j].second_char;
				fonts[fonts_count].kerning_cache_amounts[kerning_index_in_cache] = kerning_table[j].amount;
				
				kerning_index_in_cache ++;
			}
		}
	}
	
	if (succeed)
	{
        char tmp[256];
		cur_exp = strtok (fonts[fonts_count].texture, "\"");
        strcpy(tmp, cur_exp);
        strcpy(fonts[fonts_count].texture, tmp);
		TexManager_AddTexture(fonts[fonts_count].texture, 0);
		
		strcpy(fonts[fonts_count].name, Files_GetFileBaseName(&font_file));
		
		fonts_count ++;
	}
	
	Files_CloseFile(&font_file);
}


void Font_PrintText(float x, float y, float text_scale, U32 text_color, char *text, U32 flags, char *font_name)
{
	if (text == NULL)
		return;
	if (font_name == NULL)
		return;
	
	int current_font = -1;
	
	for (int i = 0; i < fonts_count; i++)
	{
		if (strcmp(fonts[i].name, font_name) == 0)
			current_font = i;
	}
	
	if (current_font < 0)
	{
		LogPrint("Error: font name not found!\n");
		return;
	}
	
	float cursor_x = x;
	float cursor_y = y;
	
	float char_pos_x;
	float char_pos_y;
	float char_pos_x2;
	float char_pos_y2;
	
	float char_coord_x;
	float char_coord_y;
	float char_coord_x2;
	float char_coord_y2;
	
	int line_char_count = 0;
	int text_lenght = strlen(text);
	
	int current_quad;
	
	Font *font_ptr = &fonts[current_font];
	FontCharacter *char_ptr;
	
	U8 *output_text = (U8 *)text;
	
	U32 color_default = text_color;
	U32 color = text_color;
	
	float scale = text_scale * font_ptr->default_scale;
	
	int reading_color = FALSE;
	
	x = ROUNDF(x);
	y = ROUNDF(y);
	
	if (flags & TEXT_VERTICAL_CENTERED)	
		cursor_y += ROUNDF(font_ptr->line_height * scale * 0.5f);
	else
		cursor_y += font_ptr->line_height * scale;
	
	for (int i = 0; i <= text_lenght; i++)
	{
		if (reading_color)
		{
			reading_color = FALSE;
			
			if (output_text[i] == '0')
			{
				color = color_default;
			}
			else if (output_text[i] == '1')
			{
				color = font_colors_presets[0];
			}
			else if (output_text[i] == '2')
			{
				color = font_colors_presets[1];
			}
			else if (output_text[i] == '3')
			{
				color = font_colors_presets[2];
			}
			else if (output_text[i] == '4')
			{
				color = font_colors_presets[3];
			}
			else if (output_text[i] == '5')
			{
				color = font_colors_presets[4];
			}
			else if (output_text[i] == '6')
			{
				color = font_colors_presets[5];
			}
			else if (output_text[i] == '7')
			{
				color = font_colors_presets[6];
			}
			else if (output_text[i] == '8')
			{
				color = font_colors_presets[7];
			}
			else
			{
				color = color_default;
			}
			continue;
		}
		
		if (output_text[i] == '#')
		{
			reading_color = TRUE;
			continue;
		}
		
		if (output_text[i] == '\n' || output_text[i] == '\0')
		{
			float dx;
			
			if (flags & TEXT_ALIGN_CENTER)
				dx = ROUNDF((cursor_x - x) * 0.5f);
			else if (flags & TEXT_ALIGN_RIGHT)
				dx = cursor_x - x;
			else
				dx = 0;
			
			if (dx != 0)
				for (int j = 0; j < line_char_count; j++)
					for (int k = 0; k < 8; k += 2)
						font_ptr->font_vertices[(font_ptr->char_count - line_char_count + j) * 8 + k] -= dx;
			
			line_char_count = 0;
			
			cursor_x = x;
			cursor_y -= font_ptr->line_height * scale;
			
			continue;
		}
		
		if (output_text[i] < 32)
			continue;
		
		char_ptr = &font_ptr->font_chars[output_text[i]];
		
		if (char_ptr->width <= 0 || char_ptr->height <= 0)
		{
			cursor_x += char_ptr->x_advance * scale;
			continue;
		}
		
		if (font_ptr->char_count >= MAX_TEXT_SIZE)
			break;
		
		char_pos_x = cursor_x + char_ptr->x_offset * scale;
		char_pos_y = cursor_y - (char_ptr->height + char_ptr->y_offset) * scale;
		char_pos_x2 = char_pos_x + char_ptr->width * scale;
		char_pos_y2 = char_pos_y + char_ptr->height * scale;
		
		char_coord_x = char_ptr->x * font_ptr->scale_x;
		char_coord_y = char_ptr->y * font_ptr->scale_y;
		char_coord_x2 = char_coord_x + char_ptr->width * font_ptr->scale_x;
		char_coord_y2 = char_coord_y + char_ptr->height * font_ptr->scale_y;
		
		current_quad = font_ptr->char_count * 8;
		
		font_ptr->font_vertices[current_quad] = char_pos_x;
		font_ptr->font_tex_coords[current_quad] = char_coord_x;
		current_quad ++;
		
		font_ptr->font_vertices[current_quad] = char_pos_y;
		font_ptr->font_tex_coords[current_quad] = char_coord_y2;
		current_quad ++;
		
		font_ptr->font_vertices[current_quad] = char_pos_x2;
		font_ptr->font_tex_coords[current_quad] = char_coord_x2;
		current_quad ++;
		
		font_ptr->font_vertices[current_quad] = char_pos_y;
		font_ptr->font_tex_coords[current_quad] = char_coord_y2;
		current_quad ++;

		font_ptr->font_vertices[current_quad] = char_pos_x2;
		font_ptr->font_tex_coords[current_quad] = char_coord_x2;
		current_quad ++;
		
		font_ptr->font_vertices[current_quad] = char_pos_y2;
		font_ptr->font_tex_coords[current_quad] = char_coord_y;
		current_quad ++;
		
		font_ptr->font_vertices[current_quad] = char_pos_x;
		font_ptr->font_tex_coords[current_quad] = char_coord_x;
		current_quad ++;
		
		font_ptr->font_vertices[current_quad] = char_pos_y2;
		font_ptr->font_tex_coords[current_quad] = char_coord_y;
		current_quad ++;
		
		font_ptr->font_colors[font_ptr->char_count * 4 + 0] = color;
		font_ptr->font_colors[font_ptr->char_count * 4 + 1] = color;
		font_ptr->font_colors[font_ptr->char_count * 4 + 2] = color;
		font_ptr->font_colors[font_ptr->char_count * 4 + 3] = color;
		

		cursor_x += char_ptr->x_advance * scale;
		
		for (int j = 0; j < char_ptr->kernings_count; j++)
		{
			if (font_ptr->kerning_cache_chars[char_ptr->kernings_first_index + j] == output_text[i + 1])
			{
				cursor_x += font_ptr->kerning_cache_amounts[char_ptr->kernings_first_index + j] * scale;
				break;
			}
		}
		
		font_ptr->char_count ++;
		line_char_count ++;
	}
}


void Font_Render()
{
	Render_EnableTextures();
	Render_EnableVertexArray();
	Render_EnableColorArray();
	Render_EnableTexCoordArray();
	Render_EnableIndexArray();

	for (int i = 0; i < fonts_count; i++)
	{
		if (fonts[i].char_count > 0)
		{
			TexManager_SetTextureByName(fonts[i].texture);

			Render_SetVertexArray(fonts[i].font_vertices, 2, TR_FLOAT, 0);
			Render_SetColorArray(fonts[i].font_colors, 4, TR_UNSIGNED_BYTE, 0);
			Render_SetTexCoordArray(fonts[i].font_tex_coords, 2, TR_FLOAT, 0);
			Render_SetIndexArray(font_index_array, TR_UNSIGNED_SHORT);
			
			Render_DrawArrays(TR_TRIANGLES, fonts[i].char_count * 6);
		}
		
		fonts[i].char_count = 0;
	}
}


void Font_GetTextSize(char *text, char *font_name, Vector2D *size)
{
	if (text == NULL)
		return;
	if (font_name == NULL)
		return;
	if (size == NULL)
		return;
	
	int current_font = -1;
	
	for (int i = 0; i < fonts_count; i++)
	{
		if (strcmp(fonts[i].name, font_name) == 0)
			current_font = i;
	}
	
	if (current_font < 0)
	{
		LogPrint("Error: font name not found!\n");
		return;
	}
	
	float current_width = 0.0f;
	float max_width = 0.0f;
	float current_height = 0.0f;
	
	Font *font_ptr = &fonts[current_font];
	FontCharacter *char_ptr;
	
	int text_lenght = strlen(text);
	
	U8 *output_text = (U8 *)text;
	
	int reading_color = FALSE;
	
	for (int i = 0; i <= text_lenght; i++)
	{
		if (reading_color)
		{
			reading_color = FALSE;
			continue;
		}
		
		if (output_text[i] == '#')
		{
			if (!reading_color)
			{
				reading_color = TRUE;
				continue;
			}
		}
		
		if (output_text[i] == '\n' || output_text[i] == '\0')
		{
			if (current_width > max_width)
				max_width = current_width;
			current_width = 0;
			current_height += font_ptr->line_height;
			
			continue;
		}
		
		if (output_text[i] < 32)
			continue;
		
		char_ptr = &font_ptr->font_chars[output_text[i]];
		
		current_width += char_ptr->x_advance;
		
		for (int j = 0; j < char_ptr->kernings_count; j++)
		{
			if (font_ptr->kerning_cache_chars[char_ptr->kernings_first_index + j] == output_text[i + 1])
			{
				current_width += font_ptr->kerning_cache_amounts[char_ptr->kernings_first_index + j];
				break;
			}
		}
	}
	
	size->x = max_width * font_ptr->default_scale;
	size->y = current_height * font_ptr->default_scale;
}


void Font_Release()
{
	for (int i = 0; i < fonts_count; i++)
		TexManager_RemoveTextureByName(fonts[i].texture);
}

