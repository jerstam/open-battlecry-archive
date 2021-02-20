#pragma once

#include "../macros.h"

enum
{
	MAX_UNITS = 4096,
	MAX_BUILDINGS = 1024
};

struct Position
{
	float x;
	float y;
};
typedef struct Position Position;

struct Movement
{
	float x;
	float y;
	float target_x;
	float target_y;
	float speed;
};
typedef struct Movement Movement;

struct Status
{
	u16 health;
	u8 armor;
	u8 resistance;
	u32 flags;
	u16 damage_over_time;
	u16 damage_over_time_timer;
};
typedef struct Status Status;

struct GameData
{
	Position unit_positions[MAX_UNITS];
	Position building_positions[MAX_BUILDINGS];
	Movement unit_movement[MAX_UNITS];
	Status unit_status[MAX_UNITS];
};
typedef struct GameData GameData;