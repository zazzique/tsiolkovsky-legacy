//
//  GUIControls.m
//  Carnivores
//
//  Created by Dmitry Nechay on 25.08.09.
//  Copyright 2009 Action Forms. All rights reserved.
//

#include "Common.h"

#include "FastMath.h"
#include "Vector.h"
#include "TextureManager.h"
#include "Sprites.h"
#include "Font.h"
#include "Render.h"
#include "GUIControls.h"

#define MAX_GUI_CONTROLS_COUNT 128
#define MAX_GUI_TOUCHES_SUPPORTED 16

#define GUI_CONTROLLER_INNER_RADIUS 40.0f

#define GUI_CONTROL_NONE -500
#define GUI_CONTROL_CANCELED -499
#define GUI_CONTROL_BACKGROUND -1

#define GUI_QUICK_TOUCH_TIME 0.15f

typedef struct _GUIControl
{
	int group;
	U32 subgroup_flags;
	int type;
	float x;
	float y;
	float x2;
	float y2;
	float width;
	float height;
	U32 flags;
	U32 color;
	float scale;
	BOOL use_sprites;
	BOOL use_text;
	BOOL active;
	BOOL visible;
	BOOL touch_began;
	BOOL touch_done;
	BOOL dynamic_size;
	SpriteHandler sprite_1;
	SpriteHandler sprite_2;
	char font_name[32];
	float font_scale;
	char text[256];
	float text_pos_x;
	float text_pos_y;
	float slider_value;
	float slider_min;
	float slider_max;
    
} GUIControl;

GUIControl gui_controls[MAX_GUI_CONTROLS_COUNT];
int gui_controls_count;

BOOL gui_lock;

int gui_active_group;
U32 gui_active_subgroups;

int gui_touched_controls[MAX_GUI_TOUCHES_SUPPORTED];
Vector2D gui_touched_locations[MAX_GUI_TOUCHES_SUPPORTED];
Vector2D gui_touched_start_locations[MAX_GUI_TOUCHES_SUPPORTED];

int gui_event_id;
int gui_event_activated;

float gui_fade_time_in;
float gui_fade_time_out;
float gui_fade_time;

int quick_touch;
float quick_touch_time;

float gui_fade_vertices[8];

BOOL gui_enable_touches_reset;


void GUI_Init()
{
	gui_enable_touches_reset = TRUE;

    gui_lock = FALSE;
	
	gui_controls_count = 0;
	gui_active_group = 0;
	gui_active_subgroups = GUI_SUBGROUP_DEFAULT;
	
	for (int i = 0; i < MAX_GUI_CONTROLS_COUNT; i++)
	{
		gui_controls[i].touch_began = FALSE;
		gui_controls[i].touch_done = FALSE;
	}
	
	for (int i = 0; i < MAX_GUI_TOUCHES_SUPPORTED; i++)
	{
		gui_touched_controls[i] = GUI_CONTROL_NONE;
	}
	
	gui_event_id = -1;
	gui_event_activated = FALSE;
	
	gui_fade_time_in = 0.0f;
	gui_fade_time_out = 0.0f;
	gui_fade_time = 0.0f;
	
	quick_touch = FALSE;
	quick_touch_time = 0.0f;
	
	gui_fade_vertices[0] = 0.0f;
	gui_fade_vertices[1] = 0.0f;
	gui_fade_vertices[2] = (float)v_sx;
	gui_fade_vertices[3] = 0.0f;
	gui_fade_vertices[4] = 0.0f;
	gui_fade_vertices[5] = (float)v_sy;
	gui_fade_vertices[6] = (float)v_sx;
	gui_fade_vertices[7] = (float)v_sy;
}

void GUI_Lock()
{
    gui_lock = TRUE;
}

void GUI_Unlock()
{
    gui_lock = FALSE;
}

