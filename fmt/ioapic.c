7700 // The I/O APIC manages hardware interrupts for an SMP system.
7701 // http://www.intel.com/design/chipsets/datashts/29056601.pdf
7702 // See also picirq.c.
7703 
7704 #include "types.h"
7705 #include "defs.h"
7706 #include "traps.h"
7707 
7708 #define IOAPIC  0xFEC00000   // Default physical address of IO APIC
7709 
7710 #define REG_ID     0x00  // Register index: ID
7711 #define REG_VER    0x01  // Register index: version
7712 #define REG_TABLE  0x10  // Redirection table base
7713 
7714 // The redirection table starts at REG_TABLE and uses
7715 // two registers to configure each interrupt.
7716 // The first (low) register in a pair contains configuration bits.
7717 // The second (high) register contains a bitmask telling which
7718 // CPUs can serve that interrupt.
7719 #define INT_DISABLED   0x00010000  // Interrupt disabled
7720 #define INT_LEVEL      0x00008000  // Level-triggered (vs edge-)
7721 #define INT_ACTIVELOW  0x00002000  // Active low (vs high)
7722 #define INT_LOGICAL    0x00000800  // Destination is CPU id (vs APIC ID)
7723 
7724 volatile struct ioapic *ioapic;
7725 
7726 // IO APIC MMIO structure: write reg, then read or write data.
7727 struct ioapic {
7728   uint reg;
7729   uint pad[3];
7730   uint data;
7731 };
7732 
7733 static uint
7734 ioapicread(int reg)
7735 {
7736   ioapic->reg = reg;
7737   return ioapic->data;
7738 }
7739 
7740 static void
7741 ioapicwrite(int reg, uint data)
7742 {
7743   ioapic->reg = reg;
7744   ioapic->data = data;
7745 }
7746 
7747 
7748 
7749 
7750 void
7751 ioapicinit(void)
7752 {
7753   int i, id, maxintr;
7754 
7755   ioapic = (volatile struct ioapic*)IOAPIC;
7756   maxintr = (ioapicread(REG_VER) >> 16) & 0xFF;
7757   id = ioapicread(REG_ID) >> 24;
7758   if(id != ioapicid)
7759     cprintf("ioapicinit: id isn't equal to ioapicid; not a MP\n");
7760 
7761   // Mark all interrupts edge-triggered, active high, disabled,
7762   // and not routed to any CPUs.
7763   for(i = 0; i <= maxintr; i++){
7764     ioapicwrite(REG_TABLE+2*i, INT_DISABLED | (T_IRQ0 + i));
7765     ioapicwrite(REG_TABLE+2*i+1, 0);
7766   }
7767 }
7768 
7769 void
7770 ioapicenable(int irq, int cpunum)
7771 {
7772   // Mark interrupt edge-triggered, active high,
7773   // enabled, and routed to the given cpunum,
7774   // which happens to be that cpu's APIC ID.
7775   ioapicwrite(REG_TABLE+2*irq, T_IRQ0 + irq);
7776   ioapicwrite(REG_TABLE+2*irq+1, cpunum << 24);
7777 }
7778 
7779 
7780 
7781 
7782 
7783 
7784 
7785 
7786 
7787 
7788 
7789 
7790 
7791 
7792 
7793 
7794 
7795 
7796 
7797 
7798 
7799 
