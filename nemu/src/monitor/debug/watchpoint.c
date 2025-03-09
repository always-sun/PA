#include "monitor/watchpoint.h"
#include "monitor/expr.h"

#define NR_WP 32

static WP wp_pool[NR_WP];
static WP *head, *free_;
WP* wptemp;
void init_wp_pool() {
  int i;
  for (i = 0; i < NR_WP; i ++) {
    wp_pool[i].NO = i;
    wp_pool[i].next = &wp_pool[i + 1];
    wp_pool[i].value = 0;
    wp_pool[i].num = 0;
  }
  wp_pool[NR_WP - 1].next = NULL;

  head = NULL;
  free_ = wp_pool;

  used=0;
}

/* TODO: Implement the functionality of watchpoint */
void print_watchpoints() {
  WP* curr = head;
  if (head == NULL) {
    printf("No watchpoints.\n");
    return ;
  }
  while (curr != NULL) {
    printf("Watchpoint:  %d:\t%s\t\t%d\n", curr->NO, curr->expr, curr->num);
    curr = curr->next;
  }

}
bool new_wp(char *args){
    if(free_==NULL)
        assert(0);
    WP* result=free_;
    free_=free_->next;
    result->NO=used;
    used++;
    result->next=NULL;
    strcpy(result->expr,args);
    result->num=0;
    bool success;
    result->value=expr(result->expr, &success);
    if(success==false){
        printf("error in new_wp:expression fault!\n");
        return false;
    }
    wptemp=head;
    if(wptemp==NULL)
        head=result;
    else{
        while(wptemp->next!=NULL)
            wptemp=wptemp->next;
        wptemp->next=result;
    }
    printf("Success: set watchpoint %d,value=%d\n",result->NO,result->value);
    return true;
}


bool free_wp(int num1) {
    WP *thewp = NULL;
    if (head == NULL) {
        printf("no watchpoint now\n");
        return false;
    }

    if (head->NO == num1) {
        thewp = head;
        head = head->next;
    } else {
        WP *wptemp = head;
        while (wptemp != NULL && wptemp->next != NULL) {
            if (wptemp->next->NO == num1) {
                thewp = wptemp->next;
                wptemp->next = wptemp->next->next;
                break;
            }
            wptemp = wptemp->next;
        }
    }

    if (thewp != NULL) {
        thewp->next = free_;
        free_ = thewp;
        return true;
    }

    return false;
}
bool check_wp() {
    bool success;
    int result;
    
    if (head == NULL)
        return true;

    WP *current = head;
    while (current  != NULL) {
        result = expr(current ->expr, &success);
        if (result != current ->value) {
            current ->num += 1;
            printf("Hardware watchpoint %d: %s\n", current ->NO, current ->expr);
            printf("value_past: %d\n value_now: %d\n\n", current ->value, result);
            current ->value = result;
            return false;
        }
        current  = current ->next;
    }
    
    return true;
}