void GUI_AddControl(GUIControlId *control_id, int group, U32 subgroups, int type, Vector2D *pos, Vector2D *size, U32 flags, SpriteHandler *sprite_1, SpriteHandler *sprite_2, const char *text, const char *font_name, Vector2D *text_pos)
{
	if (gui_controls_count >= MAX_GUI_CONTROLS_COUNT)
	{
		LogPrint("Error: too many GUI elements!\n");
		return;
	}
	
	*control_id = gui_controls_count;
	
	gui_controls[gui_controls_count].group = group;
	
	gui_controls[gui_controls_count].subgroup_flags = subgroups;
	
	gui_controls[gui_controls_count].type = type;
	
	if (pos != NULL)
	{
		gui_controls[gui_controls_count].x = pos->x;
		gui_controls[gui_controls_count].y = pos->y;
	}
	else
	{
		gui_controls[gui_controls_count].x = 0.0f;
		gui_controls[gui_controls_count].y = 0.0f;
	}
	
	gui_controls[gui_controls_count].x2 = 0.0f;
	gui_controls[gui_controls_count].y2 = 0.0f;
	
	if (size != NULL)
	{
		gui_controls[gui_controls_count].width = size->x;
		gui_controls[gui_controls_count].height = size->y;
		
		gui_controls[gui_controls_count].dynamic_size = FALSE;
	}
	else
	{
		gui_controls[gui_controls_count].width = 0.0f;
		gui_controls[gui_controls_count].height = 0.0f;
		
		gui_controls[gui_controls_count].dynamic_size = TRUE;
	}
	
	if (text_pos != NULL)
	{
		gui_controls[gui_controls_count].text_pos_x = text_pos->x;
		gui_controls[gui_controls_count].text_pos_y = text_pos->y;
	}
	else
	{
		gui_controls[gui_controls_count].text_pos_x = 0.0f;
		gui_controls[gui_controls_count].text_pos_y = 0.0f;
	}
	
	gui_controls[gui_controls_count].flags = flags;
	
	gui_controls[gui_controls_count].color = 0xffffffff;
	
	gui_controls[gui_controls_count].scale = 1.0f;
	
	gui_controls[gui_controls_count].font_scale = 1.0f;
	
	if (sprite_1 == NULL)
	{
		gui_controls[gui_controls_count].use_sprites = FALSE;
	}
	else
	{
		gui_controls[gui_controls_count].use_sprites = TRUE;
		
		gui_controls[gui_controls_count].sprite_1 = *sprite_1;
		
		if (sprite_2 == NULL)
		{
			gui_controls[gui_controls_count].sprite_2 = *sprite_1;
		}
		else
		{
			gui_controls[gui_controls_count].sprite_2 = *sprite_2;
		}
	}
	
	if (font_name == NULL)
	{
		gui_controls[gui_controls_count].use_text = FALSE;
	}
	else
	{
		gui_controls[gui_controls_count].use_text = TRUE;
		
		strcpy(gui_controls[gui_controls_count].font_name, font_name);
		strcpy(gui_controls[gui_controls_count].text, text);
	}
	
	gui_controls[gui_controls_count].touch_began = FALSE;
	gui_controls[gui_controls_count].touch_done = FALSE;
	
	gui_controls[gui_controls_count].active = TRUE;
	gui_controls[gui_controls_count].visible = TRUE;
	
	gui_controls[gui_controls_count].slider_value = 0;
	gui_controls[gui_controls_count].slider_min = -1;
	gui_controls[gui_controls_count].slider_max = 1;
	
	if (gui_controls[gui_controls_count].dynamic_size)
	{
		Vector2D dynamic_size;
		
		if (gui_controls[gui_controls_count].use_sprites)
		{
			Sprites_GetSpriteSize(&gui_controls[gui_controls_count].sprite_1, &dynamic_size);
			
			gui_controls[gui_controls_count].width = dynamic_size.x;
			gui_controls[gui_controls_count].height = dynamic_size.y;
		}
		else if (gui_controls[gui_controls_count].use_text)
		{
			Font_GetTextSize(gui_controls[gui_controls_count].text, gui_controls[gui_controls_count].font_name, &dynamic_size);
			
			gui_controls[gui_controls_count].width = dynamic_size.x;
			gui_controls[gui_controls_count].height = dynamic_size.y;
		}
	}
	
	gui_controls_count ++;
}


void GUI_SetControlPos(GUIControlId control_id, float x, float y)
{
	if (control_id < 0 || control_id >= gui_controls_count)
		return;
	
	gui_controls[control_id].x = x;
	gui_controls[control_id].y = y;
}


void GUI_SetControlFlags(GUIControlId control_id, U32 flags)
{
	if (control_id < 0 || control_id >= gui_controls_count)
		return;
	
	gui_controls[control_id].flags = flags;
}


void GUI_SetControlColor(GUIControlId control_id, U32 color)
{
	if (control_id < 0 || control_id >= gui_controls_count)
		return;
	
	gui_controls[control_id].color = color;
}


void GUI_SetControlScale(GUIControlId control_id, float scale)
{
	if (control_id < 0 || control_id >= gui_controls_count)
		return;
	
	gui_controls[control_id].scale = scale;
}


void GUI_SetControlText(GUIControlId control_id, char *text)
{
	if (control_id < 0 || control_id >= gui_controls_count)
		return;
	
	if (text != NULL)
		strcpy(gui_controls[control_id].text, text);
	
	if (!gui_controls[control_id].use_sprites && gui_controls[control_id].use_text)
	{
		Vector2D dynamic_size;
		
		Font_GetTextSize(gui_controls[control_id].text, gui_controls[control_id].font_name, &dynamic_size);
		
		gui_controls[control_id].width = dynamic_size.x;
		gui_controls[control_id].height = dynamic_size.y;
	}
}


