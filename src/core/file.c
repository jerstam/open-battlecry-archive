#include "file.h"

#include <ShlObj.h>
#include <Shlwapi.h>
#include <stdio.h>
#include <assert.h>

static char base_path[MAX_PATH];
static char user_path[MAX_PATH];

void file_init(void)
{
	GetCurrentDirectoryA(MAX_PATH, base_path);
	SHGetFolderPathA(NULL, CSIDL_PROFILE, NULL, 0, user_path);
}

bool file_exists(const char* file_name)
{
	return PathFileExistsA(file_name);
}

static void swap_slashes(const char* file_name, char* win_file_name)
{
    assert(file_name);
    assert(win_file_name);

    const char* src_char = file_name;
    char* dest_char = win_file_name;

    u32 length = strlen(file_name);

    while (src_char < file_name + length)
    {
        if (*src_char == '/')
            *dest_char = '\\';
        else if (*src_char == '@')
            *dest_char = ':';
        else
            *dest_char = *src_char;

        dest_char++;
        src_char++;
    }
    *dest_char = '\0';
}

const char* file_get_base_path(void)
{
	return &base_path[0];
}

const char* file_get_user_path(void)
{
	return &user_path[0];
}
