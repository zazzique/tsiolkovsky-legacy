
#ifndef _FONT_H_
#define _FONT_H_

#define TEXT_ALIGN_LEFT 0x00000000
#define TEXT_ALIGN_RIGHT 0x00000001
#define TEXT_ALIGN_CENTER 0x00000002
#define TEXT_VERTICAL_CENTERED 0x00000004


void Font_Init();
void Font_Add(char *font_name);
void Font_PrintText(float x, float y, float text_scale, U32 text_color, char *text, U32 flags, char *font_name);
void Font_GetTextSize(char *text, char *font_name, Vector2D *size);
void Font_Render();
void Font_Release();

#endif /* _FONT_H_ */

