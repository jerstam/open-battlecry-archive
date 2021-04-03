#pragma once

#include "common.h"

void file_init(void);
bool file_exists(const char* file_name);
const char* file_get_base_path(void);
const char* file_get_user_path(void);