void GUI_SetControlFontScale(GUIControlId control_id, float scale)
{
	if (control_id < 0 || control_id >= gui_controls_count)
		return;
	
	gui_controls[control_id].font_scale = scale;

	if (!gui_controls[control_id].use_sprites && gui_controls[control_id].use_text)
	{
		Vector2D dynamic_size;
		
		Font_GetTextSize(gui_controls[control_id].text, gui_controls[control_id].font_name, &dynamic_size);
        
        dynamic_size.x *= scale;
        dynamic_size.y *= scale;
		
		gui_controls[control_id].width = dynamic_size.x;
		gui_controls[control_id].height = dynamic_size.y;
	}
}


void GUI_SetControlSprites(GUIControlId control_id, SpriteHandler *sprite_1, SpriteHandler *sprite_2)
{
	if (control_id < 0 || control_id >= gui_controls_count)
		return;
	
	if (sprite_1 == NULL)
		return;
	
	gui_controls[control_id].sprite_1 = *sprite_1;
	
	if (sprite_2 == NULL)
	{
		gui_controls[control_id].sprite_2 = *sprite_1;
	}
	else
	{
		gui_controls[control_id].sprite_2 = *sprite_2;
	}
	
	if (gui_controls[control_id].use_sprites)
	{
		Vector2D dynamic_size;
		
		Sprites_GetSpriteSize(&gui_controls[control_id].sprite_1, &dynamic_size);
		
		gui_controls[control_id].width = dynamic_size.x;
		gui_controls[control_id].height = dynamic_size.y;
	}
}


void GUI_SetControlActive(GUIControlId control_id, BOOL active)
{
	if (control_id < 0 || control_id >= gui_controls_count)
		return;
	
	gui_controls[control_id].active = active;
}


void GUI_SetControlVisible(GUIControlId control_id, BOOL visible)
{
	if (control_id < 0 || control_id >= gui_controls_count)
		return;
	
	gui_controls[control_id].visible = visible;
}


void GUI_SetSliderValue(GUIControlId control_id, float value)
{
	Vector2D sprite_1_size, sprite_2_size;
	float slider_k;
	
	if (control_id < 0 || control_id >= gui_controls_count)
		return;
	
	if (!gui_controls[control_id].use_sprites)
		return;
	
	gui_controls[control_id].slider_value = value;
		
	Sprites_GetSpriteSize(&gui_controls[control_id].sprite_1, &sprite_1_size);
	Sprites_GetSpriteSize(&gui_controls[control_id].sprite_2, &sprite_2_size);
		
	float div_tmp = sprite_1_size.x - 2.0f - sprite_2_size.x;
	if (div_tmp == 0.0f)
		div_tmp = FLT_EPSILON;
	slider_k = (gui_controls[control_id].slider_max - gui_controls[control_id].slider_min) / div_tmp;
	gui_controls[control_id].x2 = (gui_controls[control_id].slider_value - gui_controls[control_id].slider_min) / slider_k + 1.0f;
	gui_controls[control_id].y2 = (sprite_1_size.y - sprite_2_size.y) * 0.5f;
}


void GUI_SetSliderParams(GUIControlId control_id, float min, float max)
{
	if (control_id < 0 || control_id >= gui_controls_count)
		return;
	
	gui_controls[control_id].slider_min = min;
	gui_controls[control_id].slider_max = max;
}


void GUI_GetControlPos(GUIControlId control_id, float *x, float *y)
{
	if (control_id < 0 || control_id >= gui_controls_count)
		return;
	
	*x = gui_controls[control_id].x;
	*y = gui_controls[control_id].y;
}


