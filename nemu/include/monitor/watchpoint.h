#ifndef __WATCHPOINT_H__
#define __WATCHPOINT_H__

#include "common.h"


typedef struct watchpoint {
  int NO;
  struct watchpoint *next;
  
  /* TODO: Add more members if necessary */
  char expr[256];       //存储监视表达式
  int value;            //记录表达式的计算值
  int num;
} WP;

int used;
void init_wp_pool();
bool new_wp();
bool free_wp(int);
bool check_wp();
void print_watchpoints();



#endif
