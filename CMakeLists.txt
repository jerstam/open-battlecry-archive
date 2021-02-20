cmake_minimum_required(VERSION 3.15.0)
project(battlecry VERSION 0.1.0)

message(STATUS "Using toolchain: ${CMAKE_TOOLCHAIN_FILE}")

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

find_package(Vulkan REQUIRED)
find_package(GLFW3 CONFIG REQUIRED)

# Common
set(SRC_COMMON
    src/cvar.h
    src/cvar.c
    src/hash.h
    src/hash.c
    src/hash_map.h
    src/hash_map.c
    src/jobs.h
    src/jobs.c
    src/log.h
    src/log.c
    src/os.h
    src/os_windows.c
    src/random.h
    src/random.c)

# Tracy
set(SRC_TRACY
    third_party/tracy/common/TracyApi.h
    third_party/tracy/client/TracyProfiler.cpp
    third_party/tracy/TracyC.h
    third_party/tracy/TracyClient.cpp)

add_library(tracy STATIC ${SRC_TRACY})
target_include_directories(tracy INTERFACE third_party/tracy)
target_compile_definitions(tracy INTERFACE -DTRACY_ENABLE=1)

# Game
set(SRC_GAME
    src/game/game.h
    src/game/game.c
    src/game/game_state.h
    src/game/game_state.c)

add_library(game ${SRC_COMMON} ${SRC_GAME})

# Vma
add_library(vma STATIC src/client/vk_mem_alloc.h src/client/vk_mem_alloc.cpp)
set_target_properties(vma PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries(vma PRIVATE Vulkan::Vulkan)

if (WIN32)
    target_compile_definitions(vma PRIVATE -DVK_USE_PLATFORM_WIN32_KHR -DWIN32_LEAN_AND_MEAN)
endif()

# Shaders
set(SRC_SHADERS
    assets/shaders/blit.frag
    assets/shaders/fullscreen.vert
    assets/shaders/line.frag
    assets/shaders/line.vert
    assets/shaders/sprite.frag
    assets/shaders/sprite.vert)

include(cmake/compile_shaders.cmake)

foreach(s ${SRC_SHADERS})
    compile_shader(SOURCE_FILE ${s} OUTPUT_FILE_LIST shader_headers)
endforeach()

add_custom_target(shaders DEPENDS ${shader_headers})

# Client
set(SRC_CLIENT
    src/client/main.c
    src/client/client_windows.c
    src/client/gpu.h
    src/client/gpu_buffer.c
    src/client/gpu.c
    src/client/input.h
    src/client/input.c
    src/client/resource.h
    src/client/resource.c
    src/client/volk.h
    src/client/volk.c
    src/client/window.c
    src/client/window.h
    src/client/renderer.c src/client/renderer.h)

if (WIN32)
    add_executable(client
        ${SRC_COMMON} ${SRC_CLIENT} assets/battlecry.rc)
else()
    add_executable(client
        ${SRC_COMMON} ${SRC_CLIENT})
endif()

set_target_properties(client PROPERTIES LINKER_LANGUAGE C)
target_link_libraries(client PRIVATE glfw Vulkan::Vulkan vma)

if (WIN32)
    target_compile_definitions(client PRIVATE -DVK_USE_PLATFORM_WIN32_KHR -DWIN32_LEAN_AND_MEAN -D_CRT_SECURE_NO_WARNINGS)
endif()

if (MSVC)
    target_compile_options(client PRIVATE "/std:c17")
endif()

# Textures
file(GLOB_RECURSE images "${CMAKE_CURRENT_SOURCE_DIR}/assets/images/*.png")

include(cmake/compile_textures.cmake)

foreach(i ${images})
    compile_texture(SOURCE_FILE ${i} OUTPUT_FILE_LIST texture_files)
endforeach()

add_custom_target(textures DEPENDS ${texture_files})