void GUI_DrawControls()
{
	U32 flags;
    float dx, dy;
	
	for (int i = 0; i < gui_controls_count; i ++)
	{
		if (gui_controls[i].group != gui_active_group)
			continue;
		
		if (!(gui_controls[i].subgroup_flags & gui_active_subgroups))
			continue;
		
		if (!gui_controls[i].visible)
			continue;
		
		if (gui_controls[i].use_sprites)
		{
            flags = SPRITE_ALIGN_LEFT | SPRITE_ALIGN_DOWN;
            
            dx = 0;
            dy = 0;
            
			if (gui_controls[i].flags & GUI_CONTROL_ALIGN_LEFT)
                dx = 0;
			if (gui_controls[i].flags & GUI_CONTROL_ALIGN_RIGHT)
				dx = -gui_controls[i].width * gui_controls[i].scale;
			if (gui_controls[i].flags & GUI_CONTROL_ALIGN_CENTER)
				dx = -gui_controls[i].width * 0.5f * gui_controls[i].scale;
			if (gui_controls[i].flags & GUI_CONTROL_VERTICAL_CENTERED)
				dy = -gui_controls[i].height * 0.5f * gui_controls[i].scale;
            
			if (gui_controls[i].type == GUI_LABEL)
			{
				Sprites_DrawSprite(&gui_controls[i].sprite_1, gui_controls[i].x, gui_controls[i].y, 1.0f, 0.0f, gui_controls[i].color, flags);
			}
			else if (gui_controls[i].type == GUI_BUTTON)
			{
				Sprites_DrawSprite(gui_controls[i].touch_began ? &gui_controls[i].sprite_2 : &gui_controls[i].sprite_1, gui_controls[i].x + dx, gui_controls[i].y + dy, gui_controls[i].scale, 0.0f, gui_controls[i].color, flags);
			}
			else if (gui_controls[i].type == GUI_CONTROLLER)
			{
				Sprites_DrawSprite(&gui_controls[i].sprite_1, gui_controls[i].x + dx, gui_controls[i].y + dy, gui_controls[i].scale, 0.0f, gui_controls[i].color, flags);
				Sprites_DrawSprite(&gui_controls[i].sprite_2, gui_controls[i].x2 + gui_controls[i].x + dx, gui_controls[i].y2 + gui_controls[i].y + dy, gui_controls[i].scale, 0.0f, gui_controls[i].color, flags);
			}
			else if (gui_controls[i].type == GUI_SLIDER)
			{
				Sprites_DrawSprite(&gui_controls[i].sprite_1, gui_controls[i].x + dx, gui_controls[i].y + dy, gui_controls[i].scale, 0.0f, gui_controls[i].color, flags);
				Sprites_DrawSprite(&gui_controls[i].sprite_2, gui_controls[i].x2 + gui_controls[i].x + dx, gui_controls[i].y2 + gui_controls[i].y + dy, gui_controls[i].scale, 0.0f, gui_controls[i].color, flags);
			}
		}
		if (gui_controls[i].use_text)
		{
			flags = 0x00000000;
			
			if (gui_controls[i].flags & GUI_CONTROL_ALIGN_LEFT)
				flags |= TEXT_ALIGN_LEFT;
			if (gui_controls[i].flags & GUI_CONTROL_ALIGN_RIGHT)
				flags |= TEXT_ALIGN_RIGHT;
			if (gui_controls[i].flags & GUI_CONTROL_ALIGN_CENTER)
				flags |= TEXT_ALIGN_CENTER;
			if (gui_controls[i].flags & GUI_CONTROL_VERTICAL_CENTERED)
				flags |= TEXT_VERTICAL_CENTERED;
			
			if (gui_controls[i].type == GUI_LABEL)
			{
				Font_PrintText(gui_controls[i].x + gui_controls[i].text_pos_x, gui_controls[i].y + gui_controls[i].text_pos_y, gui_controls[i].font_scale, gui_controls[i].color, gui_controls[i].text, flags, gui_controls[i].font_name);
			}
			else if (gui_controls[i].type == GUI_BUTTON)
			{
				Font_PrintText(gui_controls[i].x + gui_controls[i].text_pos_x, gui_controls[i].y + gui_controls[i].text_pos_y, gui_controls[i].font_scale, gui_controls[i].touch_began ? 0xff0040ff : gui_controls[i].color, gui_controls[i].text, flags, gui_controls[i].font_name);
			}
		}
	}
}


void GUI_SetActiveGroup(int group, U32 subgroups)
{
	gui_active_group = group;
	gui_active_subgroups = subgroups;
}


int GUI_GetUnusedTouch()
{
	for (int i = 0; i < MAX_GUI_TOUCHES_SUPPORTED; i++)
		if (gui_touched_controls[i] == GUI_CONTROL_NONE)
			return i;
	
	return 0;
}

int GUI_GetTouchByLocation(float x, float y)
{
	float min_distance = 1024;
	float distance = 0;
	int nearest_touch = -1;
	
	for (int i = 0; i < MAX_GUI_TOUCHES_SUPPORTED; i++)
	{
		if (gui_touched_controls[i] == GUI_CONTROL_NONE)
			continue;
		
		distance = sqrtf(SQR(x - gui_touched_locations[i].x) + SQR(y - gui_touched_locations[i].y));
		
		if (min_distance > distance)
		{
			min_distance = distance;
			nearest_touch = i;
		}
	}
	
	return nearest_touch;
}

