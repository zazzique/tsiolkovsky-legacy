
#include "Common.h"
#include "GameDefines.h"
#include "GameVariables.h"
#include "FastMath.h"
#include "Vector.h"
#include "Timer.h"
#include "GameConfig.h"
#include "Files.h"
#include "Sound.h"
#include "Render.h"
#include "TextureManager.h"
#include "Font.h"
#include "Sprites.h"
#include "GUIControls.h"
#include "ModelManager.h"
#include "Menu.h"
#include "Game.h"

#define MAX_PARTICLES_COUNT 512
#define MAX_BORDERS_COUNT 2048
#define MAX_COINS_COUNT 256
#define MAX_TAP_CIRCLES_COUNT 32

int game_stage;
int next_game_stage;

Vector2D borders_pos[MAX_BORDERS_COUNT];
U32 borders_count;

Vector2D coins_pos[MAX_COINS_COUNT];
U32 coins_count;

Ship ship;

float target_zoom;
float previous_zoom;
float zoom_time;
float zoom_countdown_timer;

typedef struct _Camera2D
{
	Vector2D pos;
	float scale;

} Camera2D;

Camera2D camera;

typedef struct _Particle
{
	Vector2D pos;
	Vector2D speed;
	float countdown_timer;

} Particle;

Particle particles[MAX_PARTICLES_COUNT];
Particle tap_circles[MAX_TAP_CIRCLES_COUNT];

SpriteHandler sprite_tap_circle;

SpriteHandler sprite_ship;
SpriteHandler sprite_particle;
SpriteHandler sprite_border;
SpriteHandler sprite_coin;

SpriteHandler sprite_ship_shadow;
SpriteHandler sprite_particle_shadow;
SpriteHandler sprite_border_shadow;
SpriteHandler sprite_coin_shadow;

SpriteHandler sprite_background;
SpriteHandler sprite_vignette;

SpriteHandler sprite_black;

BOOL timer_10hz;
float timer_10hz_aux;

float tutorial_timer;

BOOL gameplay_enabled;
BOOL tutorial_mode;

float ingame_menu_k;

void Game_MakeZoomEffect(float time, float zoom)
{
	zoom_time = time;
	zoom_countdown_timer = zoom_time;
	target_zoom = zoom;
	previous_zoom = camera.scale;
}

void DrawGameObject(SpriteHandler *sprite, SpriteHandler *sprite_shadow, float sprite_x, float sprite_y, float sprite_rotation, float sprite_scale, U32 sprite_color)
{
	float scale = 1 / camera.scale;
	float x = scale * (camera.pos.x - sprite_x) + screen_center_x;
	float y = scale * (camera.pos.y - sprite_y) + screen_center_y;

	if (sprite_shadow != NULL)
		Sprites_DrawSprite(sprite_shadow, x + scale * 2.0f, y - scale * 2.0f, scale * sprite_scale, RAD2DEG(sprite_rotation), 0xffffff00 & sprite_color, SPRITE_CENTERED);

	Sprites_DrawSprite(sprite, x, y, scale * sprite_scale, RAD2DEG(sprite_rotation), sprite_color, SPRITE_CENTERED);
}

void Reset_TapCircles()
{
	for (int i = 0; i < MAX_TAP_CIRCLES_COUNT; i ++)
	{
		tap_circles[i].countdown_timer = 0.0f;
	}
}

void Process_TapCircles()
{
	for (int i = 0; i < MAX_TAP_CIRCLES_COUNT; i ++)
	{
		if (tap_circles[i].countdown_timer > 0.0f)
			tap_circles[i].countdown_timer -= game_delta_t;
	}
}

void Render_TapCircles()
{
	for (int i = 0; i < MAX_TAP_CIRCLES_COUNT; i ++)
	{
		if (tap_circles[i].countdown_timer > 0.0f)
		{
			float life_factor = tap_circles[i].countdown_timer / 1.0f;
			float scale = 0.25f + (1.0f - life_factor) * 2.5f;

			U8 color[4];
			color[0] = 0xff;
			color[1] = 0xff;
			color[2] = 0xff;
			color[3] = (U8)(life_factor * 96);

			Sprites_DrawSprite(&sprite_tap_circle, tap_circles[i].pos.x, tap_circles[i].pos.y, UNISCALE(scale), 0.0f, *(U32 *)&color[0], SPRITE_CENTERED);
		}
	}
}

