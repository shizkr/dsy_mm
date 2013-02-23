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

#define D45_STRAIGHT_IN  ((double)45.0) /* mm */
#define D135_STRAIGHT_IN  ((double)10.0) /* mm */

/* fixed information */
#define BLOCK    ((double)180.0)      /* mm  */
#define DIAG_BLK ((double)127.2792)   /* mm  */
#define TPERIOD  ((double)0.00001)    /* sec */
#define PI       ((double)3.14159265) /* pi  */
#define TAN22_5  ((double)0.41421356) /* tan(22.5) */

/*
 * Varialbles
 */

/* FRF/FLF */
/* total straight length is 1 BLOCK, length of circular arc is 2*PI*R/4 */
#define F90_LOAD() (int)((((BLOCK*PI/4.0) + BLOCK) / VIN) * 1000)

/* FRRF/FLLF */
/* total straight length is 1 BLOCK, length of circular arc is 2*PI*R/2 */
#define F180_LOAD() (int)((((BLOCK*PI/2.0) + BLOCK) / VIN) * 1000)

/* RRL/LLR */
/* total straight length is 1 BLOCK, length of circular arc is 2*PI*R/2 */
#define D90_LOAD() (int)(((DIAG_BLK*PI/2.0) / VIN) * 1000)

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

/* FLR/FRL */
/* FLR/FRL diagonal 45 degree turn in calculation. the length and time have
 * same value with LF/RF 45 degree turn out case.
 */
int diagonal_45_trun(double straight_in)
{
	double r;
	double time;
	double arc;
	double diag_rest;
	double len;

	if (straight_in >= BLOCK/2.0)
		return 0;
	r = (BLOCK/2.0 - straight_in)/TAN22_5;
	arc = PI*r/4.0;

	diag_rest = DIAG_BLK - (BLOCK/2.0 - straight_in);
	len = straight_in + diag_rest + arc;
	time = len / VIN * 1000.0;

	return (int)time;
}

/* LLR/RRL */
/* LLR/RRL diagonal 135 degree turn out calculation. the length and time have
 * same value with FRRL/FLLR 135 degree turn in case.
 */
int diagonal_135_turn(void)
{
	double r;
	double time;
	double arc;
	double rest;
	double len;

	r = DIAG_BLK*2.0*TAN22_5;
	arc = 2.0*PI*r*(135.0/360.0);

	rest = (BLOCK * 1.5) - (DIAG_BLK * 2.0);
	len = rest + arc;
	time = len / VIN * 1000.0;

	return (int)time;
}

int main(int argc, char *argv[])
{
	int i, time;


	time = forward_load(BLOCK);
	printf("FF took: %d mS\n", time);

	/* forward */
	for (i = 2; i < 16; i++) {
		time = forward_load((double)i*BLOCK);
		printf("%d BLOCK took: %d mS\n",
				i, time);
	}

	time = forward_diagonal_load(DIAG_BLK);
	printf("D1 took: %d mS\n", time);

	/* diagonal forward */
	for (i = 2; i < 28; i++) {
		time = forward_diagonal_load((double)i*DIAG_BLK);
		printf("%d diagonal block took: %d mS\n",
				i, time);
	}

	/* forward 45 in */
	printf("FRL/FLR took: %d mS\n",
			diagonal_45_trun(D45_STRAIGHT_IN));

	/* forward 90 in/out */
	printf("FLF/FRF took: %d mS\n", F90_LOAD());

	/* forward 135 in */
	printf("FLLR/FRRL took: %d mS\n", diagonal_135_turn());

	/* forward 180 in/out */
	printf("FLLF/FRRF took: %d mS\n", F180_LOAD());

	/*
	 * diagnal out turns
	 */

	/* diagonal 45 out */
	printf("LF/RF took: %d mS\n",
			diagonal_45_trun(D45_STRAIGHT_IN));

	/* diagonal 90 turn */
	printf("LLR/RRL took: %d mS\n", D90_LOAD());

	/* diagonal 135 out */
	printf("LLF/RRF took: %d mS\n", diagonal_135_turn());

	exit(EXIT_SUCCESS);
}