void GUI_TouchBegan(float x, float y)
{
    int touch_id = GUI_GetUnusedTouch();
    
	if ((touch_id < 0) || (touch_id >= MAX_GUI_TOUCHES_SUPPORTED))
		return;
    
	BOOL is_background = TRUE;
	
	if (gui_lock || (gui_fade_time > 0.0f))
		return;
    
	for (int i = 0; i < gui_controls_count; i ++)
	{
		if (gui_controls[i].group != gui_active_group)
			continue;
		
		if (!(gui_controls[i].subgroup_flags & gui_active_subgroups))
			continue;
		
		if (!gui_controls[i].visible)
			continue;
		
		if (!gui_controls[i].active)
			continue;
        
		if (gui_controls[i].type == GUI_LABEL)
			continue;
        
		if (GUI_PointInControl(i, x, y))
		{
			gui_controls[i].touch_began = TRUE;
			
			gui_touched_controls[touch_id] = i;
			is_background = FALSE;
		}
	}
    
	if (is_background)
	{
		gui_touched_controls[touch_id] = GUI_CONTROL_BACKGROUND;
		
		quick_touch_time = 0.0f;
	}
    
	gui_touched_locations[touch_id].x = x;
	gui_touched_locations[touch_id].y = y;
    
	gui_touched_start_locations[touch_id].x = x;
	gui_touched_start_locations[touch_id].y = y;
}

void GUI_TouchMoved(int touch_id, float x, float y)
{
	if ((touch_id < 0) || (touch_id >= MAX_GUI_TOUCHES_SUPPORTED))
		return;
    
	gui_touched_locations[touch_id].x = x;
	gui_touched_locations[touch_id].y = y;
    
    if (gui_touched_controls[touch_id] == GUI_CONTROL_NONE)
	{
		return;
	}
    
	if (gui_touched_controls[touch_id] == GUI_CONTROL_BACKGROUND)
	{
		return;
	}
    
	if (gui_controls[gui_touched_controls[touch_id]].group != gui_active_group)
	{
		gui_touched_controls[touch_id] = GUI_CONTROL_NONE;
		return;
	}
	if (!(gui_controls[gui_touched_controls[touch_id]].subgroup_flags & gui_active_subgroups))
	{
		gui_touched_controls[touch_id] = GUI_CONTROL_NONE;
		return;
	}
	if (!gui_controls[gui_touched_controls[touch_id]].visible)
	{
		gui_touched_controls[touch_id] = GUI_CONTROL_NONE;
		return;
	}
	if (!gui_controls[gui_touched_controls[touch_id]].active)
	{
		gui_touched_controls[touch_id] = GUI_CONTROL_NONE;
		return;
	}
	if (gui_touched_controls[touch_id] != GUI_CONTROL_NONE)
	{
		if (GUI_PointInControl(gui_touched_controls[touch_id], x, y))
		{
			gui_controls[gui_touched_controls[touch_id]].touch_began = TRUE;
		}
		else
		{
			gui_controls[gui_touched_controls[touch_id]].touch_began = FALSE;
		}
	}
}

void GUI_TouchEnded(int touch_id, float x, float y)
{
	if ((touch_id < 0) || (touch_id >= MAX_GUI_TOUCHES_SUPPORTED))
		return;
    
	if (gui_touched_controls[touch_id] == GUI_CONTROL_BACKGROUND)
	{
		gui_touched_locations[touch_id] = gui_touched_start_locations[touch_id];
		
		if (quick_touch_time <= GUI_QUICK_TOUCH_TIME)
		{
			quick_touch = TRUE;
		}
        
		quick_touch_time = 0.0f;
	}
	else if ((gui_touched_controls[touch_id] != GUI_CONTROL_NONE) && (gui_touched_controls[touch_id] >= 0))
	{
		if (GUI_PointInControl(gui_touched_controls[touch_id], x, y))
		{
			if (gui_controls[gui_touched_controls[touch_id]].group != gui_active_group)
			{
				gui_controls[gui_touched_controls[touch_id]].touch_began = FALSE;
				gui_controls[gui_touched_controls[touch_id]].touch_done = FALSE;
			}
			else if (!(gui_controls[gui_touched_controls[touch_id]].subgroup_flags & gui_active_subgroups))
			{
				gui_controls[gui_touched_controls[touch_id]].touch_began = FALSE;
				gui_controls[gui_touched_controls[touch_id]].touch_done = FALSE;
			}
			else
			{
				gui_controls[gui_touched_controls[touch_id]].touch_began = FALSE;
				gui_controls[gui_touched_controls[touch_id]].touch_done = TRUE;
			}
		}
		else
		{
			gui_controls[gui_touched_controls[touch_id]].touch_began = FALSE;
			gui_controls[gui_touched_controls[touch_id]].touch_done = FALSE;
		}
	}
    
	gui_touched_controls[touch_id] = GUI_CONTROL_NONE;
}

void GUI_TouchesReset()
{
	if (!gui_enable_touches_reset)
		return;

	for (int i = 0; i < MAX_GUI_TOUCHES_SUPPORTED; i++)
		gui_touched_controls[i] = GUI_CONTROL_NONE;
}

