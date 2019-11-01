#include "monitor/watchpoint.h"
#include "monitor/expr.h"

#define NR_WP 32

static WP wp_pool[NR_WP];
static WP *head, *free_;

void init_wp_pool() {
	int i;
	for(i = 0; i < NR_WP; i ++) {
		wp_pool[i].NO = i;
		wp_pool[i].next = &wp_pool[i + 1];
	}
	wp_pool[NR_WP - 1].next = NULL;

	head = NULL;
	free_ = wp_pool;
}

/* TODO: Implement the functionality of watchpoint */
void new_wp(char *args) {
	if(free_ == NULL) {
		assert(0);
	}

	int n = free_->NO;
	free_ = free_->next;
	Log("set breakpoint[%d]\n", n);

	wp_pool[n].next = head;
	head = wp_pool + n;

	bool success = true;
	strcpy(wp_pool[n].str, args);
	uint32_t val = expr(args, &success);
	if(!success) {
		assert(0);
	}
	wp_pool[n].val = val;
}

void free_wp(int n) {
	bool is_used = false;
	WP *cur = head, *pre = NULL;

	while(cur != NULL) {
		Log("breakpoint[%d]\n", cur->NO);

		if(cur->NO == n) {
			is_used = true;

			if(pre != NULL) {
				pre->next = cur->next;
				break;
			}
		}

		pre = cur;
		cur = cur->next;
	}
	if(!is_used) {
		assert(0);
	}

	wp_pool[n].next = free_;
	free_ = wp_pool + n;
}

bool check_wp() {
	WP *cur = head;
	bool is_changed = false;

	while(cur != NULL) {
		Log("breakpoint[%d]\n", cur->NO);

		bool success = true;
		uint32_t val_now = expr(cur->str, &success);
		if(!success) {
			assert(0);
		}

		if(val_now != cur->val) {
			is_changed = true;
			printf("  breakpoint [%d] has been triggered, prev = %08x, now = %08x\n", cur->NO, cur->val, val_now);
			cur->val = val_now;
		}

		cur = cur->next;
	}

	return is_changed;
}

