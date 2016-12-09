
#include "Common.h"
#include "GameDefines.h"
#include "GameVariables.h"
#include "FastMath.h"
#include "Vector.h"
#include "GameConfig.h"
#include "Timer.h"
#include "Sound.h"
#include "TextureManager.h"
#include "Font.h"
#include "Sprites.h"
#include "GUIControls.h"
#include "ModelManager.h"
#include "Game.h"
#include "Menu.h"

#define MENU_TIME_FADE_IN 0.12f
#define MENU_TIME_FADE_OUT 0.25f

SpriteHandler sprite_button;

Vector2D menu_play_level_pos[LEVELS_COUNT];
GUIControlId menu_play_level[LEVELS_COUNT];
GUIControlId game_exit;

GUIControlId game_left_engine;
GUIControlId game_right_engine;

int current_level_index;
BOOL show_ingame_menu;


void Menu_UpdateStates()
{
	int levels_to_unlock = 3;

	for (int i = 0; i < LEVELS_COUNT; i ++)
	{
		if (!level_stats[i].locked && !level_stats[i].scored)
			levels_to_unlock --;
	}

	if (levels_to_unlock > 0)
		for (int n = 0; n < levels_to_unlock; n ++)
			for (int i = 0; i < LEVELS_COUNT; i ++)
			{
				if (level_stats[i].locked)
				{
					level_stats[i].locked = FALSE;
					break;
				}
			}

	char level_name[32];

	for (int i = 0; i < LEVELS_COUNT; i ++)
	{
		if (level_stats[i].locked)
		{
			GUI_SetControlActive(menu_play_level[i], FALSE);
			GUI_SetControlColor(menu_play_level[i], 0xff404040);
			sprintf(level_name, "#1%d", i + 1);
			GUI_SetControlText(menu_play_level[i], level_name);
		}
		else if (level_stats[i].scored)
		{
			GUI_SetControlActive(menu_play_level[i], TRUE);
			GUI_SetControlColor(menu_play_level[i], 0xff00e3ff);
			sprintf(level_name, "#8%d", i + 1);
			GUI_SetControlText(menu_play_level[i], level_name);
		}
		else
		{
			GUI_SetControlActive(menu_play_level[i], TRUE);
			GUI_SetControlColor(menu_play_level[i], 0xffe0e0e0);
			sprintf(level_name, "#8%d", i + 1);
			GUI_SetControlText(menu_play_level[i], level_name);
		}
	}

	GameConfig_Save();
}

void Menu_Init()
{
	char level_name[256];
	Vector2D pos, size, text_pos, interval;

	Sprites_AddSprite(&sprite_button, "button.tga", 25.0f, 25.0f, 128.0f - 50.0f, 128.0f - 50.0f, 32);

	text_pos.x = UNISCALE(-3.0f);
	text_pos.y =  UNISCALE(-1.5f);

	interval.x = (float)v_sx / 7.0f;
	interval.y = (float)v_sy / 5.0f;
	
	int counter = 0;

	pos.y = v_sy - interval.y;

	for (int j = 0; j < 6; j ++)
	{
		pos.x = interval.x;

		for (int i = 0; i < 6; i ++)
		{
			if (counter >= LEVELS_COUNT)
				break;

			menu_play_level_pos[counter] = pos;

			sprintf(level_name, "%d", counter + 1);
			GUI_AddControl(&menu_play_level[counter], GUI_GROUP_MENU_MAIN, GUI_SUBGROUP_DEFAULT, GUI_BUTTON, &pos, NULL, GUI_CONTROL_ALIGN_CENTER | GUI_CONTROL_VERTICAL_CENTERED, &sprite_button, NULL, level_name, "tronique", &text_pos);
			GUI_SetControlScale(menu_play_level[counter], UNISCALE(0.5f));

			counter ++;
			pos.x += interval.x;
		}

		pos.y -= interval.y;
	}

	pos.x = (float)v_sx / 3.0f;
	pos.y = (float)v_sy - UNISCALE(25.0f);
	size.x = (float)v_sx / 3.0f;
	size.y = UNISCALE(25.0f);
	GUI_AddControl(&game_exit, GUI_GROUP_GAME, GUI_SUBGROUP_DEFAULT, GUI_BUTTON, &pos, &size, GUI_CONTROL_ALIGN_LEFT, NULL, NULL, NULL, NULL, NULL);
	
	pos.x = 0;
	pos.y = 0;
	size.x = (float)v_sx / 3.0f;
	size.y = (float)v_sy;
	GUI_AddControl(&game_left_engine, GUI_GROUP_GAME, GUI_SUBGROUP_DEFAULT, GUI_BUTTON, &pos, &size, GUI_CONTROL_ALIGN_LEFT, NULL, NULL, NULL, NULL, NULL);

	pos.x = (float)v_sx;
	pos.y = 0;
	size.x = (float)v_sx / 3.0f;
	size.y = (float)v_sy;
	GUI_AddControl(&game_right_engine, GUI_GROUP_GAME, GUI_SUBGROUP_DEFAULT, GUI_BUTTON, &pos, &size, GUI_CONTROL_ALIGN_RIGHT, NULL, NULL, NULL, NULL, NULL);

	GUI_MakeFadeWithEvent(MENU_EVENT_NONE, 0.0f, 0.5f);

	Menu_UpdateStates();
}

