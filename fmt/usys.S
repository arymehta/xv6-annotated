8550 #include "syscall.h"
8551 #include "traps.h"
8552 
8553 #define SYSCALL(name) \
8554   .globl name; \
8555   name: \
8556     movl $SYS_ ## name, %eax; \
8557     int $T_SYSCALL; \
8558     ret
8559 
8560 SYSCALL(fork)
8561 SYSCALL(exit)
8562 SYSCALL(wait)
8563 SYSCALL(pipe)
8564 SYSCALL(read)
8565 SYSCALL(write)
8566 SYSCALL(close)
8567 SYSCALL(kill)
8568 SYSCALL(exec)
8569 SYSCALL(open)
8570 SYSCALL(mknod)
8571 SYSCALL(unlink)
8572 SYSCALL(fstat)
8573 SYSCALL(link)
8574 SYSCALL(mkdir)
8575 SYSCALL(chdir)
8576 SYSCALL(dup)
8577 SYSCALL(getpid)
8578 SYSCALL(sbrk)
8579 SYSCALL(sleep)
8580 SYSCALL(uptime)
8581 SYSCALL(hello)
8582 SYSCALL(lseek)
8583 
8584 
8585 
8586 
8587 
8588 
8589 
8590 
8591 
8592 
8593 
8594 
8595 
8596 
8597 
8598 
8599 
