#ifndef DIAGONAL_H
#define DIAGONAL_H

#include "algo.h"

enum speed_load_enum {
	SL_FF,      /* FF */
	SL_FLR,     /* -45'  in  */
	SL_FLF,     /* -90'  in  */
	SL_FLLR,    /* -135' in  */
	SL_FLLF,    /* -180' in  */
	SL_FRL,     /*  45'  out */
	SL_FRF,     /*  90'  out */
	SL_FRRL,    /*  135' out */
	SL_FRRF,    /*  180' out */
	SL_LF,	    /* -45' out */
	SL_LLR,     /* -90' in  */
	SL_LLF,     /* -135'out */
	SL_RF,	    /*  45' out */
	SL_RRL,     /*  90' in  */
	SL_RRF,     /*  135 out */
	SL_LR,
	SL_RL,
	SL_F3,      /* forward speed load table */
	SL_F4,
	SL_F5,
	SL_F6,
	SL_F7,
	SL_F8,
	SL_F9,
	SL_F10,
	SL_F11,
	SL_F12,
	SL_F13,
	SL_F14,
	SL_F15,
	SL_D2,
	SL_D3,
	SL_D4,
	SL_D5,
	SL_D6,
	SL_D7,
	SL_D8,
	SL_D9,
	SL_D10,
	SL_D11,
	SL_D12,
	SL_D13,
	SL_D14,
	SL_D15,
	SL_D16,
	SL_D17,
	SL_D18,
	SL_D19,
	SL_D20,
	SL_D21,
	SL_D22,
	SL_D23,
	SL_D24,
	SL_D25,
	SL_D26,
	SL_D27,
	SL_MAX
};

enum PATTERN_ENUM {
	P_FF,      /* FF */
	P_FLR,     /* -45'  in  */
	P_FLF,     /* -90'  in  */
	P_FLLR,    /* -135' in  */
	P_FLLF,    /* -180' in  */
	P_FRL,     /*  45'  out */
	P_FRF,     /*  90'  out */
	P_FRRL,    /*  135' out */
	P_FRRF,    /*  180' out */
	P_LF,      /* -45' out */
	P_LLR,     /* -90' in  */
	P_LLF,     /* -135'out */
	P_RF,      /*  45' out */
	P_RRL,     /*  90' in  */
	P_RRF,     /*  135 out */
	P_LR,      /*  diag 1 block */
	P_RL,      /*  diag 1 block */
	P_MAX
};

struct diagonal_type {
	unsigned char *pttn;
	unsigned char size;
};

struct diag_pttn_time_type {
	enum speed_load_enum pttn;
	unsigned int time;
};

struct diagonal_node {
	struct diagonal_node *link[4];
	struct diag_pttn_time_type pttn;
#ifdef DEBUG
	int node_num;
#endif
};

/* default diagonal speed load per pattern */
extern unsigned int default_load_time[SL_MAX];
extern unsigned int load_time_1[];

void diagonal_pattern_tree_init(unsigned int *load_time);
struct diag_pttn_time_type *diagonal_pattern_search(unsigned char *pattern,
		int pttn_size);

#endif
