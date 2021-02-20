#pragma once

#include "../macros.h"

typedef struct GLFWwindow GLFWwindow;

void window_init(void);
void window_quit(void);
void window_poll_events(void);

bool window_minimized(void);
bool window_should_close(void);

GLFWwindow* window_get_handle(void);