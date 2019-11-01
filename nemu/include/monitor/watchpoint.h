#ifndef __WATCHPOINT_H__
#define __WATCHPOINT_H__

#include "common.h"

typedef struct watchpoint {
	int NO;
	struct watchpoint *next;

	/* TODO: Add more members if necessary */
	char str[32];
	uint32_t val;

} WP;

void new_wp(char *);

void free_wp(int n);

bool check_wp();
#endif
