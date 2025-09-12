8500 # Initial process execs /init.
8501 # This code runs in user space.
8502 
8503 #include "syscall.h"
8504 #include "traps.h"
8505 
8506 
8507 # exec(init, argv)
8508 .globl start
8509 start:
8510   pushl $argv
8511   pushl $init
8512   pushl $0  // where caller pc would be
8513   movl $SYS_exec, %eax
8514   int $T_SYSCALL
8515 
8516 # for(;;) exit();
8517 exit:
8518   movl $SYS_exit, %eax
8519   int $T_SYSCALL
8520   jmp exit
8521 
8522 # char init[] = "/init\0";
8523 init:
8524   .string "/init\0"
8525 
8526 # char *argv[] = { init, 0 };
8527 .p2align 2
8528 argv:
8529   .long init
8530   .long 0
8531 
8532 
8533 
8534 
8535 
8536 
8537 
8538 
8539 
8540 
8541 
8542 
8543 
8544 
8545 
8546 
8547 
8548 
8549 
