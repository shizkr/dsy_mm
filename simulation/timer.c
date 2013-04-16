#include <stdlib.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <gtk/gtk.h>

#include "run_mouse.h"
#include "draw_mouse.h"
#include "timer.h"
#include "maze.h"

#define TAG "timer: "
#include "debug.h"

#ifdef DEBUG
static int debug_flag;
/* = DEBUG_SEARCH; */
#endif

static gboolean timer_handler(gpointer data)
{
	unsigned char *maze_file = (unsigned char *)data;
#ifdef DEBUG
	static int count;
#endif

	print_dbg(DEBUG_SEARCH, "=>timer expired %d times\n", ++count);
	if (simul_mouse_search_run(maze_file) >= 2)
		return 0;

	if (get_mouse_run_gui_state() != MOUSE_RUNNING)
		return 0;

	return 1;
}

void run_timer(unsigned char *maze_file)
{
#ifdef DEBUG
	time_t t1, t2;
	double elapsed;
#endif

	if (maze_file == NULL)
		print_exit("%s: maze_file is NULL.\n", __func__);

	g_timeout_add(TIMER_MS, timer_handler, maze_file);

#ifdef DEBUG
	t1 = clock();
	t2 = clock();

	elapsed = ((double)(t2 - t1)/(double)CLOCKS_PER_SEC)*
		1000.0;      /* sec to ms */
	printf("elapsed time:%fmS\n", elapsed);
#endif
}
