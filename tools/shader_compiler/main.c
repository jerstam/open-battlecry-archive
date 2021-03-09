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

	char* path_dot = strrchr(file_path, '.');
	if (path_dot == NULL)
		return;

	if (path_dot[1] == 'v' || path_dot[1] == 'f' || (path_dot[1] == 'c' && path_dot[2] == 'o'))
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
		if (path_dot[1] == 'f')
			shader_kind = shaderc_glsl_fragment_shader;
		else if (path_dot[1] == 'c')
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

		const size_t code_size = shaderc_result_get_length(result);
		const uint32_t* code = (const uint32_t*)shaderc_result_get_bytes(result);

		path_dot[0] = '\0';
		switch (shader_kind)
		{
			case shaderc_glsl_vertex_shader:
				strcat(path_dot, "_vert.h");
				break;
			case shaderc_glsl_fragment_shader:
				strcat(path_dot, "_frag.h");
				break;
			case shaderc_glsl_compute_shader:
				strcat(path_dot, "_comp.h");
				break;
		}

		// Shader name in the form "shader_vert"
		char shader_name[32];
		strcpy(shader_name, filename);
		char* name_dot = strrchr(shader_name, '.');
		name_dot[0] = '_';

		file = fopen(file_path, "w");
		assert(file);

		const uint32_t u32_count = (uint32_t)(code_size / sizeof(uint32_t));

		fprintf(file, "#pragma once\n\n#include \"../core/common.h\"\n\nconst u32 %s_spirv[%d] = {\n", shader_name, u32_count);
		
		for (uint32_t i = 0; i < u32_count;)
		{
			int u32_left = u32_count - i;
			if (u32_left >= 8)
			{
				fprintf(file, "0x%08x,0x%08x,0x%08x,0x%08x,0x%08x,0x%08x,0x%08x,0x%08x,\n",
					code[i + 0], code[i + 1], code[i + 2], code[i + 3],
					code[i + 4], code[i + 5], code[i + 6], code[i + 7]);
				i += 8;
			}
			else
			{
				for (int x = 0; x < u32_left; x++)
				{
					if (x == u32_left - 1)
						fprintf(file, "0x%08x\n", code[i]);
					else
						fprintf(file, "0x%08x,", code[i]);
					i++;
				}
			}
		}

		fprintf(file, "};\nconst u64 %s_size = %lld;\n", shader_name, code_size);
		fclose(file);

		// Reflection
		SpvReflectShaderModule module = { 0 };
		SpvReflectResult reflect_result = spvReflectCreateShaderModule(code_size, code, &module);
		assert(reflect_result == SPV_REFLECT_RESULT_SUCCESS);

		// Input variables
		uint32_t input_variable_count = 0;
		reflect_result = spvReflectEnumerateInputVariables(&module, &input_variable_count, NULL);
		assert(reflect_result == SPV_REFLECT_RESULT_SUCCESS);

		SpvReflectInterfaceVariable* input_variables[8];
		reflect_result = spvReflectEnumerateInputVariables(&module, &input_variable_count, input_variables);
		assert(reflect_result == SPV_REFLECT_RESULT_SUCCESS);

		// Output variables
		uint32_t output_variable_count = 0;
		reflect_result = spvReflectEnumerateOutputVariables(&module, &output_variable_count, NULL);
		assert(reflect_result == SPV_REFLECT_RESULT_SUCCESS);

		SpvReflectInterfaceVariable* output_variables[8];
		reflect_result = spvReflectEnumerateOutputVariables(&module, &output_variable_count, output_variables);
		assert(reflect_result == SPV_REFLECT_RESULT_SUCCESS);

		// Push constants
		uint32_t push_constant_count = 0;
		reflect_result = spvReflectEnumeratePushConstantBlocks(&module, &push_constant_count, NULL);
		assert(reflect_result == SPV_REFLECT_RESULT_SUCCESS);

		SpvReflectBlockVariable* push_constant_blocks[4];
		reflect_result = spvReflectEnumeratePushConstantBlocks(&module, &push_constant_count, push_constant_blocks);
		assert(reflect_result == SPV_REFLECT_RESULT_SUCCESS);

		// Descriptor sets
		uint32_t descriptor_set_count = 0;
		reflect_result = spvReflectEnumerateDescriptorSets(&module, &descriptor_set_count, NULL);
		assert(reflect_result == SPV_REFLECT_RESULT_SUCCESS);

		SpvReflectDescriptorSet* descriptor_sets[8];
		reflect_result = spvReflectEnumerateDescriptorSets(&module, &descriptor_set_count, descriptor_sets);
		assert(reflect_result == SPV_REFLECT_RESULT_SUCCESS);

		for (uint32_t i = 0; i < descriptor_set_count; i++)
		{
			const SpvReflectDescriptorSet* set = descriptor_sets[i];

			for (uint32_t b = 0; b < set->binding_count; b++)
			{
				const SpvReflectDescriptorBinding* binding = set->bindings[b];

				printf("  Binding (%d,%d): %d\n", binding->set, b, binding->descriptor_type);
			}
		}

		spvReflectDestroyShaderModule(&module);

		shaderc_result_release(result);
		printf("Compiled %s\n", filename);
	}
}

int main(int argc, char* argv[])
{
	SetConsoleTitleA("Shader Compiler");

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

	(void)getchar();

	dmon_unwatch(watch_id);
	dmon_deinit();
	shaderc_compiler_release(compiler);

	return 0;
}