void Menu_Process()
{
	int event_id;
	int new_event = GUI_ProcessEvents(&event_id, delta_t);
	
	if (game_stage == GAME_STAGE_GAME)
	{
		GUI_SetActiveGroup(GUI_GROUP_GAME, GUI_SUBGROUP_DEFAULT);
				
		if (GUI_ControlIsPressed(game_exit))
		{
			//show_ingame_menu = TRUE;
			//Game_MakeZoomEffect(MENU_TIME_FADE_IN + MENU_TIME_FADE_OUT, 1.05f);
			GUI_MakeFadeWithEvent(MENU_EVENT_GAME_TO_MAIN, MENU_TIME_FADE_IN, MENU_TIME_FADE_OUT);
		}

		if (GUI_ControlIsTouched(game_left_engine) || left_engine_override)
		{
			ship.left_engine_power = TRUE;

			if (gameplay_enabled == FALSE)
				Reset_Particles();

			gameplay_enabled = TRUE;
			tutorial_mode = FALSE;
		}
		else
		{
			ship.left_engine_power = FALSE;
		}

		if (GUI_ControlIsTouched(game_right_engine) || right_engine_override)
		{
			ship.right_engine_power = TRUE;

			if (gameplay_enabled == FALSE)
				Reset_Particles();

			gameplay_enabled = TRUE;
			tutorial_mode = FALSE;
		}
		else
		{
			ship.right_engine_power = FALSE;
		}
			
		if (new_event)
		{
			if (event_id == MENU_EVENT_GAME_TO_MAIN)
			{
				Game_Exit();
				Menu_UpdateStates();
			}
		}
	}
	else if (game_stage == GAME_STAGE_MENU_MAIN)
	{
		GUI_SetActiveGroup(GUI_GROUP_MENU_MAIN, GUI_SUBGROUP_DEFAULT);
		
		for (int i = 0; i < LEVELS_COUNT; i ++)
		{
			if (GUI_ControlIsPressed(menu_play_level[i]))
			{
				//Game_MakeZoomEffect(MENU_TIME_FADE_IN + MENU_TIME_FADE_OUT, 1.0f);
				current_level_index = i;
				GUI_MakeFadeWithEvent(MENU_EVENT_MAIN_TO_GAME, MENU_TIME_FADE_IN, MENU_TIME_FADE_OUT);
				break;
			}
		}

		if (new_event)
		{
			if (event_id == MENU_EVENT_MAIN_TO_GAME)
			{
				Game_NewGame(current_level_index);
			}
		}
	}
}


void Menu_Render()
{
	char str[256];

	if (game_stage == GAME_STAGE_GAME)
	{
		GUI_SetActiveGroup(GUI_GROUP_GAME, GUI_SUBGROUP_DEFAULT);

		strcpy(str, "[ ii ]");
		Font_PrintText(screen_center_x, v_sy - UNISCALE(20.0f), 1.0f, 0xffffffff, str, TEXT_ALIGN_CENTER, "legacy");

		sprintf(str, "%.2f", level_time);
		Font_PrintText(screen_center_x - screen_center_x * 0.33f, v_sy - UNISCALE(20.0f), 1.0f, 0xffffffff, str, TEXT_ALIGN_RIGHT, "legacy");

		sprintf(str, "$%d", coins_count);
		Font_PrintText(screen_center_x + screen_center_x * 0.33f, v_sy - UNISCALE(20.0f), 1.0f, 0xffffffff, str, TEXT_ALIGN_LEFT, "legacy");
	}
	else if (game_stage == GAME_STAGE_MENU_MAIN)
	{
		GUI_SetActiveGroup(GUI_GROUP_MENU_MAIN, GUI_SUBGROUP_DEFAULT);

		for (int i = 0; i < LEVELS_COUNT; i ++)
		{
			if (level_stats[i].scored)
			{
				sprintf(str, "%.3f", level_stats[i].time);
				Font_PrintText(menu_play_level_pos[i].x, menu_play_level_pos[i].y - UNISCALE(22.0f), 0.5f, 0xffffffff, str, TEXT_ALIGN_CENTER, "tronique");
			}
		}
	}

	// Debug info
	char screen_size_str[256];
	switch (screen_size)
	{
		case SCREEN_SIZE_SMALL:
			strcpy(screen_size_str, "small");
			break;
		case SCREEN_SIZE_NORMAL:
			strcpy(screen_size_str, "normal");
			break;
		case SCREEN_SIZE_LARGE:
			strcpy(screen_size_str, "large");
			break;
		case SCREEN_SIZE_XLARGE:
			strcpy(screen_size_str, "xlarge");
			break;
	}
		
	//sprintf(str, "%d x %d, scale: %.1f, size: %s", v_sx, v_sy, pixel_scale, screen_size_str);
	//Font_PrintText(UNISCALE(6.0f), UNISCALE(6.0f), 0.5f, 0xffffffff, str, TEXT_ALIGN_LEFT, "tronique");
}


void Menu_Release()
{
	//
}

