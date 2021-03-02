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
	char file_path[MAX_PATH];
	sprintf(file_path, "%s%s", dirname, filename);

	char* dot = strrchr(file_path, '.');
	if (dot == NULL)
		return;

	if (dot[1] == 'v' || dot[1] == 'f' || (dot[1] == 'c' && dot[2] == 'o'))
	{
		FILE* file = fopen(file_path, "r");
		assert(file);

		fseek(file, 0, SEEK_END);
		long size = ftell(file);
		rewind(file);
		assert(size <= MAX_SHADER_FILE_SIZE);

		char source[MAX_SHADER_FILE_SIZE] = { 0 };
		fread(source, 1, size, file);
		fclose(file);
		source[size] = '\0';

		shaderc_shader_kind shader_kind = shaderc_glsl_vertex_shader;
		if (dot[1] == 'f')
			shader_kind = shaderc_glsl_fragment_shader;
		else if (dot[1] == 'c')
			shader_kind = shaderc_glsl_compute_shader;

		shaderc_compile_options_t options = shaderc_compile_options_initialize();
		shaderc_compile_options_set_optimization_level(options, shaderc_optimization_level_performance);
		shaderc_compile_options_set_target_env(options, shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_2);
		shaderc_compile_options_set_target_spirv(options, shaderc_spirv_version_1_5);
		shaderc_compile_options_set_generate_debug_info(options);

		size_t source_length = strlen(source);
		shaderc_compilation_result_t result = shaderc_compile_into_spv(
			compiler, 
			source, 
			source_length,
			shader_kind,
			filename,
			"main",
			options);
		assert(result);

		shaderc_compile_options_release(options);

		shaderc_compilation_status status = shaderc_result_get_compilation_status(result);
		if (status != shaderc_compilation_status_success)
		{
			printf("%s\n", shaderc_result_get_error_message(result));
			fflush(stdout);
			return;
		}

		const size_t bytes_size = shaderc_result_get_length(result);
		const uint32_t* bytes = (const uint32_t*)shaderc_result_get_bytes(result);

		dot[0] = '\0';
		switch (shader_kind)
		{
			case shaderc_glsl_vertex_shader:
				strcat(dot, "_vert.h");
				break;
			case shaderc_glsl_fragment_shader:
				strcat(dot, "_frag.h");
				break;
			case shaderc_glsl_compute_shader:
				strcat(dot, "_comp.h");
				break;
		}

		file = fopen(file_path, "w");
		fwrite(bytes, bytes_size, 1, file);
		fclose(file);

		shaderc_result_release(result);

		printf("Compiled %s\n", filename);
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

	dmon_unwatch(watch_id);
	dmon_deinit();
	shaderc_compiler_release(compiler);

	return 0;
}
