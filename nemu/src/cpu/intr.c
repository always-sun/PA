#include "cpu/exec.h"
#include "memory/mmu.h"

void raise_intr(uint8_t NO, vaddr_t ret_addr) {
  /* TODO: Trigger an interrupt/exception with ``NO''.
   * That is, use ``NO'' to index the IDT.
   */

  // 1. 压栈
  rtl_push((rtlreg_t *)&cpu.eflags);
  cpu.eflags.IF = 0;
  rtl_push((rtlreg_t *)&cpu.cs);
  rtl_push((rtlreg_t *)&ret_addr);

  // 2. 读取 IDT
  uint32_t idtr_base = cpu.idtr.base;
  uint32_t eip_low  = vaddr_read(idtr_base + NO * 8, 4);
  uint32_t eip_high = vaddr_read(idtr_base + NO * 8 + 4, 4);

  // 可选：合法性检查
  if ((eip_high & 0x00008000) == 0) assert(0);

  // 3. 拼接目标地址
  uint32_t offset = (eip_low & 0x0000ffff) | (eip_high & 0xffff0000);

  // 4. 跳转
  decoding.jmp_eip = offset;
  decoding.is_jmp = 1;
  
}

void dev_raise_intr() {
}
