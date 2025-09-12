3600 #include "types.h"
3601 #include "defs.h"
3602 #include "param.h"
3603 #include "memlayout.h"
3604 #include "mmu.h"
3605 #include "proc.h"
3606 #include "x86.h"
3607 #include "syscall.h"
3608 
3609 // User code makes a system call with INT T_SYSCALL.
3610 // System call number in %eax.
3611 // Arguments on the stack, from the user call to the C
3612 // library system call function. The saved user %esp points
3613 // to a saved program counter, and then the first argument.
3614 
3615 // Fetch the int at addr from the current process.
3616 int
3617 fetchint(uint addr, int *ip)
3618 {
3619   struct proc *curproc = myproc();
3620 
3621   if(addr >= curproc->sz || addr+4 > curproc->sz)
3622     return -1;
3623   *ip = *(int*)(addr);
3624   return 0;
3625 }
3626 
3627 // Fetch the nul-terminated string at addr from the current process.
3628 // Doesn't actually copy the string - just sets *pp to point at it.
3629 // Returns length of string, not including nul.
3630 int
3631 fetchstr(uint addr, char **pp)
3632 {
3633   char *s, *ep;
3634   struct proc *curproc = myproc();
3635 
3636   if(addr >= curproc->sz)
3637     return -1;
3638   *pp = (char*)addr;
3639   ep = (char*)curproc->sz;
3640   for(s = *pp; s < ep; s++){
3641     if(*s == 0)
3642       return s - *pp;
3643   }
3644   return -1;
3645 }
3646 
3647 
3648 
3649 
3650 // Fetch the nth 32-bit system call argument.
3651 int
3652 argint(int n, int *ip)
3653 {
3654   return fetchint((myproc()->tf->esp) + 4 + 4*n, ip);
3655 }
3656 
3657 // Fetch the nth word-sized system call argument as a pointer
3658 // to a block of memory of size bytes.  Check that the pointer
3659 // lies within the process address space.
3660 int
3661 argptr(int n, char **pp, int size)
3662 {
3663   int i;
3664   struct proc *curproc = myproc();
3665 
3666   if(argint(n, &i) < 0)
3667     return -1;
3668   if(size < 0 || (uint)i >= curproc->sz || (uint)i+size > curproc->sz)
3669     return -1;
3670   *pp = (char*)i;
3671   return 0;
3672 }
3673 
3674 // Fetch the nth word-sized system call argument as a string pointer.
3675 // Check that the pointer is valid and the string is nul-terminated.
3676 // (There is no shared writable memory, so the string can't change
3677 // between this check and being used by the kernel.)
3678 int
3679 argstr(int n, char **pp)
3680 {
3681   int addr;
3682   if(argint(n, &addr) < 0)
3683     return -1;
3684   return fetchstr(addr, pp);
3685 }
3686 
3687 
3688 
3689 
3690 
3691 
3692 
3693 
3694 
3695 
3696 
3697 
3698 
3699 
3700 extern int sys_chdir(void);
3701 extern int sys_close(void);
3702 extern int sys_dup(void);
3703 extern int sys_exec(void);
3704 extern int sys_exit(void);
3705 extern int sys_fork(void);
3706 extern int sys_fstat(void);
3707 extern int sys_getpid(void);
3708 extern int sys_kill(void);
3709 extern int sys_link(void);
3710 extern int sys_mkdir(void);
3711 extern int sys_mknod(void);
3712 extern int sys_open(void);
3713 extern int sys_pipe(void);
3714 extern int sys_read(void);
3715 extern int sys_sbrk(void);
3716 extern int sys_sleep(void);
3717 extern int sys_unlink(void);
3718 extern int sys_wait(void);
3719 extern int sys_write(void);
3720 extern int sys_uptime(void);
3721 extern int sys_hello(void);
3722 extern int sys_lseek(void);
3723 
3724 static int (*syscalls[])(void) = {
3725 [SYS_fork]    sys_fork,
3726 [SYS_exit]    sys_exit,
3727 [SYS_wait]    sys_wait,
3728 [SYS_pipe]    sys_pipe,
3729 [SYS_read]    sys_read,
3730 [SYS_kill]    sys_kill,
3731 [SYS_exec]    sys_exec,
3732 [SYS_fstat]   sys_fstat,
3733 [SYS_chdir]   sys_chdir,
3734 [SYS_dup]     sys_dup,
3735 [SYS_getpid]  sys_getpid,
3736 [SYS_sbrk]    sys_sbrk,
3737 [SYS_sleep]   sys_sleep,
3738 [SYS_uptime]  sys_uptime,
3739 [SYS_open]    sys_open,
3740 [SYS_write]   sys_write,
3741 [SYS_mknod]   sys_mknod,
3742 [SYS_unlink]  sys_unlink,
3743 [SYS_link]    sys_link,
3744 [SYS_mkdir]   sys_mkdir,
3745 [SYS_close]   sys_close,
3746 [SYS_hello]   sys_hello,
3747 [SYS_lseek]   sys_lseek
3748 };
3749 
3750 void
3751 syscall(void)
3752 {
3753   int num;
3754   struct proc *curproc = myproc();
3755 
3756   num = curproc->tf->eax;
3757   if(num > 0 && num < NELEM(syscalls) && syscalls[num]) {
3758     curproc->tf->eax = syscalls[num]();
3759   } else {
3760     cprintf("%d %s: unknown sys call %d\n",
3761             curproc->pid, curproc->name, num);
3762     curproc->tf->eax = -1;
3763   }
3764 }
3765 
3766 
3767 
3768 
3769 
3770 
3771 
3772 
3773 
3774 
3775 
3776 
3777 
3778 
3779 
3780 
3781 
3782 
3783 
3784 
3785 
3786 
3787 
3788 
3789 
3790 
3791 
3792 
3793 
3794 
3795 
3796 
3797 
3798 
3799 
