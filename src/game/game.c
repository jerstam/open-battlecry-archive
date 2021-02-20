#include "game.h"
#include "../log.h"
#include "../jobs.h"
#include "../client/input.h"
#include "../client/resource.h"

#include <math.h>
#include <string.h>

static bool pressed;
static bool unpressed;

static void worker_function(void* data, uintptr_t arg)
{
	for (int i = 0; i < 100000; ++i)
	{
		double a = sin(i);
	}
}

void game_init(void)
{	
	jobs_init();

	resource_load_texture("AHHXw.KTX2", NULL);
	//resource_load_texture("assets/textures/knights/AHHXw.KTX2", NULL);
}

void game_quit(void)
{
	jobs_quit();
}

void game_load(void)
{

}

void game_unload(void)
{
	jobs_wait_idle();
}

void game_update(float delta_time)
{
	for (int i = 0; i < 100000; ++i)
	{
		double a = sin(i);
	}

	jobs_add_count(worker_function, NULL, jobs_thread_count());

	if (input_key(KEY_F5) && !pressed)
	{
		pressed = true;
		log_info("Started capture...");
	}

	if (input_key(KEY_F6) && !unpressed)
	{
		unpressed = true;
        log_info("Stopped capture.");
	}

	jobs_wait_idle();
}
