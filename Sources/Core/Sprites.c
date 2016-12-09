
#include "Common.h"

#include "FastMath.h"
#include "Vector.h"
#include "Render.h"
#include "TextureManager.h"
#include "Sprites.h"


#define MAX_SPRITE_DESCRIPTIONS_COUNT  256
#define MAX_SPRITES_COUNT              2048

typedef struct _SpriteDescription
{
	float width;
	float height;
	float tc_x;
	float tc_y;
	float tc_width;
	float tc_height;
	int texture_index; // TODO: init
	int priority; // TODO: init
}
SpriteDescription;

SpriteDescription sprite_descriptions[MAX_SPRITE_DESCRIPTIONS_COUNT];
int sprite_descriptions_count;

float sprites_vertices[MAX_SPRITES_COUNT * 8];
U32 sprites_colors[MAX_SPRITES_COUNT * 4];
float sprites_tex_coords[MAX_SPRITES_COUNT * 8];
U16 sprites_index_array[MAX_SPRITES_COUNT * 6];

U16 sprites_description_indices[MAX_SPRITES_COUNT]; // TODO: hmm, init or no
int sprites_count; // TODO: check for max count


void Sprites_Init()
{
	sprite_descriptions_count = 0;
	sprites_count = 0;
	
	for (int i = 0; i < MAX_SPRITES_COUNT; i ++) // TODO: should have some limit
	{
		sprites_index_array[i * 6 + 0] = (i * 4) + 0;
		sprites_index_array[i * 6 + 1] = (i * 4) + 1;
		sprites_index_array[i * 6 + 2] = (i * 4) + 2;
		sprites_index_array[i * 6 + 3] = (i * 4) + 2;
		sprites_index_array[i * 6 + 4] = (i * 4) + 3;
		sprites_index_array[i * 6 + 5] = (i * 4) + 0;
	}
}

void Sprites_AddSprite(SpriteHandler *sprite_handler, char *texture_name, float tc_x, float tc_y, float tc_width, float tc_height, int priority)
{
	if (sprite_handler == NULL)
		return;
	
	int current_sprite = sprite_descriptions_count;
	
	if (sprite_descriptions_count >= MAX_SPRITE_DESCRIPTIONS_COUNT)
	{
		LogPrint("Error: too many sprites!\n");
		sprite_handler = NULL;
		return;
	}

	sprite_descriptions[current_sprite].texture_index = TexManager_AddTexture(texture_name, 0);

	if (sprite_descriptions[current_sprite].texture_index < 0)
	{
		LogPrint("Error: unable to create sprite!\n");
		sprite_handler = NULL;
		return;
	}

	I32 width, height;
	TexManager_GetTextureResolutionByIndex(sprite_descriptions[current_sprite].texture_index, &width, &height);
		
	float scale_x = 1.0f / width;
	float scale_y = 1.0f / height;
	
	if (tc_width <= 0)
		tc_width = (float)width;
	if (tc_height <= 0)
		tc_height = (float)height;
	
	sprite_descriptions[current_sprite].width = tc_width;
	sprite_descriptions[current_sprite].height = tc_height;
	sprite_descriptions[current_sprite].tc_x = tc_x * scale_x;
	sprite_descriptions[current_sprite].tc_y = tc_y * scale_y;
	sprite_descriptions[current_sprite].tc_width = tc_width * scale_x;
	sprite_descriptions[current_sprite].tc_height = tc_height * scale_y;
	sprite_descriptions[current_sprite].priority = priority;
	
	sprite_descriptions_count ++;
	
	*sprite_handler = current_sprite;
}

