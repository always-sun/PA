#include "proc.h"

#define MAX_NR_PROC 4

static PCB pcb[MAX_NR_PROC];
static int nr_proc = 0;
PCB *current = NULL;

uintptr_t loader(_Protect *as, const char *filename);

void load_prog(const char *filename) {
  int i = nr_proc ++;
  _protect(&pcb[i].as);

  uintptr_t entry = loader(&pcb[i].as, filename);

  // TODO: remove the following three lines after you have implemented _umake()
  //_switch(&pcb[i].as);
  //current = &pcb[i];
  //((void (*)(void))entry)();

  _Area stack;
  stack.start = pcb[i].stack;
  stack.end = stack.start + sizeof(pcb[i].stack);

  pcb[i].tf = _umake(&pcb[i].as, stack, stack, (void *)entry, NULL, NULL);
}



int current_game = 0;



_RegSet* schedule(_RegSet *prev) {
  static int counter = 0;
  const int RATIO = 10;  

  if (current != NULL) {
    current->tf = prev;
  }

  //current = &pcb[0];  
  //current = (current == &pcb[0] ? &pcb[1] : &pcb[0]);
  if (current == NULL || current == &pcb[1]) {
    // 切回仙剑
    //current = &pcb[0];
    current = (current_game == 0 ? &pcb[0] : &pcb[2]);
    counter = 0;
  } else {
    // 当前是 pal
    if (counter < RATIO) {
      counter++;
      current = (current_game == 0 ? &pcb[0] : &pcb[2]);
    } else {
      current = &pcb[1];  // 切换到 hello
    }
  }

  Log("Switching to process with page dir PTR=0x%08x\n", (uint32_t)current->as.ptr);

  _switch(&current->as);
  return current->tf;

}