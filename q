memlayout.h <global> 4 #define PHYSTOP 0xE000000
vm.c <global> 124 { (void *)data, V2P(data), PHYSTOP, PTE_W},
kalloc.c kfree 64 if((uint)v % PGSIZE || v < end || V2P(v) >= PHYSTOP)
main.c main 35 kinit2(P2V(4*1024*1024), P2V(PHYSTOP));
vm.c setupkvm 143 if (P2V(PHYSTOP) > (void *)DEVSPACE)
