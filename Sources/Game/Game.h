
#ifndef _GAME_H_
#define _GAME_H_

enum
{
	GAME_STAGE_MENU_MAIN,
	GAME_STAGE_GAME
};

enum
{
	GUI_GROUP_MENU_MAIN,
	GUI_GROUP_GAME,
	GUI_GROUP_GAME_MENU
};

extern int game_stage;
extern int next_game_stage;

typedef struct _Ship
{
	Vector2D pos;
	float rotation;
	Vector2D speed;
	float rotation_speed;

	BOOL left_engine_power;
	BOOL right_engine_power;

} Ship;

extern Ship ship;

extern float new_game_timer;

extern U32 coins_count;
extern BOOL gameplay_enabled;
extern BOOL tutorial_mode;

extern BOOL left_engine_override;
extern BOOL right_engine_override;

void Reset_Particles();

void Game_MakeZoomEffect(float time, float zoom);

void Game_Init();
void Game_Release();

void Game_NewGame(int level_index);
void Game_Exit();

void Game_Process();
void Game_Render();

void Game_Pause();


#endif /* _GAME_H_ */
