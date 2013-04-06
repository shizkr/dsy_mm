#ifndef TIMER_H
#define TIMER_H

#define TIMER_MS    20
#define TIMER_S    ((double)TIMER_MS/1000.0)

void timer_rearm(int msec);
void run_timer(unsigned char *maze_file);

#endif