void Add_TapCircle(Vector2D *pos)
{
	int particle_index = -1;

	for (int i = 0; i < MAX_TAP_CIRCLES_COUNT; i ++)
	{
		if (tap_circles[i].countdown_timer <= 0.0f)
		{
			particle_index = i;
			break;
		}
	}

	if (particle_index < 0)
		return;

	tap_circles[particle_index].pos = *pos;
	tap_circles[particle_index].countdown_timer = 1.0f;
}

void Reset_Particles()
{
	for (int i = 0; i < MAX_PARTICLES_COUNT; i ++)
	{
		particles[i].countdown_timer = 0.0f;
	}
}

void Process_Particles()
{
	for (int i = 0; i < MAX_PARTICLES_COUNT; i ++)
	{
		if (particles[i].countdown_timer > 0.0f)
			particles[i].countdown_timer -= game_delta_t;

		particles[i].pos.x += particles[i].speed.x * game_delta_t;
		particles[i].pos.y += particles[i].speed.y * game_delta_t;
	}
}

void Render_Particles()
{
	for (int i = 0; i < MAX_PARTICLES_COUNT; i ++)
	{
		if (particles[i].countdown_timer > 0.0f)
		{
			float life_factor = particles[i].countdown_timer / 1.6f;
			float scale = 0.25f + (1.0f - life_factor) * 3.75f;

			U8 color[4];
			color[0] = (U8)(life_factor * 255);
			color[1] = (U8)(life_factor * 255);
			color[2] = (U8)(life_factor * 255);
			color[3] = (U8)(life_factor * 255);

			if (tutorial_mode)
				color[3] /= 2;

			DrawGameObject(&sprite_particle, &sprite_particle_shadow, particles[i].pos.x, particles[i].pos.y, 0.0f, scale, *(U32 *)&color[0]);
		}
	}
}

void Add_Particle(Vector2D *pos, Vector2D *speed)
{
	int particle_index = -1;

	for (int i = 0; i < MAX_PARTICLES_COUNT; i ++)
	{
		if (particles[i].countdown_timer <= 0.0f)
		{
			particle_index = i;
			break;
		}
	}

	if (particle_index < 0)
		return;

	particles[particle_index].pos = *pos;
	particles[particle_index].speed = *speed;
	particles[particle_index].countdown_timer = 1.6f;
}

void Reset_Ship()
{
	ship.pos.x = 0.0f;
	ship.pos.y = 0.0f;
	ship.rotation = 0.0f;
	ship.speed.x = 0.0f;
	ship.speed.y = 0.0f;
	ship.rotation_speed = 0.0f;
	ship.left_engine_power = FALSE;
	ship.right_engine_power = FALSE;
}

void Process_Ship()
{
	float sin_alpha;
	float cos_alpha;

	sin_alpha = sinf(ship.rotation);
	cos_alpha = cosf(ship.rotation);

	if (ship.right_engine_power)
	{
		ship.rotation_speed += 1.6f * game_delta_t;
		ship.speed.x += 32 * game_delta_t * sin_alpha;
		ship.speed.y -= 32 * game_delta_t * cos_alpha;
	}
	if (ship.left_engine_power)
	{
		ship.rotation_speed -= 1.6f * game_delta_t;
		ship.speed.x += 32 * game_delta_t * sin_alpha;
		ship.speed.y -= 32 * game_delta_t * cos_alpha;
	}

	if (ship.rotation_speed > 31.4f)
		ship.rotation_speed = 31.4f;
	if (ship.rotation_speed < -31.4f)
		ship.rotation_speed = -31.4f;

	ship.rotation += ship.rotation_speed * game_delta_t;
	ship.pos.x += ship.speed.x * game_delta_t;
	ship.pos.y += ship.speed.y * game_delta_t;

	if (timer_10hz)
	{
		Vector2D particle_pos;
		Vector2D particle_speed;
		Vector2D circle_pos;
				
		particle_speed.x = -128 * sin_alpha + ship.speed.x;
		particle_speed.y = 128 * cos_alpha + ship.speed.y;

		BOOL right_engine_power = FALSE;
		BOOL left_engine_power = FALSE;

		if (ship.right_engine_power)
			right_engine_power = TRUE;

		if (ship.left_engine_power)
			left_engine_power = TRUE;

		if (tutorial_mode)
		{
			if ((tutorial_timer > 1.0f && tutorial_timer < 1.4f) || (tutorial_timer > 1.6f && tutorial_timer < 1.9f))
			{
				circle_pos.x = ((float)v_sx / 8.0f) * 7.0f;
				circle_pos.y = (float)v_sy / 5.0f;
				Add_TapCircle(&circle_pos);

				right_engine_power = TRUE;
			}

			if ((tutorial_timer > 3.0f && tutorial_timer < 3.4f) || (tutorial_timer > 3.6f && tutorial_timer < 3.9f))
			{
				circle_pos.x = (float)v_sx / 8.0f;
				circle_pos.y = (float)v_sy / 5.0f;
				Add_TapCircle(&circle_pos);

				left_engine_power = TRUE;
			}
		}

		if (right_engine_power)
		{
			particle_pos.x = -16 * cos_alpha - 20 * sin_alpha + ship.pos.x;
			particle_pos.y = -16 * sin_alpha + 20 * cos_alpha + ship.pos.y;

			Add_Particle(&particle_pos, &particle_speed);
		}
		if (left_engine_power)
		{
			particle_pos.x = 16 * cos_alpha - 20 * sin_alpha + ship.pos.x;
			particle_pos.y = 16 * sin_alpha + 20 * cos_alpha + ship.pos.y;

			Add_Particle(&particle_pos, &particle_speed);
		}
	}
}

