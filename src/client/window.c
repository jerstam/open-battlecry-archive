#include "window.h"
#include "../cvar.h"

#include <GLFW/glfw3.h>
#include <assert.h>

static GLFWwindow* window;

static cvar_t cv_window_width = {
    .name = "window_width",
    .description = "Width of the window",
    .type = CVAR_TYPE_INT,
    .int_value = 1280,
    .flags = CVAR_SAVE
};

static cvar_t cv_window_height = {
    .name = "window_height",
    .description = "Width of the window",
    .type = CVAR_TYPE_INT,
    .int_value = 720,
    .flags = CVAR_SAVE
};

static cvar_t cv_fullscreen = {
    .name = "fullscreen",
    .type = CVAR_TYPE_BOOL,
    .bool_value = false,
    .flags = CVAR_SAVE
};

void window_init(void)
{
    cvar_register(&cv_window_width);
    cvar_register(&cv_window_height);
    cvar_register(&cv_fullscreen);

    glfwInit();
    assert(glfwVulkanSupported());

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    window = glfwCreateWindow(cv_window_width.int_value, cv_window_height.int_value, "Battlecry", NULL, NULL);
    assert(window);
}

void window_quit(void)
{
    assert(window);
    glfwDestroyWindow(window);
    glfwTerminate();
}

bool window_minimized(void)
{
    return glfwGetWindowAttrib(window, GLFW_ICONIFIED) != 0;
}

bool window_should_close(void)
{
    return glfwWindowShouldClose(window);
}

void window_size(u32* width, u32* height)
{
    glfwGetFramebufferSize(window, width, height);
}

void window_poll_events(void)
{
    glfwPollEvents();
}

GLFWwindow* window_get_handle(void)
{
    assert(window);
    return window;
}

static float get_dpi_factor()
{
    GLFWmonitor* primary_monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* vidmode = glfwGetVideoMode(primary_monitor);

    i32 width_mm, height_mm;
    glfwGetMonitorPhysicalSize(primary_monitor, &width_mm, &height_mm);

    // As suggested by the GLFW monitor guide
    static const float inch_to_mm = 25.0f;
    static const float win_base_density = 96.0f;

    u32 dpi = (u32)(vidmode->width / (width_mm / inch_to_mm));
    float dpi_factor = dpi / win_base_density;
    return dpi_factor;
}

static float get_content_scale_factor()
{
    int fb_width, fb_height;
    glfwGetFramebufferSize(window, &fb_width, &fb_height);
    int win_width, win_height;
    glfwGetWindowSize(window, &win_width, &win_height);

    return (float)(fb_width) / win_width;
}