void GUI_GetBackgroundMovements(float *x, float *y)
{
	float x_r = 0.0f;
	float y_r = 0.0f;
	
	for (int i = 0; i < MAX_GUI_TOUCHES_SUPPORTED; i++)
    {
		if (gui_touched_controls[i] == GUI_CONTROL_BACKGROUND)
		{
			x_r += gui_touched_locations[i].x - gui_touched_start_locations[i].x;
			y_r += gui_touched_locations[i].y - gui_touched_start_locations[i].y;
			
			gui_touched_start_locations[i] = gui_touched_locations[i];
		}
    }

	*x = x_r;
	*y = y_r;
}

BOOL GUI_GetQuickTouch()
{
	int quick_touch_tmp = quick_touch;
	quick_touch = FALSE;
	
	return quick_touch_tmp;
}

BOOL GUI_PointInControl(GUIControlId control_id, float x, float y)
{
	BOOL result = FALSE;
	
	if (control_id < 0 || control_id >= gui_controls_count)
		return FALSE;
	
	float rect_left;
	float rect_right;
	float rect_top;
	float rect_bottom;
	
	if (gui_controls[control_id].type == GUI_BUTTON || gui_controls[control_id].type == GUI_CONTROLLER || gui_controls[control_id].type == GUI_SLIDER)
	{
		rect_left = gui_controls[control_id].x;
		rect_bottom = gui_controls[control_id].y;
		rect_right = gui_controls[control_id].x + gui_controls[control_id].width * gui_controls[control_id].scale;
		rect_top = gui_controls[control_id].y + gui_controls[control_id].height * gui_controls[control_id].scale;
		
		if (gui_controls[control_id].type == GUI_SLIDER)
		{
			rect_left -= 32;
			rect_bottom -= 8;
			rect_right += 32;
			rect_top += 8;
		}
		
		if (!gui_controls[control_id].use_sprites && gui_controls[control_id].use_text)
		{
			rect_bottom -= 2;
			rect_top += 6;
		}
		
		if (gui_controls[control_id].flags & GUI_CONTROL_ALIGN_RIGHT)
		{
			rect_left -= gui_controls[control_id].width * gui_controls[control_id].scale;
			rect_right -= gui_controls[control_id].width * gui_controls[control_id].scale;
		}
		if (gui_controls[control_id].flags & GUI_CONTROL_ALIGN_CENTER)
		{
			rect_left -= gui_controls[control_id].width * 0.5f * gui_controls[control_id].scale;
			rect_right -= gui_controls[control_id].width * 0.5f * gui_controls[control_id].scale;
		}
		if (gui_controls[control_id].flags & GUI_CONTROL_VERTICAL_CENTERED)
		{
			rect_top -= gui_controls[control_id].height * 0.5f * gui_controls[control_id].scale;
			rect_bottom -= gui_controls[control_id].height * 0.5f * gui_controls[control_id].scale;
		}
		
		if (x > rect_left && x < rect_right)
			if (y > rect_bottom && y < rect_top)
				result = TRUE;
	}
	
	return result;
}

BOOL GUI_ControlIsPressed(GUIControlId control_id)
{
	BOOL result = FALSE;
	
	if (gui_lock || (gui_fade_time > 0.0f))
		return FALSE;
	
	if (control_id < 0 || control_id >= gui_controls_count)
		return FALSE;
	
	if (!(gui_controls[control_id].subgroup_flags & gui_active_subgroups))
		return FALSE;
	
	if (gui_controls[control_id].group == gui_active_group)
	{
		if (gui_controls[control_id].visible && gui_controls[control_id].active)
		{
			if (gui_controls[control_id].touch_done)
			{
				gui_controls[control_id].touch_done = FALSE;
				result = TRUE;
			}
		}
	}
	
	return result;
}


BOOL GUI_ControlIsTouched(GUIControlId control_id)
{
	BOOL result = FALSE;
	
	if (gui_lock || (gui_fade_time > 0.0f))
		return FALSE;
	
	if (control_id < 0 || control_id >= gui_controls_count)
		return FALSE;
	
	if (!(gui_controls[control_id].subgroup_flags & gui_active_subgroups))
	{
		return FALSE;
	}
	
	if (gui_controls[control_id].group == gui_active_group)
	{
		if (gui_controls[control_id].visible && gui_controls[control_id].active)
		{
			for (int i = 0; i < MAX_GUI_TOUCHES_SUPPORTED; i ++)
			{
				if (gui_touched_controls[i] == control_id)
				{
					result = TRUE;
				}
			}
		}
	}
	
	return result;
}


void GUI_ControlUntouch(GUIControlId control_id)
{
	if (control_id < 0 || control_id >= gui_controls_count)
		return;
	
	for (int i = 0; i < MAX_GUI_TOUCHES_SUPPORTED; i ++)
	{
		if (gui_touched_controls[i] == control_id)
		{
			gui_touched_controls[i] = GUI_CONTROL_BACKGROUND;
			break;
		}
	}
}


