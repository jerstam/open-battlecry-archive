#include "window.h"
#include "render.h"
#include "resource.h"
#include "../core/os.h"
#include "../core/log.h"
#include "../core/cvar.h"

int main(int argc, char* argv[])
{
	(void)argc;
	(void)argv;

	os_init();
	log_init("battlecry.log");
	cvar_load();
	window_init();
	render_init();

	const double ticks_to_ms = (double)(1.0 / os_time_frequency());

	u64 last_tick = os_tick();
	while (!window_should_close())
	{
		u64 tick = os_tick();
		float delta_time = (float)(tick * ticks_to_ms * 1000.0f);
		last_tick = tick;

		if (delta_time > 0.15f)
			delta_time = 0.05f;

		window_poll_events();

		if (window_minimized())
		{
			os_sleep(1000);
			return;
		}

		render_frame();
	}

	render_quit();
	window_quit();
	log_quit();
	return 0;
}