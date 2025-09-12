7300 // Multiprocessor support
7301 // Search memory for MP description structures.
7302 // http://developer.intel.com/design/pentium/datashts/24201606.pdf
7303 
7304 #include "types.h"
7305 #include "defs.h"
7306 #include "param.h"
7307 #include "memlayout.h"
7308 #include "mp.h"
7309 #include "x86.h"
7310 #include "mmu.h"
7311 #include "proc.h"
7312 
7313 struct cpu cpus[NCPU];
7314 int ncpu;
7315 uchar ioapicid;
7316 
7317 static uchar
7318 sum(uchar *addr, int len)
7319 {
7320   int i, sum;
7321 
7322   sum = 0;
7323   for(i=0; i<len; i++)
7324     sum += addr[i];
7325   return sum;
7326 }
7327 
7328 // Look for an MP structure in the len bytes at addr.
7329 static struct mp*
7330 mpsearch1(uint a, int len)
7331 {
7332   uchar *e, *p, *addr;
7333 
7334   addr = P2V(a);
7335   e = addr+len;
7336   for(p = addr; p < e; p += sizeof(struct mp))
7337     if(memcmp(p, "_MP_", 4) == 0 && sum(p, sizeof(struct mp)) == 0)
7338       return (struct mp*)p;
7339   return 0;
7340 }
7341 
7342 
7343 
7344 
7345 
7346 
7347 
7348 
7349 
7350 // Search for the MP Floating Pointer Structure, which according to the
7351 // spec is in one of the following three locations:
7352 // 1) in the first KB of the EBDA;
7353 // 2) in the last KB of system base memory;
7354 // 3) in the BIOS ROM between 0xE0000 and 0xFFFFF.
7355 static struct mp*
7356 mpsearch(void)
7357 {
7358   uchar *bda;
7359   uint p;
7360   struct mp *mp;
7361 
7362   bda = (uchar *) P2V(0x400);
7363   if((p = ((bda[0x0F]<<8)| bda[0x0E]) << 4)){
7364     if((mp = mpsearch1(p, 1024)))
7365       return mp;
7366   } else {
7367     p = ((bda[0x14]<<8)|bda[0x13])*1024;
7368     if((mp = mpsearch1(p-1024, 1024)))
7369       return mp;
7370   }
7371   return mpsearch1(0xF0000, 0x10000);
7372 }
7373 
7374 // Search for an MP configuration table.  For now,
7375 // don't accept the default configurations (physaddr == 0).
7376 // Check for correct signature, calculate the checksum and,
7377 // if correct, check the version.
7378 // To do: check extended table checksum.
7379 static struct mpconf*
7380 mpconfig(struct mp **pmp)
7381 {
7382   struct mpconf *conf;
7383   struct mp *mp;
7384 
7385   if((mp = mpsearch()) == 0 || mp->physaddr == 0)
7386     return 0;
7387   conf = (struct mpconf*) P2V((uint) mp->physaddr);
7388   if(memcmp(conf, "PCMP", 4) != 0)
7389     return 0;
7390   if(conf->version != 1 && conf->version != 4)
7391     return 0;
7392   if(sum((uchar*)conf, conf->length) != 0)
7393     return 0;
7394   *pmp = mp;
7395   return conf;
7396 }
7397 
7398 
7399 
7400 void
7401 mpinit(void)
7402 {
7403   uchar *p, *e;
7404   int ismp;
7405   struct mp *mp;
7406   struct mpconf *conf;
7407   struct mpproc *proc;
7408   struct mpioapic *ioapic;
7409 
7410   if((conf = mpconfig(&mp)) == 0)
7411     panic("Expect to run on an SMP");
7412   ismp = 1;
7413   lapic = (uint*)conf->lapicaddr;
7414   for(p=(uchar*)(conf+1), e=(uchar*)conf+conf->length; p<e; ){
7415     switch(*p){
7416     case MPPROC:
7417       proc = (struct mpproc*)p;
7418       if(ncpu < NCPU) {
7419         cpus[ncpu].apicid = proc->apicid;  // apicid may differ from ncpu
7420         ncpu++;
7421       }
7422       p += sizeof(struct mpproc);
7423       continue;
7424     case MPIOAPIC:
7425       ioapic = (struct mpioapic*)p;
7426       ioapicid = ioapic->apicno;
7427       p += sizeof(struct mpioapic);
7428       continue;
7429     case MPBUS:
7430     case MPIOINTR:
7431     case MPLINTR:
7432       p += 8;
7433       continue;
7434     default:
7435       ismp = 0;
7436       break;
7437     }
7438   }
7439   if(!ismp)
7440     panic("Didn't find a suitable machine");
7441 
7442   if(mp->imcrp){
7443     // Bochs doesn't support IMCR, so this doesn't run on Bochs.
7444     // But it would on real hardware.
7445     outb(0x22, 0x70);   // Select IMCR
7446     outb(0x23, inb(0x23) | 1);  // Mask external interrupts.
7447   }
7448 }
7449 
