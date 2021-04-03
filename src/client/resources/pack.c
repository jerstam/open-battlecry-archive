#include "pack.h"

#include <stdio.h>
#include <string.h>
#include <assert.h>

void pack_write(const char* pack_name, const char** file_names, uint32_t file_count)
{
	assert(pack_name);
	assert(file_names);
	assert(file_count > 0);

	FILE* file = fopen(pack_name, "wb");
	assert(file);

	pack_header_t header = { 0 };
	memcpy(&header.identifier[0], "pack", 4);
	header.file_count = file_count;
	header.version = 0x01;

	fwrite(&header, sizeof(header), 1, file);

	for (int i = 0; i < file_count; i++)
	{
		char* dot = strrchr(file_names[i], '.');
		if (dot == NULL)
			continue;

		pack_resource_t resource = { 0 };

		if (dot[1] == 'k' && dot[2] == 't' && dot[3] == 'x')
		{
			resource.offset = ftell(file);
			resource.type = RESOURCE_TEXTURE;

		}
	}

	fclose(file);
}
