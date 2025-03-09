#ifndef __WATCHPOINT_H__
#define __WATCHPOINT_H__

#include "common.h"

typedef struct watchpoint {
  int NO;
  struct watchpoint *next;
  int value;
  /* TODO: Add more members if necessary */
  char expr[256];       //存储监视表达式
  int value;            //记录表达式的计算值

} WP;

void print_watchpoints();
static WP wp_pool[NR_WP];
static WP *head, *free_;
static int used_next;
static WP *wptemp;

#endif
