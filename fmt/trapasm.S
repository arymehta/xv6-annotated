3350 #include "mmu.h"
3351 
3352   # vectors.S sends all traps here.
3353 .globl alltraps
3354 alltraps:
3355   # Build trap frame.
3356   pushl %ds
3357   pushl %es
3358   pushl %fs
3359   pushl %gs
3360   pushal
3361 
3362   # Set up data segments.
3363   movw $(SEG_KDATA<<3), %ax
3364   movw %ax, %ds
3365   movw %ax, %es
3366 
3367   # Call trap(tf), where tf=%esp
3368   pushl %esp # User stack esp pushed
3369   call trap
3370   addl $4, %esp
3371 
3372   # Return falls through to trapret...
3373 .globl trapret
3374 trapret:
3375   popal
3376   popl %gs
3377   popl %fs
3378   popl %es
3379   popl %ds
3380   addl $0x8, %esp  # trapno and errcode
3381   iret
3382 
3383 
3384 
3385 
3386 
3387 
3388 
3389 
3390 
3391 
3392 
3393 
3394 
3395 
3396 
3397 
3398 
3399 