void GUI_GetControlMovements(GUIControlId control_id, float *x, float *y)
{
	float x_r = 0.0f;
	float y_r = 0.0f;
	
	for (int i = 0; i < MAX_GUI_TOUCHES_SUPPORTED; i++)
    {
		if (gui_touched_controls[i] == control_id)
		{
			x_r += gui_touched_locations[i].x - gui_touched_start_locations[i].x;
			y_r += gui_touched_locations[i].y - gui_touched_start_locations[i].y;
			
			gui_touched_start_locations[i] = gui_touched_locations[i];
		}
    }

	*x = x_r;
	*y = y_r;
}


void GUI_GetControllerVector(GUIControlId control_id, Vector2D *vector)
{
	float len;
	
	vector->x = 0.0f;
	vector->y = 0.0f;
		
	if (control_id < 0 || control_id >= gui_controls_count)
		return;
	
	if (gui_controls[control_id].subgroup_flags & gui_active_subgroups)
	{
		if (gui_controls[control_id].group == gui_active_group)
		{
			if (gui_controls[control_id].visible && gui_controls[control_id].active)
			{
				for (int i = 0; i < MAX_GUI_TOUCHES_SUPPORTED; i ++)
				{
					if (gui_touched_controls[i] == control_id)
					{
						vector->x = gui_touched_locations[i].x - gui_touched_start_locations[i].x;
						vector->y = gui_touched_locations[i].y - gui_touched_start_locations[i].y;
						len = sqrtf(vector->x * vector->x + vector->y * vector->y);
						
						if (len <= 0)
						{
							vector->x = 0.0f;
							vector->y = 0.0f;
							break;
						}
						
						if (len > GUI_CONTROLLER_INNER_RADIUS)
						{
							vector->x *= GUI_CONTROLLER_INNER_RADIUS / len;
							vector->y *= GUI_CONTROLLER_INNER_RADIUS / len;
						}
						
						vector->x /= GUI_CONTROLLER_INNER_RADIUS;
						vector->y /= GUI_CONTROLLER_INNER_RADIUS;
					}
				}
			}
		}
	}
	
	gui_controls[control_id].x2 = vector->x * GUI_CONTROLLER_INNER_RADIUS;
	gui_controls[control_id].y2 = vector->y * GUI_CONTROLLER_INNER_RADIUS;
}

void GUI_GetControlVector(GUIControlId control_id, Vector2D *vector)
{
	float len;
	
	vector->x = 0.0f;
	vector->y = 0.0f;
    
	if (control_id < 0 || control_id >= gui_controls_count)
		return;
	
	if (gui_controls[control_id].subgroup_flags & gui_active_subgroups)
	{
		if (gui_controls[control_id].group == gui_active_group)
		{
			if (gui_controls[control_id].visible && gui_controls[control_id].active)
			{
				for (int i = 0; i < MAX_GUI_TOUCHES_SUPPORTED; i ++)
				{
					if (gui_touched_controls[i] == control_id)
					{
						vector->x = gui_touched_locations[i].x - gui_touched_start_locations[i].x;
						vector->y = gui_touched_locations[i].y - gui_touched_start_locations[i].y;
					}
				}
			}
		}
	}
}


void GUI_GetPointInControl(GUIControlId control_id, Vector2D *vector)
{
	vector->x = 0.0f;
	vector->y = 0.0f;
	
	if (control_id < 0 || control_id >= gui_controls_count)
		return;
	
	if (gui_controls[control_id].subgroup_flags & gui_active_subgroups)
	{
		if (gui_controls[control_id].group == gui_active_group)
		{
			if (gui_controls[control_id].visible && gui_controls[control_id].active)
			{
				for (int i = 0; i < MAX_GUI_TOUCHES_SUPPORTED; i ++)
				{
					if (gui_touched_controls[i] == control_id)
					{
						if (gui_controls[control_id].flags & GUI_CONTROL_ALIGN_RIGHT)
						{
							vector->x = gui_touched_locations[i].x - (gui_controls[control_id].x - gui_controls[control_id].scale * gui_controls[control_id].width);
						}
						else if (gui_controls[control_id].flags & GUI_CONTROL_ALIGN_CENTER)
						{
							vector->x = gui_touched_locations[i].x - (gui_controls[control_id].x - gui_controls[control_id].scale * gui_controls[control_id].width * 0.5f);
						}
						else
						{
							vector->x = gui_touched_locations[i].x - gui_controls[control_id].x;
						}
						
						if (gui_controls[control_id].flags & GUI_CONTROL_VERTICAL_CENTERED)
						{
							vector->y = gui_touched_locations[i].y - (gui_controls[control_id].y - gui_controls[control_id].scale * gui_controls[control_id].height * 0.5f);
						}
						else
						{
							vector->y = gui_touched_locations[i].y - gui_controls[control_id].y;
						}
					}
				}
			}
		}
	}
}


