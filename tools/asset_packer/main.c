#include "png.h"
#include "bc7.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

static void process_dir(const TCHAR* path)
{
	HANDLE find = INVALID_HANDLE_VALUE;
	WIN32_FIND_DATA find_data;

	TCHAR search_path[MAX_PATH];
	wcscpy(search_path, path);
	wcscat(search_path, TEXT("\\*"));

	find = FindFirstFile(search_path, &find_data);
	if (find == INVALID_HANDLE_VALUE)
	{
		return;
	}

	int subdir_count = 0;
	TCHAR subdirs[28][MAX_PATH];

	do
	{
		if (find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			TCHAR* dir_name = find_data.cFileName;

			if (dir_name[0] != '.')
			{
				wcscpy(subdirs[subdir_count], path);
				wcscat(subdirs[subdir_count], TEXT("\\"));
				wcscat(subdirs[subdir_count], dir_name);
				subdir_count++;
				wprintf(TEXT("%s\n"), dir_name);
			}
		}
		else
		{
			TCHAR* dot = wcsrchr(find_data.cFileName, '.');
			if (dot[1] == 'p' && dot[2] == 'n' && dot[3] == 'g')
			{
				wprintf(TEXT("  %s\n"), find_data.cFileName);

				char mbs_file_name[32];
				WideCharToMultiByte(CP_UTF8, 0, find_data.cFileName, 32, mbs_file_name, 32, NULL, NULL);

				char mbs_path[MAX_PATH];
				wcstombs(mbs_path, path, MAX_PATH);
				strcat(mbs_path, "\\");
				strcat(mbs_path, mbs_file_name);


			}
		}
	} while (FindNextFile(find, &find_data) != 0);

	for (int i = 0; i < subdir_count; i++)
	{
		process_dir(subdirs[i]);
	}

	FindClose(find);
}

int _tmain(int argc, TCHAR* argv[])
{
	(void)argc;
	(void)argv;

	png_init(5400, 2400);
	bc7_init();

	bc7_quit();
	png_quit();

	return 0;
}