void Render_Ship()
{
	DrawGameObject(&sprite_ship, &sprite_ship_shadow, ship.pos.x, ship.pos.y, -ship.rotation, 1.0f, 0xffffffff);
}

void Reset_Camera()
{
	camera.pos.x = ship.pos.x;
	camera.pos.y = ship.pos.y;

	switch (screen_size)
	{
		case SCREEN_SIZE_SMALL:
			camera.scale = 2.5f;
			break;
		case SCREEN_SIZE_NORMAL:
			camera.scale = 2.0f;
			break;
		case SCREEN_SIZE_LARGE:
			camera.scale = 1.0f;
			break;
		case SCREEN_SIZE_XLARGE:
			camera.scale = 1.0f;
			break;
		default:
			camera.scale = 1.0f;
			break;
	}
	target_zoom = camera.scale;
	previous_zoom = camera.scale;
	zoom_time = 0.0f;
	zoom_countdown_timer = 0.0f;
}

void Process_Camera()
{
	const float restriction_x = screen_center_x * 0.5f;
	const float restriction_y = screen_center_y * 0.5f;

	if ((camera.pos.x - ship.pos.x) < -restriction_x)
		camera.pos.x = ship.pos.x - restriction_x;

	if ((camera.pos.y - ship.pos.y) < -restriction_y)
		camera.pos.y = ship.pos.y - restriction_y;

	if ((camera.pos.x - ship.pos.x) > restriction_x)
		camera.pos.x = ship.pos.x + restriction_x;

	if ((camera.pos.y - ship.pos.y) > restriction_y)
		camera.pos.y = ship.pos.y + restriction_y;

	float k;
	if (zoom_time > 0)
	{
		k = 1.0f - (zoom_countdown_timer / zoom_time);
		camera.scale = previous_zoom + (target_zoom - previous_zoom) * MathWave(k);

		zoom_countdown_timer -= delta_t;

		if (zoom_countdown_timer <= 0)
		{
			zoom_time = 0.0f;
			previous_zoom = target_zoom;
		}
	}
}

BOOL Load_Level(char *level_name)
{
	FileHandler level_file;

	if (!Files_OpenFile(&level_file, level_name))
	{
		LogPrint("Error: level '%s' not found!\n", level_name);
		return FALSE;
	}

	borders_count = 0;
	coins_count = 0;

	Files_Read(&level_file, &ship.pos.x, sizeof(float));
	Files_Read(&level_file, &ship.pos.y, sizeof(float));
	Files_Read(&level_file, &ship.rotation, sizeof(float));
	Files_Skip(&level_file, sizeof(int));

	ship.pos.x = -ship.pos.x;
	ship.rotation = -ship.rotation;

	camera.pos.x = ship.pos.x;
	camera.pos.y = ship.pos.y;

	Files_Read(&level_file, &borders_count, sizeof(U32));
	for (U32 i = 0; i < borders_count; i ++)
	{
		Files_Read(&level_file, &borders_pos[i].x, sizeof(float));
		Files_Read(&level_file, &borders_pos[i].y, sizeof(float));

		borders_pos[i].x = -borders_pos[i].x;
	}

	Files_Read(&level_file, &coins_count, sizeof(U32));
	for (U32 i = 0; i < coins_count; i ++)
	{
		Files_Read(&level_file, &coins_pos[i].x, sizeof(float));
		Files_Read(&level_file, &coins_pos[i].y, sizeof(float));

		coins_pos[i].x = -coins_pos[i].x;
	}
	
	Files_CloseFile(&level_file);
	
	return TRUE;
}

