
#ifndef _SPRITES_H_
#define _SPRITES_H_

#define SPRITE_CENTERED 0x00000000
#define SPRITE_ALIGN_LEFT 0x00000001
#define SPRITE_ALIGN_RIGHT 0x00000002
#define SPRITE_ALIGN_UP 0x00000004
#define SPRITE_ALIGN_DOWN 0x00000008

#define SPRITE_FLIP_X 0x00000010
#define SPRITE_FLIP_Y 0x00000020

typedef int SpriteHandler;

void Sprites_Init();
void Sprites_AddSprite(SpriteHandler *sprite_handler, char *texture_name, float tc_x, float tc_y, float tc_width, float tc_height, int priority);
void Sprites_DrawSprite(SpriteHandler *sprite_handler, float x, float y, float scale, float angle, U32 color, U32 flags);
void Sprites_DrawSpriteEx(SpriteHandler *sprite_handler, float x, float y, float scale_x, float scale_y, float tc_bias_x, float tc_bias_y, float tc_scale_x, float tc_scale_y, float angle, U32 color, U32 flags);
void Sprites_GetSpriteSize(SpriteHandler *sprite_handler, Vector2D *size);
void Sprites_SetSpriteSize(SpriteHandler *sprite_handler, const Vector2D *size);
void Sprites_Render();
void Sprites_Release();

#endif /* _SPRITES_H_ */

