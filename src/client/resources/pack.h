#pragma once

#include <stdint.h>

#define MAX_PATH_LENGTH 255

typedef enum resource_type_t
{
	RESOURCE_TEXTURE,
} resource_type_t;

typedef struct pack_header_t
{
	uint8_t identifier[4];
	uint32_t version;
	uint32_t size;
} pack_header_t;

typedef struct pack_resource_t
{
	char path[MAX_PATH_LENGTH];
	uint8_t type;
	uint32_t size;
	uint32_t offset;
} pack_resource_t;

void pack_write(const char* pack_name, const char** file_names, uint32_t file_count);