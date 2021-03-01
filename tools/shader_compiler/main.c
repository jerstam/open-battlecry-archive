#define DMON_IMPL
#define DMON_MAX_WATCHES 4
#include "dmon.h"
#include "spirv_reflect.h"

#include <shaderc/shaderc.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>

enum
{
	MAX_SHADER_FILE_SIZE = 1024 * 4
};

static char watch_path[MAX_PATH];
static shaderc_compiler_t compiler;

static void watch_callback(dmon_watch_id watch_id, dmon_action action,
	const char* dirname, const char* filename, const char* oldname, void* user)
{
	char* dot = strrchr(filename, '.');
	if (dot == NULL)
		return;

	if (dot[1] == 'v' || dot[1] == 'f' || (dot[1] == 'c' && dot[2] == 'o'))
	{
		printf("Compiling %s\n", filename);

		char file_path[MAX_PATH];
		sprintf(file_path, "%s%s", dirname, filename);

		FILE* file = fopen(file_path, "r");
		assert(file);

		fseek(file, 0, SEEK_END);
		long size = ftell(file);
		rewind(file);
		assert(size <= MAX_SHADER_FILE_SIZE);

		char source[MAX_SHADER_FILE_SIZE];
		fread(source, size, 1, file);
		fclose(file);

		shaderc_shader_kind shader_kind = shaderc_glsl_vertex_shader;
		if (dot[1] == 'f')
			shader_kind = shaderc_glsl_fragment_shader;
		else if (dot[1] == 'c')
			shader_kind = shaderc_glsl_compute_shader;

		shaderc_compilation_result_t result = shaderc_compile_into_preprocessed_text(
			compiler, 
			source, 
			strlen(source), 
			shader_kind,
			file_path,
			"main",
			NULL);
		assert(result);

		size_t error_count = shaderc_result_get_num_errors(result);
		if (error_count > 0)
		{
			printf("%s\n", shaderc_result_get_error_message(result));
		}

		uint32_t* bytes = (uint32_t*)shaderc_result_get_bytes(result);
	}
}

int main(int argc, char* argv[])
{
	compiler = shaderc_compiler_initialize();

	char exe_path[MAX_PATH];
	GetCurrentDirectoryA(MAX_PATH, exe_path);

	char* pos = strstr(exe_path, "\\tools");
	assert(pos);
	*pos = '\0';

	strcpy(watch_path, exe_path);
	strcat(watch_path, "\\src\\client\\shaders\\");

	printf("Watching: %s\n", watch_path);

	dmon_init();
	dmon_watch_id watch_id = dmon_watch(watch_path, watch_callback, DMON_WATCHFLAGS_RECURSIVE, NULL);

	getchar();

	/*DWORD file_attributes = GetFileAttributes(watch_path);
	assert(file_attributes != INVALID_FILE_ATTRIBUTES);

	directory = CreateFile(
		watch_path,
		FILE_LIST_DIRECTORY,
		FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
		NULL,
		OPEN_EXISTING,
		FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,
		NULL);
	assert(directory != INVALID_HANDLE_VALUE);

	iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, 1);
	assert(iocp);

	close_event = CreateEvent(NULL, TRUE, FALSE, NULL);
	assert(close_event);

	watch_thread = CreateThread(0, 0, watch_directory, NULL, 0, 0);
	assert(watch_thread);

	WaitForSingleObject(watch_thread, INFINITE);
	*/

	dmon_unwatch(watch_id);
	dmon_deinit();
	shaderc_compiler_release(compiler);

	return 0;
}
