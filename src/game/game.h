#pragma once

#include "../macros.h"

enum GameState
{
	GAME_STATE_MENU,
	GAME_STATE_GAME,
	GAME_STATE_QUIT
};
typedef enum GameState GameState;

void game_init(void);
void game_quit(void);

void game_load(void);
void game_unload(void);

void game_update(float delta_time);