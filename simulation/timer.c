#include <stdlib.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>

#include "run_mouse.h"
#include "timer.h"

#define TAG "timer: "
#include "debug.h"


/* save the maze file pointer */
static char *timer_mazefile;

void timer_rearm(int msec)
{
	struct itimerval value;

	printf("=>%s\n", __func__);
	getitimer(ITIMER_VIRTUAL, &value);
	value.it_value.tv_sec = 0;
	value.it_value.tv_usec = msec * 1000;
	setitimer(ITIMER_VIRTUAL, &value, NULL);
}

static void timer_handler (int signum)
{
	static int count;

	printf("=>timer expired %d times\n", ++count);
	if (simul_mouse_search_run(timer_mazefile) == 2)
		timer_rearm(0);

	printf("<=timer expired\n");
}

void run_timer(char *maze_file)
{
	struct sigaction sa;
	struct itimerval timer;
	time_t t1, t2;
	double elapsed;

	if (maze_file == NULL)
		print_exit("%s: maze_file is NULL.\n", __func__);
	timer_mazefile = maze_file;

	/* Install timer_handler as the signal handler for SIGVTALRM. */
	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = &timer_handler;
	sigaction(SIGVTALRM, &sa, NULL);

	/* Configure the timer to expire after 250 msec... */
	timer.it_value.tv_sec = 0;
	timer.it_value.tv_usec = 100000;
	/* and every 250 msec after that. */
	timer.it_interval.tv_sec = 0;
	timer.it_interval.tv_usec = 1000;
	/* Start a virtual timer. It counts down whenever this process is
	executing. */
	setitimer(ITIMER_VIRTUAL, &timer, NULL);

#ifdef DEBUG
	t1 = clock();
	t2 = clock();

	elapsed = ((double)(t2 - t1)/(double)CLOCKS_PER_SEC)*
		1000.0;      /* sec to ms */
	printf("elapsed time:%fmS\n", elapsed);
#endif
}