void Sprites_DrawSprite(SpriteHandler *sprite_handler, float x, float y, float scale, float angle, U32 color, U32 flags)
{
	if (sprite_handler == NULL)
	{
		LogPrint("Error: sprite not found!\n");
		return;
	}
	
	if ((*sprite_handler < 0) || (*sprite_handler >= sprite_descriptions_count))
	{
		LogPrint("Error: sprite not found!\n");
		return;
	}

	if (sprites_count >= MAX_SPRITES_COUNT)
	{
		LogPrint("Error: sprites draw limit exceeded!\n");
		return;
	}

	sprites_description_indices[sprites_count] = *sprite_handler;

	SpriteDescription *sdptr = &sprite_descriptions[*sprite_handler]; // TODO: not id but index and get rid or redefine handler

	const float alpha = DEG2RAD(-angle); // TODO: hmmm... maybe rads only? Remove comment if no.
	
	float sin_alpha;
	float cos_alpha;
	
	if (alpha != 0.0f)
	{
		sin_alpha = sinf(alpha);
		cos_alpha = cosf(alpha);
	}
	else
	{
		sin_alpha = 0.0f;
		cos_alpha = 1.0f;
	}
	
	float sprite_rect_left;
	float sprite_rect_right;
	float sprite_rect_top;
	float sprite_rect_bottom;
	
	float sprite_tc_rect_left;
	float sprite_tc_rect_right;
	float sprite_tc_rect_top;
	float sprite_tc_rect_bottom;
	
	if (flags & SPRITE_ALIGN_LEFT)
	{
		sprite_rect_left = 0.0f;
		sprite_rect_right = sdptr->width * scale;
	}
	else if (flags & SPRITE_ALIGN_RIGHT)
	{
		sprite_rect_left = - sdptr->width * scale;
		sprite_rect_right = 0.0f;
	}
	else
	{
		sprite_rect_right = sdptr->width * 0.5f * scale;
		sprite_rect_left = - sprite_rect_right;
	}
	
	if (flags & SPRITE_ALIGN_DOWN)
	{
		sprite_rect_bottom = 0.0f;
		sprite_rect_top = sdptr->height * scale;
	}
	else if (flags & SPRITE_ALIGN_UP)
	{
		sprite_rect_bottom = - sdptr->height * scale;
		sprite_rect_top = 0.0f;
	}
	else
	{
		sprite_rect_top = sdptr->height * 0.5f * scale;
		sprite_rect_bottom = - sprite_rect_top;
	}
	
	if (flags & SPRITE_FLIP_X)
	{
		sprite_tc_rect_right = sdptr->tc_x;
		sprite_tc_rect_left = sdptr->tc_x + sdptr->tc_width;
	}
	else
	{
		sprite_tc_rect_left = sdptr->tc_x;
		sprite_tc_rect_right = sdptr->tc_x + sdptr->tc_width;
	}
	
	if (flags & SPRITE_FLIP_Y)
	{
		sprite_tc_rect_bottom = sdptr->tc_y;
		sprite_tc_rect_top = sdptr->tc_y + sdptr->tc_height;
	}
	else
	{
		sprite_tc_rect_top = sdptr->tc_y;
		sprite_tc_rect_bottom = sdptr->tc_y + sdptr->tc_height;
	}
		
	float *vptr = &sprites_vertices[sprites_count * 8];
	float *tcptr = &sprites_tex_coords[sprites_count * 8];
	U32 *cptr = &sprites_colors[sprites_count * 4];
		
	*vptr = sprite_rect_left * cos_alpha - sprite_rect_bottom * sin_alpha + x;		vptr++;
	*vptr = sprite_rect_left * sin_alpha + sprite_rect_bottom * cos_alpha + y;		vptr++;
	*vptr = sprite_rect_right * cos_alpha - sprite_rect_bottom * sin_alpha + x;		vptr++;
	*vptr = sprite_rect_right * sin_alpha + sprite_rect_bottom * cos_alpha + y;		vptr++;
	*vptr = sprite_rect_right * cos_alpha - sprite_rect_top * sin_alpha + x;		vptr++;
	*vptr = sprite_rect_right * sin_alpha + sprite_rect_top * cos_alpha + y;		vptr++;
	*vptr = sprite_rect_left * cos_alpha - sprite_rect_top * sin_alpha + x;			vptr++;
	*vptr = sprite_rect_left * sin_alpha + sprite_rect_top * cos_alpha + y;
		
	*tcptr = sprite_tc_rect_left;	tcptr++;
	*tcptr = sprite_tc_rect_bottom;	tcptr++;
	*tcptr = sprite_tc_rect_right;	tcptr++;
	*tcptr = sprite_tc_rect_bottom;	tcptr++;
	*tcptr = sprite_tc_rect_right;	tcptr++;
	*tcptr = sprite_tc_rect_top;	tcptr++;
	*tcptr = sprite_tc_rect_left;	tcptr++;
	*tcptr = sprite_tc_rect_top;
		
	*cptr = color;					cptr++;
	*cptr = color;					cptr++;
	*cptr = color;					cptr++;
	*cptr = color;

	sprites_count ++;
}

