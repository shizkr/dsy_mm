#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "debug.h"
#include "diagonal.h"

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
#define VIN     ((double)2300.0)  /* mm/s */
#define VMAX    ((double)4000.0)  /* mm/s */
#define ACCEL   ((double)9000.0)  /* mm/s^2 */

#define D45_STRAIGHT_IN  ((double)45.0) /* mm */

/* fixed information */
#define BLOCK    ((double)180.0)      /* mm  */
#define DIAG_BLK ((double)127.2792)   /* mm  */
#define TPERIOD  ((double)0.00001)    /* sec */
#define PI       ((double)3.14159265) /* pi  */
#define TAN22_5  ((double)0.41421356) /* tan(22.5) */

#define GEN_FILENAME "core/load_table.c"
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

char *turn_string[SL_MAX] = {
	"SL_FF",
	"SL_FLR",
	"SL_FLF",
	"SL_FLLR",
	"SL_FLLF",
	"SL_FRL",
	"SL_FRF",
	"SL_FRRL",
	"SL_FRRF",
	"SL_LF",
	"SL_LLR",
	"SL_LLF",     /* -135'out */
	"SL_RF",	  /*  45' out */
	"SL_RRL",     /*  90' in  */
	"SL_RRF",     /*  135 out */
	"SL_LR",
	"SL_RL",
	"SL_F3",      /* forward speed load table */
	"SL_F4",
	"SL_F5",
	"SL_F6",
	"SL_F7",
	"SL_F8",
	"SL_F9",
	"SL_F10",
	"SL_F11",
	"SL_F12",
	"SL_F13",
	"SL_F14",
	"SL_F15",
	"SL_D2",
	"SL_D3",
	"SL_D4",
	"SL_D5",
	"SL_D6",
	"SL_D7",
	"SL_D8",
	"SL_D9",
	"SL_D10",
	"SL_D11",
	"SL_D12",
	"SL_D13",
	"SL_D14",
	"SL_D15",
	"SL_D16",
	"SL_D17",
	"SL_D18",
	"SL_D19",
	"SL_D20",
	"SL_D21",
	"SL_D22",
	"SL_D23",
	"SL_D24",
	"SL_D25",
	"SL_D26",
	"SL_D27"
};

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
int diagonal_45_turn(double straight_in)
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

int gen_table(FILE *fd, int stage)
{
	int f_45, f_90, f_135, f_180;
	int d_45, d_90, d_135;
	int i, time;

	if (fd == NULL)
		return -1;

	f_45 = diagonal_45_turn(D45_STRAIGHT_IN);
	f_90 = F90_LOAD();
	f_135 = diagonal_135_turn();
	f_180 = F180_LOAD();
	d_45 = diagonal_45_turn(D45_STRAIGHT_IN);
	d_90 = D90_LOAD();
	d_135 = diagonal_135_turn();

	fprintf(fd, "#include \"diagonal.h\"\n\n");
	fprintf(fd, "unsigned int load_time_%d[SL_MAX] = {\n", stage);

	for (i = 0; i < SL_MAX; i++) {
		switch (i) {
		case 0:
			time = forward_load(BLOCK);
			break;
		case 1:
		case 5:
			time = f_45;
			break;
		case 2:
		case 6:
			time = f_90;
			break;
		case 3:
		case 7:
			time = f_135;
			break;
		case 4:
		case 8:
			time = f_180;
			break;
		case 9:
		case 12:
			time = d_45;
			break;
		case 10:
		case 13:
			time = d_90;
			break;
		case 11:
		case 14:
			time = d_135;
			break;
		case 15:
		case 16:
			time = forward_diagonal_load(DIAG_BLK);
			break;
		default:
			time = 0;
		}
		/* f2 ~ f15 */
		if (i >= 17 && i <= 29)
			time = forward_load(BLOCK*(i-15));
		/* d2 ~ d27 */
		if (i >= 30 && i <= 55)
			time = forward_diagonal_load(DIAG_BLK*(i-28));

		fprintf(fd, "\t%d, /* %d, %s */\n", time, i, turn_string[i]);
	}

	fprintf(fd, "};\n");

	return 0;
}

int main(int argc, char *argv[])
{
	int i, time;

	FILE *fd = NULL;

	fd = fopen(GEN_FILENAME, "w+");

	if (NULL == fd) {
		print_error("fopen() Error!!!\n");
		exit(EXIT_FAILURE);
	}

	if (gen_table(fd, 1))
		printf("Failed to generate load table for the turns\n");

	time = forward_load(BLOCK);
	print_info("FF took: %d mS\n", time);

	/* forward */
	for (i = 2; i < 16; i++) {
		time = forward_load((double)i*BLOCK);
		print_info("%d BLOCK took: %d mS\n",
				i, time);
	}

	time = forward_diagonal_load(DIAG_BLK);
	print_info("D1 took: %d mS\n", time);

	/* diagonal forward */
	for (i = 2; i < 28; i++) {
		time = forward_diagonal_load((double)i*DIAG_BLK);
		print_info("%d diagonal block took: %d mS\n",
				i, time);
	}

	/* forward 45 in */
	print_info("FRL/FLR took: %d mS\n",
			diagonal_45_turn(D45_STRAIGHT_IN));

	/* forward 90 in/out */
	print_info("FLF/FRF took: %d mS\n", F90_LOAD());

	/* forward 135 in */
	print_info("FLLR/FRRL took: %d mS\n", diagonal_135_turn());

	/* forward 180 in/out */
	print_info("FLLF/FRRF took: %d mS\n", F180_LOAD());

	/*
	 * diagnal out turns
	 */

	/* diagonal 45 out */
	print_info("LF/RF took: %d mS\n",
			diagonal_45_turn(D45_STRAIGHT_IN));

	/* diagonal 90 turn */
	print_info("LLR/RRL took: %d mS\n", D90_LOAD());

	/* diagonal 135 out */
	print_info("LLF/RRF took: %d mS\n", diagonal_135_turn());

	fclose(fd);

	exit(EXIT_SUCCESS);
}



