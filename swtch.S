# Context switch
#
#   void swtch(struct context **old, struct context *new);
#
# Save the current registers on the stack, creating
# a struct context, and save its address in *old.
# Switch stacks to new and pop previously-saved registers.


/* Aryan: swtch() was called as a C FUNCTION in scheduler() and sched()
 * This mean the compiler compiled it using calling convention
 * calling convention -- arguments pushed to stack in reverse order!
 * Thats why we get args by accessing esp!
 *
 * */
.globl swtch
swtch:
  movl 4(%esp), %eax // old
  movl 8(%esp), %edx // new

  # Save old callee-saved registers
  pushl %ebp
  pushl %ebx
  pushl %esi
  pushl %edi
  // Aryan: pushed context registes to OLD stack (havent switched yet) -- this saves the current state of old process

  # Switch stacks
  movl %esp, (%eax)
  movl %edx, %esp // now esp = new stack

  # Load new callee-saved registers
  popl %edi
  popl %esi
  popl %ebx
  popl %ebp
  ret

  /* Aryan: since esp is now new, values popped into the register are from the NEW location
   * Final ret instruction -- pops EIP value that is automatically pushed into hardware, so effectively JUMPS to new
   * */
