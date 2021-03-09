#pragma once

#include "common.h"

typedef struct bucket_t
{
	u32 key_hash;
	u16 value;
	u16	probe_sequence_length;
} bucket_t;

typedef struct hash_map_t
{
	u16	count;
	u16	capacity;
	u32	seed;
	u64	div_info;
	bucket_t* buckets;
} hash_map_t;

void hash_map_init(hash_map_t* hash_map, u16 capacity);
void hash_map_free(hash_map_t* hash_map);

void hash_map_add(hash_map_t* hash_map, const char* key, u64 key_length, u16 value);
u16 hash_map_get(hash_map_t* hash_map, const char* key, u64 key_length);
u16 hash_map_remove(hash_map_t* hash_map, const char* key, u64 key_length);