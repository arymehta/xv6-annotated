2400 #include "types.h"
2401 #include "defs.h"
2402 #include "param.h"
2403 #include "memlayout.h"
2404 #include "mmu.h"
2405 #include "x86.h"
2406 #include "proc.h"
2407 #include "spinlock.h"
2408 
2409 struct {
2410   struct spinlock lock;
2411   struct proc proc[NPROC];
2412 } ptable;
2413 
2414 static struct proc *initproc;
2415 
2416 int nextpid = 1;
2417 extern void forkret(void);
2418 extern void trapret(void);
2419 
2420 static void wakeup1(void *chan);
2421 
2422 void
2423 pinit(void)
2424 {
2425   initlock(&ptable.lock, "ptable");
2426 }
2427 
2428 // Must be called with interrupts disabled
2429 int
2430 cpuid() {
2431   return mycpu()-cpus;
2432 }
2433 
2434 // Must be called with interrupts disabled to avoid the caller being
2435 // rescheduled between reading lapicid and running through the loop.
2436 struct cpu*
2437 mycpu(void)
2438 {
2439   int apicid, i;
2440 
2441   if(readeflags()&FL_IF)
2442     panic("mycpu called with interrupts enabled\n");
2443 
2444   apicid = lapicid();
2445   // APIC IDs are not guaranteed to be contiguous. Maybe we should have
2446   // a reverse map, or reserve a register to store &cpus[i].
2447   for (i = 0; i < ncpu; ++i) {
2448     if (cpus[i].apicid == apicid)
2449       return &cpus[i];
2450   }
2451   panic("unknown apicid\n");
2452 }
2453 
2454 // Disable interrupts so that we are not rescheduled
2455 // while reading proc from the cpu structure
2456 struct proc*
2457 myproc(void) {
2458   struct cpu *c;
2459   struct proc *p;
2460   pushcli();
2461   c = mycpu();
2462   p = c->proc;
2463   popcli();
2464   return p;
2465 }
2466 
2467 
2468 // Look in the process table for an UNUSED proc.
2469 // If found, change state to EMBRYO and initialize
2470 // state required to run in the kernel.
2471 // Otherwise return 0.
2472 static struct proc*
2473 allocproc(void)
2474 {
2475   struct proc *p;
2476   char *sp;
2477 
2478   acquire(&ptable.lock);
2479 
2480   for(p = ptable.proc; p < &ptable.proc[NPROC]; p++)
2481     if(p->state == UNUSED)
2482       goto found;
2483 
2484   release(&ptable.lock);
2485   return 0;
2486 
2487 found:
2488   p->state = EMBRYO;
2489   p->pid = nextpid++;
2490 
2491   release(&ptable.lock);
2492 
2493   // Allocate kernel stack.
2494   if((p->kstack = kalloc()) == 0){
2495     p->state = UNUSED;
2496     return 0;
2497   }
2498   sp = p->kstack + KSTACKSIZE;
2499 
2500   // Leave room for trap frame.
2501   sp -= sizeof *p->tf;
2502   p->tf = (struct trapframe*)sp;
2503 
2504   // Set up new context to start executing at forkret,
2505   // which returns to trapret.
2506   sp -= 4;
2507   *(uint*)sp = (uint)trapret;
2508 
2509   sp -= sizeof *p->context;
2510   p->context = (struct context*)sp;
2511   memset(p->context, 0, sizeof *p->context);
2512   p->context->eip = (uint)forkret;
2513 
2514   return p;
2515 }
2516 
2517 
2518 // Set up first user process.
2519 void
2520 userinit(void)
2521 {
2522   struct proc *p;
2523   extern char _binary_initcode_start[], _binary_initcode_size[];
2524 
2525   p = allocproc();
2526 
2527   initproc = p;
2528   if((p->pgdir = setupkvm()) == 0)
2529     panic("userinit: out of memory?");
2530   inituvm(p->pgdir, _binary_initcode_start, (int)_binary_initcode_size);
2531   p->sz = PGSIZE;
2532   memset(p->tf, 0, sizeof(*p->tf));
2533   p->tf->cs = (SEG_UCODE << 3) | DPL_USER;
2534   p->tf->ds = (SEG_UDATA << 3) | DPL_USER;
2535   p->tf->es = p->tf->ds;
2536   p->tf->ss = p->tf->ds;
2537   p->tf->eflags = FL_IF;
2538   p->tf->esp = PGSIZE;
2539   p->tf->eip = 0;  // beginning of initcode.S
2540 
2541   safestrcpy(p->name, "initcode", sizeof(p->name));
2542   p->cwd = namei("/");
2543 
2544   // this assignment to p->state lets other cores
2545   // run this process. the acquire forces the above
2546   // writes to be visible, and the lock is also needed
2547   // because the assignment might not be atomic.
2548   acquire(&ptable.lock);
2549 
2550   p->state = RUNNABLE;
2551 
2552   release(&ptable.lock);
2553 }
2554 
2555 // Grow current process's memory by n bytes.
2556 // Return 0 on success, -1 on failure.
2557 int
2558 growproc(int n)
2559 {
2560   uint sz;
2561   struct proc *curproc = myproc();
2562 
2563   sz = curproc->sz;
2564   if(n > 0){
2565     if((sz = allocuvm(curproc->pgdir, sz, sz + n)) == 0)
2566       return -1;
2567   } else if(n < 0){
2568     if((sz = deallocuvm(curproc->pgdir, sz, sz + n)) == 0)
2569       return -1;
2570   }
2571   curproc->sz = sz;
2572   switchuvm(curproc);
2573   return 0;
2574 }
2575 
2576 // Create a new process copying p as the parent.
2577 // Sets up stack to return as if from system call.
2578 // Caller must set state of returned proc to RUNNABLE.
2579 int
2580 fork(void)
2581 {
2582   int i, pid;
2583   struct proc *np;
2584   struct proc *curproc = myproc();
2585 
2586   // Allocate process.
2587   if((np = allocproc()) == 0){
2588     return -1;
2589   }
2590 
2591   // Copy process state from proc.
2592   if((np->pgdir = copyuvm(curproc->pgdir, curproc->sz)) == 0){
2593     kfree(np->kstack);
2594     np->kstack = 0;
2595     np->state = UNUSED;
2596     return -1;
2597   }
2598   np->sz = curproc->sz;
2599   np->parent = curproc;
2600   *np->tf = *curproc->tf;
2601 
2602   // Clear %eax so that fork returns 0 in the child.
2603   np->tf->eax = 0;
2604 
2605   for(i = 0; i < NOFILE; i++)
2606     if(curproc->ofile[i])
2607       np->ofile[i] = filedup(curproc->ofile[i]);
2608   np->cwd = idup(curproc->cwd);
2609 
2610   safestrcpy(np->name, curproc->name, sizeof(curproc->name));
2611 
2612   pid = np->pid;
2613 
2614   acquire(&ptable.lock);
2615 
2616   np->state = RUNNABLE;
2617 
2618   release(&ptable.lock);
2619 
2620   return pid;
2621 }
2622 
2623 // Exit the current process.  Does not return.
2624 // An exited process remains in the zombie state
2625 // until its parent calls wait() to find out it exited.
2626 void
2627 exit(void)
2628 {
2629   struct proc *curproc = myproc();
2630   struct proc *p;
2631   int fd;
2632 
2633   if(curproc == initproc)
2634     panic("init exiting");
2635 
2636   // Close all open files.
2637   for(fd = 0; fd < NOFILE; fd++){
2638     if(curproc->ofile[fd]){
2639       fileclose(curproc->ofile[fd]);
2640       curproc->ofile[fd] = 0;
2641     }
2642   }
2643 
2644   begin_op();
2645   iput(curproc->cwd);
2646   end_op();
2647   curproc->cwd = 0;
2648 
2649   acquire(&ptable.lock);
2650   // Parent might be sleeping in wait().
2651   wakeup1(curproc->parent);
2652 
2653   // Pass abandoned children to init.
2654   for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
2655     if(p->parent == curproc){
2656       p->parent = initproc;
2657       if(p->state == ZOMBIE)
2658         wakeup1(initproc);
2659     }
2660   }
2661 
2662   // Jump into the scheduler, never to return.
2663   curproc->state = ZOMBIE;
2664   sched();
2665   panic("zombie exit");
2666 }
2667 
2668 // Wait for a child process to exit and return its pid.
2669 // Return -1 if this process has no children.
2670 int
2671 wait(void)
2672 {
2673   struct proc *p;
2674   int havekids, pid;
2675   struct proc *curproc = myproc();
2676 
2677   acquire(&ptable.lock);
2678   for(;;){
2679     // Scan through table looking for exited children.
2680     havekids = 0;
2681     for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
2682       if(p->parent != curproc)
2683         continue;
2684       havekids = 1;
2685       if(p->state == ZOMBIE){
2686         // Found one.
2687         pid = p->pid;
2688         kfree(p->kstack);
2689         p->kstack = 0;
2690         freevm(p->pgdir);
2691         p->pid = 0;
2692         p->parent = 0;
2693         p->name[0] = 0;
2694         p->killed = 0;
2695         p->state = UNUSED;
2696         release(&ptable.lock);
2697         return pid;
2698       }
2699     }
2700     // No point waiting if we don't have any children.
2701     if(!havekids || curproc->killed){
2702       release(&ptable.lock);
2703       return -1;
2704     }
2705 
2706     // Wait for children to exit.  (See wakeup1 call in proc_exit.)
2707     sleep(curproc, &ptable.lock);  //DOC: wait-sleep
2708   }
2709 }
2710 
2711 
2712 
2713 
2714 
2715 
2716 
2717 
2718 
2719 
2720 
2721 
2722 
2723 
2724 
2725 
2726 
2727 
2728 
2729 
2730 
2731 
2732 
2733 
2734 
2735 
2736 
2737 
2738 
2739 
2740 
2741 
2742 
2743 
2744 
2745 
2746 
2747 
2748 
2749 
2750 // Per-CPU process scheduler.
2751 // Each CPU calls scheduler() after setting itself up.
2752 // Scheduler never returns.  It loops, doing:
2753 //  - choose a process to run
2754 //  - swtch to start running that process
2755 //  - eventually that process transfers control
2756 //      via swtch back to the scheduler.
2757 
2758 /*
2759  * ARYAN:
2760  *      - switches to P2 from scheduler
2761  *      - steps involved:
2762  *          -> setting up TSS (so that cpu knows where kstack of new process lies)
2763  *          -> saving current scheduler state
2764  *          -> switch to stack of new process
2765  *          -> return into the kstack of that process
2766  *
2767  *          NOTE: context switch:
2768  *          - scheduler is called only once *directly* to schedule the INIT process
2769  *          - otherwise its always callled through sched()
2770  *          - sched() -- switches from P1 to scheduler code
2771  *          - scheduler() -- picks P2, and switches to that
2772  *          - both functions use swtch.S
2773  *
2774  *
2775  *          STEP BY STEP:
2776  *          1) P1 is running
2777  *          2) timer interrupt
2778  *          3) IDT -> change mode from user to kernel -> change ss, esp P1 kstack(from TSS) -> vectors.S ->
2779  *          alltraps(sets up trapframe) -> trap()
2780  *          4) trap() -> TIMERINTR -> yeild()
2781  *          5) yeild() -> swtch(&(p1->context), cpu->sheduler) == switches executing stack from P1 kstack to scheduler
2782  *              -> scheduler()
2783  *          6) scheduler() selects a process using round robin -> sets up TSS for P2 (switchuvm())
2784  *              -> swtch(&(cpu->scheduler), p2->context) == switches to kstack of P2
2785  *          7) swtch.
2786  *          ret --> pops the EIP --> which was automatically saved by the hardware, FROM P2 kstack
2787  *          --> means jumps directly into P2 kstack
2788  *
2789  *          8) So in this process, in swtch(), P1's context got appropriately saved in P1 kstack, and succesfully jumped to P2
2790  *
2791  *
2792 */
2793 void
2794 scheduler(void)
2795 {
2796   struct proc *p;
2797   struct cpu *c = mycpu();
2798   c->proc = 0;
2799 
2800   for(;;){
2801     // Enable interrupts on this processor.
2802     sti();
2803 
2804     // Loop over process table looking for process to run.
2805     acquire(&ptable.lock);
2806     for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
2807       if(p->state != RUNNABLE)
2808         continue;
2809 
2810       // Switch to chosen process.  It is the process's job
2811       // to release ptable.lock and then reacquire it
2812       // before jumping back to us.
2813 
2814 
2815       c->proc = p;
2816       switchuvm(p); // ARYAN: setting up TSS of current CPU to kstack of P2
2817       p->state = RUNNING;
2818 
2819       swtch(&(c->scheduler), p->context); // ARYAN: switches from scheduler to kstack of P2
2820       // ** dosent return here normally
2821 
2822 
2823       switchkvm();
2824     // ARYAN: only executes if there was no process scheduled -- sets the pgdir to KERNEL only pgdir
2825     // (the contents of this page are copied across page tables of all the user pgdirs, but
2826     // kpgdir dosent contain any user process pgdata)
2827 
2828       // Process is done running for now.
2829       // It should have changed its p->state before coming back.
2830       c->proc = 0;
2831     }
2832     release(&ptable.lock);
2833 
2834   }
2835 }
2836 
2837 
2838 
2839 
2840 
2841 
2842 
2843 
2844 
2845 
2846 
2847 
2848 
2849 
2850 /* ARYAN
2851  * Q) Why is TSS even needed? Dont we already have proc->kstack to tell us where kstack is?
2852  * Ans) User mode process -> interrupt -> neeeds kstack immediately to push and create trapframe
2853  * -> BUT struct proc is in kernel space, cannot access from user mode
2854  * -> this is why the value is stored before hand while P1 was BEING scheduled(while it was still in kernel mode).
2855  * */
2856 
2857 
2858 // Enter scheduler.  Must hold only ptable.lock
2859 // and have changed proc->state. Saves and restores
2860 // intena because intena is a property of this
2861 // kernel thread, not this CPU. It should
2862 // be proc->intena and proc->ncli, but that would
2863 // break in the few places where a lock is held but
2864 // there's no process.
2865 void
2866 sched(void)
2867 {
2868   int intena;
2869   struct proc *p = myproc();
2870 
2871   if(!holding(&ptable.lock))
2872     panic("sched ptable.lock");
2873   if(mycpu()->ncli != 1)
2874     panic("sched locks");
2875   if(p->state == RUNNING)
2876     panic("sched running");
2877   if(readeflags()&FL_IF)
2878     panic("sched interruptible");
2879   intena = mycpu()->intena;
2880     // Aryan: switches from P1 kstack to scheduler (no need to setup TSS because it was already setup when P1 got scheduled)
2881   swtch(&p->context, mycpu()->scheduler);
2882   mycpu()->intena = intena;
2883 }
2884 
2885 // Give up the CPU for one scheduling round.
2886 void
2887 yield(void)
2888 {
2889   acquire(&ptable.lock);  //DOC: yieldlock
2890   myproc()->state = RUNNABLE;
2891   sched();
2892   release(&ptable.lock);
2893 }
2894 
2895 
2896 
2897 
2898 
2899 
2900 // A fork child's very first scheduling by scheduler()
2901 // will swtch here.  "Return" to user space.
2902 void
2903 forkret(void)
2904 {
2905   static int first = 1;
2906   // Still holding ptable.lock from scheduler.
2907   release(&ptable.lock);
2908 
2909   if (first) {
2910     // Some initialization functions must be run in the context
2911     // of a regular process (e.g., they call sleep), and thus cannot
2912     // be run from main().
2913     first = 0;
2914     iinit(ROOTDEV);
2915     initlog(ROOTDEV);
2916   }
2917 
2918   // Return to "caller", actually trapret (see allocproc).
2919 }
2920 
2921 // Atomically release lock and sleep on chan.
2922 // Reacquires lock when awakened.
2923 void
2924 sleep(void *chan, struct spinlock *lk)
2925 {
2926   struct proc *p = myproc();
2927 
2928   if(p == 0)
2929     panic("sleep");
2930 
2931   if(lk == 0)
2932     panic("sleep without lk");
2933 
2934   // Must acquire ptable.lock in order to
2935   // change p->state and then call sched.
2936   // Once we hold ptable.lock, we can be
2937   // guaranteed that we won't miss any wakeup
2938   // (wakeup runs with ptable.lock locked),
2939   // so it's okay to release lk.
2940   if(lk != &ptable.lock){  //DOC: sleeplock0
2941     acquire(&ptable.lock);  //DOC: sleeplock1
2942     release(lk);
2943   }
2944   // Go to sleep.
2945   p->chan = chan;
2946   p->state = SLEEPING;
2947 
2948   sched();
2949 
2950   // Tidy up.
2951   p->chan = 0;
2952 
2953   // Reacquire original lock.
2954   if(lk != &ptable.lock){  //DOC: sleeplock2
2955     release(&ptable.lock);
2956     acquire(lk);
2957   }
2958 }
2959 
2960 
2961 
2962 
2963 
2964 
2965 
2966 
2967 
2968 
2969 
2970 
2971 
2972 
2973 
2974 
2975 
2976 
2977 
2978 
2979 
2980 
2981 
2982 
2983 
2984 
2985 
2986 
2987 
2988 
2989 
2990 
2991 
2992 
2993 
2994 
2995 
2996 
2997 
2998 
2999 
3000 // Wake up all processes sleeping on chan.
3001 // The ptable lock must be held.
3002 static void
3003 wakeup1(void *chan)
3004 {
3005   struct proc *p;
3006 
3007   for(p = ptable.proc; p < &ptable.proc[NPROC]; p++)
3008     if(p->state == SLEEPING && p->chan == chan)
3009       p->state = RUNNABLE;
3010 }
3011 
3012 // Wake up all processes sleeping on chan.
3013 void
3014 wakeup(void *chan)
3015 {
3016   acquire(&ptable.lock);
3017   wakeup1(chan);
3018   release(&ptable.lock);
3019 }
3020 
3021 // Kill the process with the given pid.
3022 // Process won't exit until it returns
3023 // to user space (see trap in trap.c).
3024 int
3025 kill(int pid)
3026 {
3027   struct proc *p;
3028 
3029   acquire(&ptable.lock);
3030   for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
3031     if(p->pid == pid){
3032       p->killed = 1;
3033       // Wake process from sleep if necessary.
3034       if(p->state == SLEEPING)
3035         p->state = RUNNABLE;
3036       release(&ptable.lock);
3037       return 0;
3038     }
3039   }
3040   release(&ptable.lock);
3041   return -1;
3042 }
3043 
3044 
3045 
3046 
3047 
3048 
3049 
3050 // Print a process listing to console.  For debugging.
3051 // Runs when user types ^P on console.
3052 // No lock to avoid wedging a stuck machine further.
3053 void
3054 procdump(void)
3055 {
3056   static char *states[] = {
3057   [UNUSED]    "unused",
3058   [EMBRYO]    "embryo",
3059   [SLEEPING]  "sleep ",
3060   [RUNNABLE]  "runble",
3061   [RUNNING]   "run   ",
3062   [ZOMBIE]    "zombie"
3063   };
3064   int i;
3065   struct proc *p;
3066   char *state;
3067   uint pc[10];
3068 
3069   for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
3070     if(p->state == UNUSED)
3071       continue;
3072     if(p->state >= 0 && p->state < NELEM(states) && states[p->state])
3073       state = states[p->state];
3074     else
3075       state = "???";
3076     cprintf("%d %s %s", p->pid, state, p->name);
3077     if(p->state == SLEEPING){
3078       getcallerpcs((uint*)p->context->ebp+2, pc);
3079       for(i=0; i<10 && pc[i] != 0; i++)
3080         cprintf(" %p", pc[i]);
3081     }
3082     cprintf("\n");
3083   }
3084 }
3085 
3086 
3087 
3088 
3089 
3090 
3091 
3092 
3093 
3094 
3095 
3096 
3097 
3098 
3099 