float GUI_GetSliderValue(GUIControlId control_id)
{
	Vector2D sprite_1_size, sprite_2_size;
	float slider_k;
		
	if (control_id < 0 || control_id >= gui_controls_count)
		return 0;
	
	if (gui_controls[control_id].subgroup_flags & gui_active_subgroups)
	{
		if (gui_controls[control_id].group == gui_active_group)
		{
			if (gui_controls[control_id].visible && gui_controls[control_id].active)
			{
				for (int i = 0; i < MAX_GUI_TOUCHES_SUPPORTED; i ++)
				{
					if (gui_touched_controls[i] == control_id)
					{
						if (!gui_controls[control_id].use_sprites)
							continue;
						
						Sprites_GetSpriteSize(&gui_controls[control_id].sprite_1, &sprite_1_size);
						Sprites_GetSpriteSize(&gui_controls[control_id].sprite_2, &sprite_2_size);
						
						float div_tmp = sprite_1_size.x - 2.0f - sprite_2_size.x;
						if (div_tmp == 0.0f)
							div_tmp = FLT_EPSILON;
						slider_k = (gui_controls[control_id].slider_max - gui_controls[control_id].slider_min) / div_tmp;
						
						gui_controls[control_id].slider_value += (gui_touched_locations[i].x - gui_touched_start_locations[i].x) * slider_k;
						gui_touched_start_locations[i].x = gui_touched_locations[i].x;
						
						if (gui_controls[control_id].slider_value < gui_controls[control_id].slider_min)
							gui_controls[control_id].slider_value = gui_controls[control_id].slider_min;
						if (gui_controls[control_id].slider_value > gui_controls[control_id].slider_max)
							gui_controls[control_id].slider_value = gui_controls[control_id].slider_max;
						
						gui_controls[control_id].x2 = (gui_controls[control_id].slider_value - gui_controls[control_id].slider_min) / slider_k + 1.0f;
						gui_controls[control_id].y2 = (sprite_1_size.y - sprite_2_size.y) * 0.5f;
					}
				}
			}
		}
	}
	
	return gui_controls[control_id].slider_value;
}


void GUI_MakeFadeWithEvent(int event_id, float in_time, float out_time)
{
	gui_event_id = event_id;
	gui_event_activated = FALSE;
	
	gui_fade_time_in = in_time;
	gui_fade_time_out = out_time;
	gui_fade_time = gui_fade_time_in + gui_fade_time_out;
}

int GUI_ProcessEvents(int *event_id, float delta_time)
{
	static int gui_prev_active_group = -1;
	
	if (gui_prev_active_group != gui_active_group)
	{
		for (int i = 0; i < gui_controls_count; i++)
		{
			gui_controls[i].touch_began = FALSE;
			gui_controls[i].touch_done = FALSE;
			
			quick_touch_time = 1000.0f;
			quick_touch = FALSE;
		}
		
		gui_prev_active_group = gui_active_group;
	}
	
	quick_touch_time += delta_time;

	float gui_prev_time = gui_fade_time;
	
	gui_fade_time -= delta_time;
	
	if (gui_fade_time <= gui_fade_time_out)
	{
		if (gui_prev_time > gui_fade_time_out)
		{
			gui_event_activated = TRUE;
		}
	}
	
	if (gui_event_activated)
	{
		*event_id = gui_event_id;
		
		gui_event_activated = FALSE;
		
		return TRUE;
	}
	
	return FALSE;
}

void GUI_RenderFade()
{
	if (gui_fade_time > 0.0f)
	{
		float fade_k = 0.0f;
		
		if (gui_fade_time > gui_fade_time_out)
		{
			fade_k = 1.0f - ((gui_fade_time - gui_fade_time_out) / (gui_fade_time_in + FLT_EPSILON));
		}
		else
		{
			fade_k = gui_fade_time / (gui_fade_time_out + FLT_EPSILON);
		}
		
		fade_k = MathWave(fade_k);
		
        Render_DisableTextures();
        Render_EnableVertexArray();
        Render_DisableTexCoordArray();
        Render_DisableColorArray();
        Render_DisableIndexArray();
		
        
        Render_SetBlendFunc(TR_SRC_ALPHA, TR_ONE_MINUS_SRC_ALPHA);
        
        Render_SetVertexArray(&gui_fade_vertices[0], 2, TR_FLOAT, 0);
        Render_SetColor(0.0f, 0.0f, 0.0f, fade_k);
        
        Render_DrawArrays(TR_TRIANGLE_STRIP, 4);
	}
}


void GUI_Release()
{
	//
}

