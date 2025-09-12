7450 // The local APIC manages internal (non-I/O) interrupts.
7451 // See Chapter 8 & Appendix C of Intel processor manual volume 3.
7452 
7453 #include "param.h"
7454 #include "types.h"
7455 #include "defs.h"
7456 #include "date.h"
7457 #include "memlayout.h"
7458 #include "traps.h"
7459 #include "mmu.h"
7460 #include "x86.h"
7461 
7462 // Local APIC registers, divided by 4 for use as uint[] indices.
7463 #define ID      (0x0020/4)   // ID
7464 #define VER     (0x0030/4)   // Version
7465 #define TPR     (0x0080/4)   // Task Priority
7466 #define EOI     (0x00B0/4)   // EOI
7467 #define SVR     (0x00F0/4)   // Spurious Interrupt Vector
7468   #define ENABLE     0x00000100   // Unit Enable
7469 #define ESR     (0x0280/4)   // Error Status
7470 #define ICRLO   (0x0300/4)   // Interrupt Command
7471   #define INIT       0x00000500   // INIT/RESET
7472   #define STARTUP    0x00000600   // Startup IPI
7473   #define DELIVS     0x00001000   // Delivery status
7474   #define ASSERT     0x00004000   // Assert interrupt (vs deassert)
7475   #define DEASSERT   0x00000000
7476   #define LEVEL      0x00008000   // Level triggered
7477   #define BCAST      0x00080000   // Send to all APICs, including self.
7478   #define BUSY       0x00001000
7479   #define FIXED      0x00000000
7480 #define ICRHI   (0x0310/4)   // Interrupt Command [63:32]
7481 #define TIMER   (0x0320/4)   // Local Vector Table 0 (TIMER)
7482   #define X1         0x0000000B   // divide counts by 1
7483   #define PERIODIC   0x00020000   // Periodic
7484 #define PCINT   (0x0340/4)   // Performance Counter LVT
7485 #define LINT0   (0x0350/4)   // Local Vector Table 1 (LINT0)
7486 #define LINT1   (0x0360/4)   // Local Vector Table 2 (LINT1)
7487 #define ERROR   (0x0370/4)   // Local Vector Table 3 (ERROR)
7488   #define MASKED     0x00010000   // Interrupt masked
7489 #define TICR    (0x0380/4)   // Timer Initial Count
7490 #define TCCR    (0x0390/4)   // Timer Current Count
7491 #define TDCR    (0x03E0/4)   // Timer Divide Configuration
7492 
7493 volatile uint *lapic;  // Initialized in mp.c
7494 
7495 
7496 
7497 
7498 
7499 
7500 static void
7501 lapicw(int index, int value)
7502 {
7503   lapic[index] = value;
7504   lapic[ID];  // wait for write to finish, by reading
7505 }
7506 
7507 void
7508 lapicinit(void)
7509 {
7510   if(!lapic)
7511     return;
7512 
7513   // Enable local APIC; set spurious interrupt vector.
7514   lapicw(SVR, ENABLE | (T_IRQ0 + IRQ_SPURIOUS));
7515 
7516   // The timer repeatedly counts down at bus frequency
7517   // from lapic[TICR] and then issues an interrupt.
7518   // If xv6 cared more about precise timekeeping,
7519   // TICR would be calibrated using an external time source.
7520   lapicw(TDCR, X1);
7521   lapicw(TIMER, PERIODIC | (T_IRQ0 + IRQ_TIMER));
7522   lapicw(TICR, 10000000);
7523 
7524   // Disable logical interrupt lines.
7525   lapicw(LINT0, MASKED);
7526   lapicw(LINT1, MASKED);
7527 
7528   // Disable performance counter overflow interrupts
7529   // on machines that provide that interrupt entry.
7530   if(((lapic[VER]>>16) & 0xFF) >= 4)
7531     lapicw(PCINT, MASKED);
7532 
7533   // Map error interrupt to IRQ_ERROR.
7534   lapicw(ERROR, T_IRQ0 + IRQ_ERROR);
7535 
7536   // Clear error status register (requires back-to-back writes).
7537   lapicw(ESR, 0);
7538   lapicw(ESR, 0);
7539 
7540   // Ack any outstanding interrupts.
7541   lapicw(EOI, 0);
7542 
7543   // Send an Init Level De-Assert to synchronise arbitration ID's.
7544   lapicw(ICRHI, 0);
7545   lapicw(ICRLO, BCAST | INIT | LEVEL);
7546   while(lapic[ICRLO] & DELIVS)
7547     ;
7548 
7549 
7550   // Enable interrupts on the APIC (but not on the processor).
7551   lapicw(TPR, 0);
7552 }
7553 
7554 int
7555 lapicid(void)
7556 {
7557   if (!lapic)
7558     return 0;
7559   return lapic[ID] >> 24;
7560 }
7561 
7562 // Acknowledge interrupt.
7563 void
7564 lapiceoi(void)
7565 {
7566   if(lapic)
7567     lapicw(EOI, 0);
7568 }
7569 
7570 // Spin for a given number of microseconds.
7571 // On real hardware would want to tune this dynamically.
7572 void
7573 microdelay(int us)
7574 {
7575 }
7576 
7577 #define CMOS_PORT    0x70
7578 #define CMOS_RETURN  0x71
7579 
7580 // Start additional processor running entry code at addr.
7581 // See Appendix B of MultiProcessor Specification.
7582 void
7583 lapicstartap(uchar apicid, uint addr)
7584 {
7585   int i;
7586   ushort *wrv;
7587 
7588   // "The BSP must initialize CMOS shutdown code to 0AH
7589   // and the warm reset vector (DWORD based at 40:67) to point at
7590   // the AP startup code prior to the [universal startup algorithm]."
7591   outb(CMOS_PORT, 0xF);  // offset 0xF is shutdown code
7592   outb(CMOS_PORT+1, 0x0A);
7593   wrv = (ushort*)P2V((0x40<<4 | 0x67));  // Warm reset vector
7594   wrv[0] = 0;
7595   wrv[1] = addr >> 4;
7596 
7597 
7598 
7599 
7600   // "Universal startup algorithm."
7601   // Send INIT (level-triggered) interrupt to reset other CPU.
7602   lapicw(ICRHI, apicid<<24);
7603   lapicw(ICRLO, INIT | LEVEL | ASSERT);
7604   microdelay(200);
7605   lapicw(ICRLO, INIT | LEVEL);
7606   microdelay(100);    // should be 10ms, but too slow in Bochs!
7607 
7608   // Send startup IPI (twice!) to enter code.
7609   // Regular hardware is supposed to only accept a STARTUP
7610   // when it is in the halted state due to an INIT.  So the second
7611   // should be ignored, but it is part of the official Intel algorithm.
7612   // Bochs complains about the second one.  Too bad for Bochs.
7613   for(i = 0; i < 2; i++){
7614     lapicw(ICRHI, apicid<<24);
7615     lapicw(ICRLO, STARTUP | (addr>>12));
7616     microdelay(200);
7617   }
7618 }
7619 
7620 #define CMOS_STATA   0x0a
7621 #define CMOS_STATB   0x0b
7622 #define CMOS_UIP    (1 << 7)        // RTC update in progress
7623 
7624 #define SECS    0x00
7625 #define MINS    0x02
7626 #define HOURS   0x04
7627 #define DAY     0x07
7628 #define MONTH   0x08
7629 #define YEAR    0x09
7630 
7631 static uint
7632 cmos_read(uint reg)
7633 {
7634   outb(CMOS_PORT,  reg);
7635   microdelay(200);
7636 
7637   return inb(CMOS_RETURN);
7638 }
7639 
7640 static void
7641 fill_rtcdate(struct rtcdate *r)
7642 {
7643   r->second = cmos_read(SECS);
7644   r->minute = cmos_read(MINS);
7645   r->hour   = cmos_read(HOURS);
7646   r->day    = cmos_read(DAY);
7647   r->month  = cmos_read(MONTH);
7648   r->year   = cmos_read(YEAR);
7649 }
7650 // qemu seems to use 24-hour GWT and the values are BCD encoded
7651 void
7652 cmostime(struct rtcdate *r)
7653 {
7654   struct rtcdate t1, t2;
7655   int sb, bcd;
7656 
7657   sb = cmos_read(CMOS_STATB);
7658 
7659   bcd = (sb & (1 << 2)) == 0;
7660 
7661   // make sure CMOS doesn't modify time while we read it
7662   for(;;) {
7663     fill_rtcdate(&t1);
7664     if(cmos_read(CMOS_STATA) & CMOS_UIP)
7665         continue;
7666     fill_rtcdate(&t2);
7667     if(memcmp(&t1, &t2, sizeof(t1)) == 0)
7668       break;
7669   }
7670 
7671   // convert
7672   if(bcd) {
7673 #define    CONV(x)     (t1.x = ((t1.x >> 4) * 10) + (t1.x & 0xf))
7674     CONV(second);
7675     CONV(minute);
7676     CONV(hour  );
7677     CONV(day   );
7678     CONV(month );
7679     CONV(year  );
7680 #undef     CONV
7681   }
7682 
7683   *r = t1;
7684   r->year += 2000;
7685 }
7686 
7687 
7688 
7689 
7690 
7691 
7692 
7693 
7694 
7695 
7696 
7697 
7698 
7699 