void Process_Level()
{
	for (int i = 0; i < borders_count; i ++)
	{
		float dist_sqr = Vector2D_SegmentLengthSQR(&ship.pos, &borders_pos[i]);

		if (dist_sqr <= SQR(30.0f))
		{
			paused = TRUE;
			GUI_MakeFadeWithEvent(MENU_EVENT_GAME_TO_MAIN, 0.75f, 0.25f);
		}
	}

	for (int i = 0; i < coins_count; i ++)
	{
		float dist_sqr = Vector2D_SegmentLengthSQR(&ship.pos, &coins_pos[i]);

		if (dist_sqr <= SQR(42.0f))
		{
			coins_count --;

			for (int j = i; j < coins_count; j ++)
			{
				coins_pos[j].x = coins_pos[j + 1].x;
				coins_pos[j].y = coins_pos[j + 1].y;
			}
		}
	}

	if (coins_count <= 0)
	{
		gameplay_enabled = FALSE;
		paused = TRUE;
		
		if (!level_stats[current_level_index].scored || (level_stats[current_level_index].time > level_time))
			level_stats[current_level_index].time = level_time;

		level_stats[current_level_index].scored = TRUE;
		GUI_MakeFadeWithEvent(MENU_EVENT_GAME_TO_MAIN, 0.75f, 0.25f);
	}
}

void Render_Level()
{
	for (U32 i = 0; i < borders_count; i ++)
	{
		DrawGameObject(&sprite_border, &sprite_border_shadow, borders_pos[i].x, borders_pos[i].y, 0.0f, 1.0f, 0xffffffff);
	}

	for (U32 i = 0; i < coins_count; i ++)
	{
		DrawGameObject(&sprite_coin, &sprite_coin_shadow, coins_pos[i].x, coins_pos[i].y, 0.0f, 0.7f, 0xffffffff);
	}
}

void Game_NewGame(int level_index)
{
	Reset_Ship();
	Reset_Particles();
	Reset_TapCircles();

	char level_name[256];
	sprintf(level_name, "%d.sl", level_index + 1);

	if (Load_Level(level_name))
	{
		paused = FALSE;
		show_ingame_menu = FALSE;
		level_time = 0;
		gameplay_enabled = FALSE;
		next_game_stage = GAME_STAGE_GAME;

		if (level_index < 3)
			tutorial_mode = TRUE;

		//Game_MakeZoomEffect(1.0f, 1.0f);
	}
}

void Game_Exit()
{
	paused = FALSE;

	ingame_menu_k = 0.0f;
	
	next_game_stage = GAME_STAGE_MENU_MAIN;

	gameplay_enabled = FALSE;
	tutorial_mode = FALSE;
}

void Game_ResetSettingsToDefault()
{
	for (int i = 0; i < LEVELS_COUNT; i ++)
	{
		level_stats[i].locked = TRUE;
		level_stats[i].scored = FALSE;

		level_stats[i].time = 0.0f;
		level_stats[i].score = 0;
	}
}

void Game_Init()
{
	Game_ResetSettingsToDefault();
	GameConfig_Load();

	Font_Add("tronique");
	Font_Add("legacy");
	
	global_fade_k = 0.0f;
	
	paused = FALSE;
	timer_10hz = FALSE;
	timer_10hz_aux = 0.0f;

	tutorial_timer = 0.0f;
	
    game_stage = GAME_STAGE_MENU_MAIN;
    next_game_stage = game_stage;

	gameplay_enabled = FALSE;
	tutorial_mode = FALSE;

	ingame_menu_k = 0.0f;

	//////////////////

	Sprites_AddSprite(&sprite_background, "background.tga", 0.0f, 0.0f, (float)v_sx, (float)v_sy, -16);

	Sprites_AddSprite(&sprite_particle_shadow, "smoke_shadow.tga", 0.0f, 0.0f, 0.0f, 0.0f, -7);
	Sprites_AddSprite(&sprite_ship_shadow, "ship_shadow.tga", 0.0f, 0.0f, 0.0f, 0.0f, -10);
	Sprites_AddSprite(&sprite_coin_shadow, "coin_shadow.tga", 0.0f, 0.0f, 0.0f, 0.0f, -9);
	Sprites_AddSprite(&sprite_border_shadow, "border_shadow.tga", 0.0f, 0.0f, 0.0f, 0.0f, -8);

	Sprites_AddSprite(&sprite_particle, "smoke.tga", 0.0f, 0.0f, 0.0f, 0.0f, 3);
	Sprites_AddSprite(&sprite_ship, "ship.tga", 0.0f, 0.0f, 0.0f, 0.0f, 0);
	Sprites_AddSprite(&sprite_coin, "coin.tga", 0.0f, 0.0f, 0.0f, 0.0f, 1);
	Sprites_AddSprite(&sprite_border, "border.tga", 0.0f, 0.0f, 0.0f, 0.0f, 2);

	Sprites_AddSprite(&sprite_tap_circle, "circle.tga", 0.0f, 0.0f, 0.0f, 0.0f, 22);
	
	Vector2D size;
	size.x = (float)v_sx;
	size.y = (float)v_sy;
	Sprites_AddSprite(&sprite_vignette, "vignette_screen.tga", 0.0f, 0.0f, 0.0f, 0.0f, 16);
	Sprites_SetSpriteSize(&sprite_vignette, &size);

	size.x = (float)v_sx;
	size.y = UNISCALE(16.0f);
	Sprites_AddSprite(&sprite_black, "black.tga", 0.0f, 0.0f, 0.0f, 0.0f, 20);
	Sprites_SetSpriteSize(&sprite_black, &size);
	///////////////////

	Reset_Camera();
	Reset_TapCircles();

	Menu_Init();
}

