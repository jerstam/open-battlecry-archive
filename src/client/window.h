#pragma once

#include "../core/common.h"

typedef enum
{
	WINDOW_BORDERLESS = 0x01,
	WINDOW_CAPTURE_MOUSE = 0x02
} window_flags_t;

typedef struct GLFWwindow GLFWwindow;
typedef struct VkSwapchainKHR_T* VkSwapchainKHR;

void window_init(void);
void window_quit(void);
void window_poll_events(void);

bool window_minimized(void);
bool window_should_close(void);

void window_size(u32* width, u32* height);

GLFWwindow* window_get_handle(void);