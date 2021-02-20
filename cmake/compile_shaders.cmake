find_program(GLSLANG_VALIDATOR glslangValidator PATHS "ENV{VULKAN_SDK}/bin/")
if(NOT GLSLANG_VALIDATOR)
    message(FATAL_ERROR "Counldn't find glgslangValidator")
endif()

message(STATUS "Using glslangValidator: ${GLSLANG_VALIDATOR}")

function(compile_shader)
    set(options "")
    set(oneValueArgs SOURCE_FILE OUTPUT_FILE_NAME OUTPUT_FILE_LIST)
    set(multiValueArgs "")
    cmake_parse_arguments(params "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    if (NOT params_SOURCE_FILE)
        message(FATAL_ERROR "compile_shader: SOURCE_FILE argument missing")
    endif()

    set(src_file "${CMAKE_SOURCE_DIR}/${params_SOURCE_FILE}")

    get_filename_component(name ${src_file} NAME_WLE)
    get_filename_component(ext_with_dot ${src_file} EXT)
    string(REGEX REPLACE "\\." "" ext ${ext_with_dot})

    set(out_dir "${CMAKE_SOURCE_DIR}/src/client/shaders")
    set(out_file "${Cout_di}/${name}_${ext}.h")
    set(array_name "${name}_${ext}_spirv")

    set(glslang_command_line
        --target-env vulkan1.2
        -V
        -o ${out_file}
        --vn ${array_name}
        ${src_file})

    add_custom_command(
        OUTPUT ${out_file}
        DEPENDS ${src_file}
        MAIN_DEPENDENCY ${src_file}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${out_dir}
        COMMAND ${GLSLANG_VALIDATOR} ${glslang_command_line})

    # Append the shader header file to the output list
    set(${params_OUTPUT_FILE_LIST} ${${params_OUTPUT_FILE_LIST}} ${out_file} PARENT_SCOPE)
endfunction()