void Game_Release()
{
	Menu_Release();
}

void Game_ProcessGameTime()
{
    game_delta_t = delta_t;
	
	if (paused)
	{
		game_delta_t = 0.0f;
	}
	
	if (gameplay_enabled)
		level_time += game_delta_t;

	timer_10hz = FALSE;
	timer_10hz_aux += game_delta_t;

	while (timer_10hz_aux > 0.1f)
	{
		timer_10hz_aux -= 0.1f;
		timer_10hz = TRUE;
	}

	if (tutorial_mode)
	{
		tutorial_timer += delta_t;
	}

	if (tutorial_timer > 4.3f)
		tutorial_timer = 0.0f;
}

void Game_Process()
{
	double prev_time;
	Game_ProcessGameTime();

	Menu_Process();
	
	game_stage = next_game_stage;

	Process_Camera();
	
	if (game_stage == GAME_STAGE_GAME)
	{
		if (!paused)
		{
			Process_Ship();
			Process_Particles();
			Process_Level();
			Process_TapCircles();

			if (show_ingame_menu)
				DeltaFunc(&ingame_menu_k, 1.0f, delta_t * 5.0f);
			else
				DeltaFunc(&ingame_menu_k, 0.0f, delta_t * 5.0f);
		}
	}
}

void Game_Render()
{
	Render_DisableDepthMask();
	Render_DisableDepthTest();
	Render_DisableAlphaTest();
	Render_SetBlendFunc(TR_SRC_ALPHA, TR_ONE_MINUS_SRC_ALPHA);
	Render_EnableBlend();
	   
    //Render_Clear(0.0f, 0.2f, 0.3f, 1.0f);

	Vector2D pos;
	Vector2D size;
	Sprites_GetSpriteSize(&sprite_background, &size);

	pos.x = -camera.pos.x / size.x;
	pos.y = camera.pos.y / size.y;

	Sprites_DrawSpriteEx(&sprite_background, 0.0f, 0.0f, 1.0f, 1.0f, pos.x - camera.scale * 0.5f, pos.y - camera.scale * 0.5f, pos.x + camera.scale * 0.5f, pos.y + camera.scale * 0.5f, 0.0f, 0xffffffff, SPRITE_ALIGN_LEFT | SPRITE_ALIGN_DOWN);

	
	if (game_stage == GAME_STAGE_GAME)
	{
		Render_Ship();
		Render_Particles();
		Render_Level();
		Render_TapCircles();

		Sprites_DrawSprite(&sprite_black, 0.0f, (float)v_sy - UNISCALE(16.0f) - MathWave(MathWave(ingame_menu_k)) * 0.65f * v_sy, 1.0f, 0.0f, 0xbbffffff, SPRITE_ALIGN_LEFT | SPRITE_ALIGN_DOWN);
	}
	
    Render_SetProjectionOrtho();
    
    Render_SetMatrixMode(TR_MODEL);
    Render_ResetMatrix();
    
    Render_SetMatrixMode(TR_TEXTURE);
    Render_ResetMatrix();
    
	Menu_Render();

	Sprites_DrawSprite(&sprite_vignette, 0.0f, 0.0f, 1.0f, 0.0f, 0xffffffff, SPRITE_ALIGN_LEFT | SPRITE_ALIGN_DOWN);
}

void Game_Pause()
{
    //
}

