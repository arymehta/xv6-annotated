3800 #include "types.h"
3801 #include "x86.h"
3802 #include "defs.h"
3803 #include "date.h"
3804 #include "param.h"
3805 #include "memlayout.h"
3806 #include "mmu.h"
3807 #include "proc.h"
3808 
3809 int
3810 sys_fork(void)
3811 {
3812   return fork();
3813 }
3814 
3815 int
3816 sys_exit(void)
3817 {
3818   exit();
3819   return 0;  // not reached
3820 }
3821 
3822 int
3823 sys_wait(void)
3824 {
3825   return wait();
3826 }
3827 
3828 int
3829 sys_kill(void)
3830 {
3831   int pid;
3832 
3833   if(argint(0, &pid) < 0)
3834     return -1;
3835   return kill(pid);
3836 }
3837 
3838 int
3839 sys_getpid(void)
3840 {
3841   return myproc()->pid;
3842 }
3843 
3844 
3845 
3846 
3847 
3848 
3849 
3850 int
3851 sys_sbrk(void)
3852 {
3853   int addr;
3854   int n;
3855 
3856   if(argint(0, &n) < 0)
3857     return -1;
3858   addr = myproc()->sz;
3859   if(growproc(n) < 0)
3860     return -1;
3861   return addr;
3862 }
3863 
3864 int
3865 sys_sleep(void)
3866 {
3867   int n;
3868   uint ticks0;
3869 
3870   if(argint(0, &n) < 0)
3871     return -1;
3872   acquire(&tickslock);
3873   ticks0 = ticks;
3874   while(ticks - ticks0 < n){
3875     if(myproc()->killed){
3876       release(&tickslock);
3877       return -1;
3878     }
3879     sleep(&ticks, &tickslock);
3880   }
3881   release(&tickslock);
3882   return 0;
3883 }
3884 
3885 // return how many clock tick interrupts have occurred
3886 // since start.
3887 int
3888 sys_uptime(void)
3889 {
3890   uint xticks;
3891 
3892   acquire(&tickslock);
3893   xticks = ticks;
3894   release(&tickslock);
3895   return xticks;
3896 }
3897 
3898 
3899 
