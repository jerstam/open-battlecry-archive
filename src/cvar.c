#include "cvar.h"
#include "hash_map.h"
#include "log.h"

#include <string.h>
#include <assert.h>

enum
{
	MAX_CVARS = 256
};

static u8 cvar_count;
static cvar_t* cvars[MAX_CVARS];
static hash_map_t hash_map;

void cvar_load(void)
{
	hash_map_init(&hash_map, MAX_CVARS);
}

void cvar_save(void)
{
	hash_map_free(&hash_map);
}

static void cvar_add(cvar_t* cvar, const char* name, const char* description, cvar_flags_t flags)
{
	assert(cvar);
	assert(name);
    assert(hash_map.buckets);

    log_debug("Cvar registered: %s", name);

	cvar->name = name;
	cvar->name_length = (u8)strlen(name);
	cvar->description = description;
	cvar->flags = flags;
	cvar->index = cvar_count++;

	cvars[cvar->index] = cvar;
	hash_map_add(&hash_map, name, cvar->name_length, cvar->index);
}

void cvar_register(cvar_t* cvar)
{
	assert(cvar);
	assert(cvar->name);
    assert(hash_map.buckets);
	
	cvar_add(cvar, cvar->name, cvar->description, cvar->flags);
}

void cvar_add_int(cvar_t* cvar, const char* name, const char* description, i32 value, cvar_flags_t flags)
{
	cvar_add(cvar, name, description, flags);

	cvar->type = CVAR_TYPE_INT;
	cvar->int_value = value;
}

void cvar_add_bool(cvar_t* cvar, const char* name, const char* description, bool value, cvar_flags_t flags)
{
	cvar_add(cvar, name, description, flags);

	cvar->type = CVAR_TYPE_BOOL;
	cvar->bool_value = value;
}

void cvar_add_float(cvar_t* cvar, const char* name, const char* description, float value, cvar_flags_t flags)
{
	cvar_add(cvar, name, description, flags);

	cvar->type = CVAR_TYPE_FLOAT;
	cvar->float_value = value;
}

cvar_t* cvar_find(const char* name)
{
	assert(name);
    assert(hash_map.buckets);

	u16 index = hash_map_get(&hash_map, name, strlen(name));
	return cvars[index];
}

const char* cvar_complete(const char* name)
{
	assert(name);

	return 0;
}

void cvar_set_int(cvar_t* cvar, i32 value)
{
	assert(cvar);

	cvar->int_value = value;
	cvar->flags |= CVAR_MODIFIED;
}

void cvar_set_bool(cvar_t* cvar, bool value)
{
	assert(cvar);

	cvar->bool_value = value;
	cvar->flags |= CVAR_MODIFIED;
}

void cvar_set_float(cvar_t* cvar, float value)
{
	assert(cvar);

	cvar->float_value = value;
	cvar->flags |= CVAR_MODIFIED;
}