void Sprites_DrawSpriteEx(SpriteHandler *sprite_handler, float x, float y, float scale_x, float scale_y, float tc_bias_x, float tc_bias_y, float tc_scale_x, float tc_scale_y, float angle, U32 color, U32 flags)
{
	if (sprite_handler == NULL)
	{
		LogPrint("Error: sprite not found!\n");
		return;
	}
	
	if ((*sprite_handler < 0) || (*sprite_handler >= sprite_descriptions_count))
	{
		LogPrint("Error: sprite not found!\n");
		return;
	}

	if (sprites_count >= MAX_SPRITES_COUNT)
	{
		LogPrint("Error: sprites draw limit exceeded!\n");
		return;
	}

	sprites_description_indices[sprites_count] = *sprite_handler;

	SpriteDescription *sdptr = &sprite_descriptions[*sprite_handler];
	
	const float alpha = DEG2RAD(-angle);
	
	float sin_alpha;
	float cos_alpha;
	
	if (alpha != 0.0f)
	{
		sin_alpha = sinf(alpha);
		cos_alpha = cosf(alpha);
	}
	else
	{
		sin_alpha = 0.0f;
		cos_alpha = 1.0f;
	}
	
	float sprite_rect_left;
	float sprite_rect_right;
	float sprite_rect_top;
	float sprite_rect_bottom;
	
	float sprite_tc_rect_left;
	float sprite_tc_rect_right;
	float sprite_tc_rect_top;
	float sprite_tc_rect_bottom;
	
	if (flags & SPRITE_ALIGN_LEFT)
	{
		sprite_rect_left = 0.0f;
		sprite_rect_right = sdptr->width * scale_x;
	}
	else if (flags & SPRITE_ALIGN_RIGHT)
	{
		sprite_rect_left = - sdptr->width * scale_x;
		sprite_rect_right = 0.0f;
	}
	else
	{
		sprite_rect_right = sdptr->width * 0.5f * scale_x;
		sprite_rect_left = - sprite_rect_right;
	}
	
	if (flags & SPRITE_ALIGN_DOWN)
	{
		sprite_rect_bottom = 0.0f;
		sprite_rect_top = sdptr->height * scale_y;
	}
	else if (flags & SPRITE_ALIGN_UP)
	{
		sprite_rect_bottom = - sdptr->height * scale_y;
		sprite_rect_top = 0.0f;
	}
	else
	{
		sprite_rect_top = sdptr->height * 0.5f * scale_y;
		sprite_rect_bottom = - sprite_rect_top;
	}
	
	if (flags & SPRITE_FLIP_X)
	{
		sprite_tc_rect_right = sdptr->tc_x + tc_bias_x * sdptr->tc_width;
		sprite_tc_rect_left = sdptr->tc_x + tc_scale_x * sdptr->tc_width;
	}
	else
	{
		sprite_tc_rect_left = sdptr->tc_x + tc_bias_x * sdptr->tc_width;
		sprite_tc_rect_right = sdptr->tc_x + tc_scale_x * sdptr->tc_width;
	}
	
	if (flags & SPRITE_FLIP_Y)
	{
		sprite_tc_rect_bottom = sdptr->tc_y + tc_bias_y * sdptr->tc_height;
		sprite_tc_rect_top = sdptr->tc_y + tc_scale_y * sdptr->tc_height;
	}
	else
	{
		sprite_tc_rect_top = sdptr->tc_y + tc_bias_y * sdptr->tc_height;
		sprite_tc_rect_bottom = sdptr->tc_y + tc_scale_y * sdptr->tc_height;
	}
		
	float *vptr = &sprites_vertices[sprites_count * 8];
	float *tcptr = &sprites_tex_coords[sprites_count * 8];
	U32 *cptr = &sprites_colors[sprites_count * 4];
		
	*vptr = sprite_rect_left * cos_alpha - sprite_rect_bottom * sin_alpha + x;		vptr++;
	*vptr = sprite_rect_left * sin_alpha + sprite_rect_bottom * cos_alpha + y;		vptr++;
	*vptr = sprite_rect_right * cos_alpha - sprite_rect_bottom * sin_alpha + x;		vptr++;
	*vptr = sprite_rect_right * sin_alpha + sprite_rect_bottom * cos_alpha + y;		vptr++;
	*vptr = sprite_rect_right * cos_alpha - sprite_rect_top * sin_alpha + x;		vptr++;
	*vptr = sprite_rect_right * sin_alpha + sprite_rect_top * cos_alpha + y;		vptr++;
	*vptr = sprite_rect_left * cos_alpha - sprite_rect_top * sin_alpha + x;			vptr++;
	*vptr = sprite_rect_left * sin_alpha + sprite_rect_top * cos_alpha + y;
		
	*tcptr = sprite_tc_rect_left;	tcptr++;
	*tcptr = sprite_tc_rect_bottom;	tcptr++;
	*tcptr = sprite_tc_rect_right;	tcptr++;
	*tcptr = sprite_tc_rect_bottom;	tcptr++;
	*tcptr = sprite_tc_rect_right;	tcptr++;
	*tcptr = sprite_tc_rect_top;	tcptr++;
	*tcptr = sprite_tc_rect_left;	tcptr++;
	*tcptr = sprite_tc_rect_top;
		
	*cptr = color;					cptr++;
	*cptr = color;					cptr++;
	*cptr = color;					cptr++;
	*cptr = color;

	sprites_count ++;
}

