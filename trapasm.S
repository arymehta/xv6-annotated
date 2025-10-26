#include "mmu.h"

  # vectors.S sends all traps here.
  # Aryan: Creates the trapframe -- ESSENTIAL for schedular process, Interrupt handling etc
.globl alltraps
alltraps:
  # Build trap frame.
  pushl %ds
  pushl %es
  pushl %fs
  pushl %gs
  pushal

  # Set up data segments.
  movw $(SEG_KDATA<<3), %ax
  movw %ax, %ds
  movw %ax, %es

  # Call trap(tf), where tf=%esp
  pushl %esp # Aryan: Pointer to trapframe -- calling convention, pushed in reverse order ** USERSPACE STACK **
  call trap
  addl $4, %esp

  # Return falls through to trapret...
.globl trapret
trapret:
  popal
  popl %gs
  popl %fs
  popl %es
  popl %ds
  addl $0x8, %esp  # trapno and errcode
  iret
