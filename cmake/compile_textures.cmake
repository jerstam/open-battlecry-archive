find_program(COMPRESSONATOR compressonatorcli PATHS "${CMAKE_SOURCE_DIR}/assets/images")
if(NOT COMPRESSONATOR)
    message(FATAL_ERROR "Counldn't find Compressonator")
endif()

message(STATUS "Using Compressonator: ${COMPRESSONATOR}")

function(compile_texture)
    set(options "")
    set(oneValueArgs SOURCE_FILE OUTPUT_FILE_NAME OUTPUT_FILE_LIST)
    set(multiValueArgs "")
    cmake_parse_arguments(params "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    if (NOT params_SOURCE_FILE)
        message(FATAL_ERROR "compile_textures: SOURCE_FILE argument missing")
    endif()

    set(src_file "${CMAKE_SOURCE_DIR}/${params_SOURCE_FILE}")

    get_filename_component(name ${src_file} NAME_WLE)
    get_filename_component(ext_with_dot ${src_file} EXT)
    string(REGEX REPLACE "\\." "" ext ${ext_with_dot})

    set(out_dir "${CMAKE_BINARY_DIR}/resources/textures")
    set(out_file "${out_dir}/${name}.png")

    set(command_line
        -fd BC7
        ${src_file}
        ${out_file})

    add_custom_command(
        OUTPUT ${out_file}
        DEPENDS ${src_file}
        MAIN_DEPENDENCY ${src_file}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${out_dir}
        COMMAND ${COMPRESSONATOR} ${command_line})

    # Append the texture file to the output list
#    set(${params_OUTPUT_FILE_LIST} ${${params_OUTPUT_FILE_LIST}} ${out_file} PARENT_SCOPE)
endfunction()