void Sprites_GetSpriteSize(SpriteHandler *sprite_handler, Vector2D *size)
{
	if (sprite_handler == NULL)
	{
		LogPrint("Error: sprite not found!\n");
		return;
	}
	
	if ((*sprite_handler < 0) || (*sprite_handler >= sprite_descriptions_count))
	{
		LogPrint("Error: sprite not found!\n");
		return;
	}
	
	size->x = sprite_descriptions[*sprite_handler].width;
	size->y = sprite_descriptions[*sprite_handler].height;
}

void Sprites_SetSpriteSize(SpriteHandler *sprite_handler, const Vector2D *size)
{
	if (sprite_handler == NULL)
	{
		LogPrint("Error: sprite not found!\n");
		return;
	}
	
	if ((*sprite_handler < 0) || (*sprite_handler >= sprite_descriptions_count))
	{
		LogPrint("Error: sprite not found!\n");
		return;
	}
	
	sprite_descriptions[*sprite_handler].width = size->x;
	sprite_descriptions[*sprite_handler].height = size->y;
}

void Sprites_Render()
{
	int priorities[MAX_SPRITE_DESCRIPTIONS_COUNT];
	int priorities_count = 0;
	
	for (int i = 0; i < sprites_count; i ++) // make list of priorities
	{
		int current_priority = sprite_descriptions[sprites_description_indices[i]].priority;

		BOOL priority_not_found = TRUE;

		for (int j = 0; j < priorities_count; j ++)
		{
			if (current_priority == priorities[j])
				priority_not_found = FALSE;
		}

		if (priority_not_found)
		{
			priorities[priorities_count] = current_priority;
			priorities_count ++;
		}
	}

	for (int j = 0; j < priorities_count - 1; j ++) // sort this list in numerical order
	{
		int current_min_index = j;

		for (int i = j + 1; i < priorities_count; i ++)
		{
			if (priorities[i] < priorities[current_min_index])
				current_min_index = i;
		}

		if (current_min_index != j)
		{
			int tmp = priorities[j];
			priorities[j] = priorities[current_min_index];
			priorities[current_min_index] = tmp;
		}
    }

	Render_EnableTextures();
	Render_EnableVertexArray();
	Render_EnableColorArray();
	Render_EnableTexCoordArray();
	Render_EnableIndexArray();

	Render_SetVertexArray(sprites_vertices, 2, TR_FLOAT, 0);
	Render_SetColorArray(sprites_colors, 4, TR_UNSIGNED_BYTE, 0);
	Render_SetTexCoordArray(sprites_tex_coords, 2, TR_FLOAT, 0);

	int previous_sprite_description_index = -1;
	int first_index;
	int indices_count;

	for (int j = 0; j < priorities_count; j ++) // go through priorities and render them separately
	{
		for (int i = 0; i < sprites_count; i ++)
		{
			SpriteDescription *sdptr = &sprite_descriptions[sprites_description_indices[i]];

			if (sdptr->priority == priorities[j])
			{
				//if (sprites_description_indices[i] != previous_sprite_description_index)
				//{
					TexManager_SetTextureByIndex(sdptr->texture_index);

					first_index = i;
					indices_count = 1;

					Render_SetIndexArray(&sprites_index_array[first_index * 6], TR_UNSIGNED_SHORT);
					Render_DrawArrays(TR_TRIANGLES, indices_count * 6);

					previous_sprite_description_index = sprites_description_indices[i];
				//}
			}




			/*if (sdptr->priority == priorities[j])
			{
				if (previous_texture_index != sdptr->texture_index)
				{
					TexManager_SetTextureByIndex(sdptr->texture_index);

					previous_texture_index = sdptr->texture_index;

					first_index = i;
					indices_count = 0;
				}

				indices_count ++;

				BOOL render_now = TRUE;

				if (i + 1 < sprites_count)
				{
					if (sdptr->texture_index == sprite_descriptions[sprites_description_indices[i + 1]].texture_index)
						render_now = FALSE;

					if (priorities[j] != sprite_descriptions[sprites_description_indices[i + 1]].priority)
						render_now = TRUE;
				}

				if (render_now)
				{
					LogPrint("Sprites from %d count %d texture %d priority %d \n",first_index, indices_count, sdptr->texture_index, sdptr->priority);
					Render_SetIndexArray(&sprites_index_array[first_index * 6], TR_UNSIGNED_SHORT);
					Render_DrawArrays(TR_TRIANGLES, indices_count * 6);
					indices_count = 0;
					first_index = i + 1; // TODO: bad
				}
			}*/
		}
	}

	//LogPrint("\n\n\n");

	sprites_count = 0;
}

void Sprites_Release()
{
	for (int i = 0; i < sprite_descriptions_count; i ++)
		TexManager_RemoveTextureByIndex(sprite_descriptions[i].texture_index);
}

