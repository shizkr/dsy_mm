#include <stdlib.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <gtk/gtk.h>

#include "run_mouse.h"
#include "timer.h"

#define TAG "timer: "
#include "debug.h"

static gboolean timer_handler(gpointer data)
{
	char *maze_file = (char *)data;
	static int count;

	printf("=>timer expired %d times\n", ++count);
	if (simul_mouse_search_run(maze_file) == 5)
		return 0;

	return 1;
}

void run_timer(char *maze_file)
{
	time_t t1, t2;
	double elapsed;

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
