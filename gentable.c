#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "debug.h"

/*
 * Velocity formula
 *
 * V = V0 + aT
 *
 *
 * Distance formula
 *
 * S = (1/2)*a*T^2 + V0*T
 */

/***************************************************
 * USER needs to modify this parameters.
 *
 ***************************************************/
#define VIN     ((double)1800.0)  /* mm/s */
#define VMAX    ((double)5000.0)  /* mm/s */
#define ACCEL   ((double)2000.0)  /* mm/s^2 */

/* fixed information */
#define BLOCK    ((double)180.0)      /* mm  */
#define DIAG_BLK ((double)127.2792)   /* mm  */
#define TPERIOD  ((double)0.00001)    /* sec */
#define PI       ((double)3.14159265) /* pi  */

/*
 * Varialbles
 */

/* FRF/FLF */
#define F45_LOAD() (int)((((BLOCK*PI/4.0) + BLOCK) / VIN) * 1000)

/* assume that input speed of all forward pattern has
 * same one. calculate the time based on actual time that
 * mouse takes to the end of pattern.
 */
int forward_load(double dist)
{
	double total_dist = 0.0;
	double half_dist = dist/2.0;
	double total_time = 0.0;
	double curr_speed = 0.0;
	double remain_dist = 0.0;
	int ret = 0;

	while (total_dist < half_dist &&
		   curr_speed < VMAX) {
		total_time += TPERIOD;
		total_dist = VIN * total_time +
		   0.5 * ACCEL * total_time * total_time;
		curr_speed = VIN + ACCEL * total_time;
	}

	if (total_dist >= half_dist)
		ret = (int)((total_time * 2.0) * 1000);
	else {
		if (curr_speed < VMAX)
			print_exit("%s speed caculation failed!\n",
					__func__);
		remain_dist = half_dist - total_dist;
		total_time += (remain_dist/curr_speed);
		ret = (int)((total_time * 2.0) * 1000);
	}

	return ret;
}

int forward_diagonal_load(double dist)
{
	double total_dist = 0.0;
	double half_dist = dist/2.0;
	double total_time = 0.0;
	double curr_speed = 0.0;
	double remain_dist = 0.0;
	int ret = 0;

	while (total_dist < half_dist &&
		   curr_speed < VMAX) {
		total_time += TPERIOD;
		total_dist = VIN * total_time +
		   0.5 * ACCEL * total_time * total_time;
		curr_speed = VIN + ACCEL * total_time;
	}

	if (total_dist >= half_dist)
		ret = (int)((total_time * 2.0) * 1000);
	else {
		if (curr_speed < VMAX)
			print_exit("%s speed caculation failed!\n",
					__func__);
		remain_dist = half_dist - total_dist;
		total_time += (remain_dist/curr_speed);
		ret = (int)((total_time * 2.0) * 1000);
	}

	return ret;
}

int main(int argc, char *argv[])
{
	int i, time;

	/* forward */
	for (i = 1; i < 16; i++) {
		time = forward_load((double)i*BLOCK);
		printf("%d BLOCK took: %d mS\n",
				i, time);
	}

	/* diagonal forward */
	for (i = 1; i < 28; i++) {
		time = forward_diagonal_load((double)i*DIAG_BLK);
		printf("%d diagonal block took: %d mS\n",
				i, time);
	}

	/* forward 45 in */
	printf("FLF/FRF took: %d mS\n",
			F45_LOAD());

	exit(EXIT_SUCCESS);
}
