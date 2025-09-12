3100 # Context switch
3101 #
3102 #   void swtch(struct context **old, struct context *new);
3103 #
3104 # Save the current registers on the stack, creating
3105 # a struct context, and save its address in *old.
3106 # Switch stacks to new and pop previously-saved registers.
3107 
3108 
3109 /* Aryan: swtch() was called as a C FUNCTION in scheduler() and sched()
3110  * This mean the compiler compiled it using calling convention
3111  * calling convention -- arguments pushed to stack in reverse order!
3112  * Thats why we get args by accessing esp!
3113  *
3114  * */
3115 .globl swtch
3116 swtch:
3117   movl 4(%esp), %eax // old
3118   movl 8(%esp), %edx // new
3119 
3120   # Save old callee-saved registers
3121   pushl %ebp
3122   pushl %ebx
3123   pushl %esi
3124   pushl %edi
3125   // Aryan: pushed context registes to OLD stack (havent switched yet) -- this saves the current state of old process
3126 
3127   # Switch stacks
3128   movl %esp, (%eax)
3129   movl %edx, %esp // now esp = new stack
3130 
3131   # Load new callee-saved registers
3132   popl %edi
3133   popl %esi
3134   popl %ebx
3135   popl %ebp
3136   ret
3137 
3138   /* Aryan: since esp is now new, values popped into the register are from the NEW location
3139    * Final ret instruction -- pops EIP value that is automatically pushed into hardware, so effectively JUMPS to new
3140    * */
3141 
3142 
3143 
3144 
3145 
3146 
3147 
3148 
3149 
