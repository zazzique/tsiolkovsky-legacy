
#ifndef _MENU_H_
#define _MENU_H_

enum
{
	MENU_EVENT_NONE,
	MENU_EVENT_GAME_TO_MAIN,
	MENU_EVENT_MAIN_TO_GAME
};

extern int current_level_index;
extern BOOL show_ingame_menu;

void Menu_Init();
void Menu_Process();
void Menu_Render();
void Menu_Release();

#endif /* _MENU_H_ */



