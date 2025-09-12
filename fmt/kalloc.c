3150 // Physical memory allocator, intended to allocate
3151 // memory for user processes, kernel stacks, page table pages,
3152 // and pipe buffers. Allocates 4096-byte pages.
3153 
3154 #include "types.h"
3155 #include "defs.h"
3156 #include "param.h"
3157 #include "memlayout.h"
3158 #include "mmu.h"
3159 #include "spinlock.h"
3160 
3161 void freerange(void *vstart, void *vend);
3162 extern char end[]; // first address after kernel loaded from ELF file
3163                    // defined by the kernel linker script in kernel.ld
3164 
3165 struct run {
3166   struct run *next;
3167 };
3168 
3169 struct {
3170   struct spinlock lock;
3171   int use_lock;
3172   struct run *freelist;
3173 } kmem;
3174 
3175 // Initialization happens in two phases.
3176 // 1. main() calls kinit1() while still using entrypgdir to place just
3177 // the pages mapped by entrypgdir on free list.
3178 // 2. main() calls kinit2() with the rest of the physical pages
3179 // after installing a full page table that maps them on all cores.
3180 void
3181 kinit1(void *vstart, void *vend)
3182 {
3183   initlock(&kmem.lock, "kmem");
3184   kmem.use_lock = 0;
3185   freerange(vstart, vend);
3186 }
3187 
3188 void
3189 kinit2(void *vstart, void *vend)
3190 {
3191   freerange(vstart, vend);
3192   kmem.use_lock = 1;
3193 }
3194 
3195 
3196 
3197 
3198 
3199 
3200 void
3201 freerange(void *vstart, void *vend)
3202 {
3203   char *p;
3204   p = (char*)PGROUNDUP((uint)vstart);
3205   for(; p + PGSIZE <= (char*)vend; p += PGSIZE)
3206     kfree(p);
3207 }
3208 
3209 // Free the page of physical memory pointed at by v,
3210 // which normally should have been returned by a
3211 // call to kalloc().  (The exception is when
3212 // initializing the allocator; see kinit above.)
3213 void
3214 kfree(char *v)
3215 {
3216   struct run *r;
3217 
3218   if((uint)v % PGSIZE || v < end || V2P(v) >= PHYSTOP)
3219     panic("kfree");
3220 
3221   // Fill with junk to catch dangling refs.
3222   memset(v, 1, PGSIZE);
3223 
3224   if(kmem.use_lock)
3225     acquire(&kmem.lock);
3226   r = (struct run*)v;
3227   r->next = kmem.freelist;
3228   kmem.freelist = r;
3229   if(kmem.use_lock)
3230     release(&kmem.lock);
3231 }
3232 
3233 // Allocate one 4096-byte page of physical memory.
3234 // Returns a pointer that the kernel can use.
3235 // Returns 0 if the memory cannot be allocated.
3236 char*
3237 kalloc(void)
3238 {
3239   struct run *r;
3240 
3241   if(kmem.use_lock)
3242     acquire(&kmem.lock);
3243   r = kmem.freelist;
3244   if(r)
3245     kmem.freelist = r->next;
3246   if(kmem.use_lock)
3247     release(&kmem.lock);
3248   return (char*)r;
3249 }
