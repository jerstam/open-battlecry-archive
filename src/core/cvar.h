#pragma once

#include "common.h"

typedef enum
{
	CVAR_TYPE_INT = 0,
	CVAR_TYPE_BOOL = 1,
	CVAR_TYPE_FLOAT = 2,
	CVAR_TYPE_STRING = 3
} cvar_type_t;

typedef enum
{
	CVAR_NONE = 0,
	CVAR_SAVE = 1,
	CVAR_CHEAT = 2,
	CVAR_MODIFIED = 4
} cvar_flags_t;

typedef struct cvar_t
{
	const char* name;
	const char* description;

	u8 type;
	u8 flags;
	u8 index;
	u8 name_length;

	union
	{
		i32 int_value;
		bool bool_value;
		float float_value;
	};
} cvar_t;

void cvar_load(void);
void cvar_save(void);

void cvar_register(cvar_t* cvar);
void cvar_add_int(cvar_t* cvar, const char* name, const char* description, i32 value, cvar_flags_t flags);
void cvar_add_bool(cvar_t* cvar, const char* name, const char* description, bool value, cvar_flags_t flags);
void cvar_add_float(cvar_t* cvar, const char* name, const char* description, float value, cvar_flags_t flags);

cvar_t* cvar_find(const char* name);
const char* cvar_complete(const char* name);

void cvar_set_int(cvar_t* cvar, i32 value);
void cvar_set_bool(cvar_t* cvar, bool value);
void cvar_set_float(cvar_t* cvar, float value);