//
//  GUIControls.h
//  Carnivores
//
//  Created by Dmitry Nechay on 25.08.09.
//  Copyright 2009 Action Forms. All rights reserved.
//

#ifndef _GUI_CONTROLS_H_
#define _GUI_CONTROLS_H_

#define GUI_CONTROL_ALIGN_LEFT 0x00000001
#define GUI_CONTROL_ALIGN_RIGHT 0x00000002
#define GUI_CONTROL_ALIGN_CENTER 0x00000004
#define GUI_CONTROL_VERTICAL_CENTERED 0x00000008

#define GUI_SUBGROUP_DEFAULT 0x00000001

typedef int GUIControlId;

enum
{
    GUI_LABEL,
	GUI_BUTTON,
	GUI_SLIDER,
	GUI_CONTROLLER
};

extern BOOL gui_enable_touches_reset;

void GUI_Init();
void GUI_Lock();
void GUI_Unlock();
void GUI_AddControl(GUIControlId *control_id, int group, U32 subgroups, int type, Vector2D *pos, Vector2D *size, U32 flags, SpriteHandler *sprite_1, SpriteHandler *sprite_2, const char *text, const char *font_name, Vector2D *text_pos);
void GUI_SetControlPos(GUIControlId control_id, float x, float y);
void GUI_SetControlFlags(GUIControlId control_id, U32 flags);
void GUI_SetControlColor(GUIControlId control_id, U32 color);
void GUI_SetControlScale(GUIControlId control_id, float scale);
void GUI_SetControlText(GUIControlId control_id, char *text);
void GUI_SetControlFontScale(GUIControlId control_id, float scale);
void GUI_SetControlSprites(GUIControlId control_id, SpriteHandler *sprite_1, SpriteHandler *sprite_2);
void GUI_SetControlActive(GUIControlId control_id, BOOL active);
void GUI_SetControlVisible(GUIControlId control_id, BOOL visible);
void GUI_SetSliderValue(GUIControlId control_id, float value);
void GUI_SetSliderParams(GUIControlId control_id, float min, float max);
void GUI_GetControlPos(GUIControlId control_id, float *x, float *y);
float GUI_GetSliderValue(GUIControlId control_id);
void GUI_SetActiveGroup(int group, U32 subgroups);
void GUI_DrawControls();
BOOL GUI_PointInControl(GUIControlId control_id, float x, float y);
int  GUI_GetUnusedTouch();
int  GUI_GetTouchByLocation(float x, float y);

void GUI_TouchBegan(float x, float y);
void GUI_TouchMoved(int touch_id, float x, float y);
void GUI_TouchEnded(int touch_id, float x, float y);
void GUI_TouchesReset();

void GUI_GetBackgroundMovements(float *x, float *y);
BOOL GUI_GetQuickTouch();
BOOL GUI_ControlIsPressed(GUIControlId control_id);
BOOL GUI_ControlIsTouched(GUIControlId control_id);
void GUI_ControlUntouch(GUIControlId control_id);
void GUI_GetControlMovements(GUIControlId control_id, float *x, float *y);
void GUI_GetControllerVector(GUIControlId control_id, Vector2D *vector);
void GUI_GetControlVector(GUIControlId control_id, Vector2D *vector);
void GUI_GetPointInControl(GUIControlId control_id, Vector2D *vector);
void GUI_MakeFadeWithEvent(int event_id, float in_time, float out_time);
int GUI_ProcessEvents(int *event_id, float delta_time);
void GUI_RenderFade();
void GUI_Release();

#endif /* _GUI_CONTROLS_H_ */
