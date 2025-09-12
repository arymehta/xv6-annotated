3250 // x86 trap and interrupt constants.
3251 
3252 // Processor-defined:
3253 #define T_DIVIDE         0      // divide error
3254 #define T_DEBUG          1      // debug exception
3255 #define T_NMI            2      // non-maskable interrupt
3256 #define T_BRKPT          3      // breakpoint
3257 #define T_OFLOW          4      // overflow
3258 #define T_BOUND          5      // bounds check
3259 #define T_ILLOP          6      // illegal opcode
3260 #define T_DEVICE         7      // device not available
3261 #define T_DBLFLT         8      // double fault
3262 // #define T_COPROC      9      // reserved (not used since 486)
3263 #define T_TSS           10      // invalid task switch segment
3264 #define T_SEGNP         11      // segment not present
3265 #define T_STACK         12      // stack exception
3266 #define T_GPFLT         13      // general protection fault
3267 #define T_PGFLT         14      // page fault
3268 // #define T_RES        15      // reserved
3269 #define T_FPERR         16      // floating point error
3270 #define T_ALIGN         17      // aligment check
3271 #define T_MCHK          18      // machine check
3272 #define T_SIMDERR       19      // SIMD floating point error
3273 
3274 // These are arbitrarily chosen, but with care not to overlap
3275 // processor defined exceptions or interrupt vectors.
3276 #define T_SYSCALL       64      // system call
3277 #define T_DEFAULT      500      // catchall
3278 
3279 #define T_IRQ0          32      // IRQ 0 corresponds to int T_IRQ
3280 
3281 #define IRQ_TIMER        0
3282 #define IRQ_KBD          1
3283 #define IRQ_COM1         4
3284 #define IRQ_IDE         14
3285 #define IRQ_ERROR       19
3286 #define IRQ_SPURIOUS    31
3287 
3288 
3289 
3290 
3291 
3292 
3293 
3294 
3295 
3296 
3297 
3298 
3299 
