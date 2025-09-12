9200 #include "asm.h"
9201 #include "memlayout.h"
9202 #include "mmu.h"
9203 
9204 # Start the first CPU: switch to 32-bit protected mode, jump into C.
9205 # The BIOS loads this code from the first sector of the hard disk into
9206 # memory at physical address 0x7c00 and starts executing in real mode
9207 # with %cs=0 %ip=7c00.
9208 
9209 .code16                       # Assemble for 16-bit mode
9210 .globl start
9211 start:
9212   cli                         # BIOS enabled interrupts; disable
9213 
9214   # Zero data segment registers DS, ES, and SS.
9215   xorw    %ax,%ax             # Set %ax to zero
9216   movw    %ax,%ds             # -> Data Segment
9217   movw    %ax,%es             # -> Extra Segment
9218   movw    %ax,%ss             # -> Stack Segment
9219 
9220   # Physical address line A20 is tied to zero so that the first PCs
9221   # with 2 MB would run software that assumed 1 MB.  Undo that.
9222 seta20.1:
9223   inb     $0x64,%al               # Wait for not busy
9224   testb   $0x2,%al
9225   jnz     seta20.1
9226 
9227   movb    $0xd1,%al               # 0xd1 -> port 0x64
9228   outb    %al,$0x64
9229 
9230 seta20.2:
9231   inb     $0x64,%al               # Wait for not busy
9232   testb   $0x2,%al
9233   jnz     seta20.2
9234 
9235   movb    $0xdf,%al               # 0xdf -> port 0x60
9236   outb    %al,$0x60
9237 
9238   # Switch from real to protected mode.  Use a bootstrap GDT that makes
9239   # virtual addresses map directly to physical addresses so that the
9240   # effective memory map doesn't change during the transition.
9241   lgdt    gdtdesc
9242   movl    %cr0, %eax
9243   orl     $CR0_PE, %eax
9244   movl    %eax, %cr0
9245 
9246 
9247 
9248 
9249 
9250   # Complete the transition to 32-bit protected mode by using a long jmp
9251   # to reload %cs and %eip.  The segment descriptors are set up with no
9252   # translation, so that the mapping is still the identity mapping.
9253   ljmp    $(SEG_KCODE<<3), $start32
9254 
9255 .code32  # Tell assembler to generate 32-bit code now.
9256 start32:
9257   # Set up the protected-mode data segment registers
9258   movw    $(SEG_KDATA<<3), %ax    # Our data segment selector
9259   movw    %ax, %ds                # -> DS: Data Segment
9260   movw    %ax, %es                # -> ES: Extra Segment
9261   movw    %ax, %ss                # -> SS: Stack Segment
9262   movw    $0, %ax                 # Zero segments not ready for use
9263   movw    %ax, %fs                # -> FS
9264   movw    %ax, %gs                # -> GS
9265 
9266   # Set up the stack pointer and call into C.
9267   movl    $start, %esp
9268   call    bootmain
9269 
9270   # If bootmain returns (it shouldn't), trigger a Bochs
9271   # breakpoint if running under Bochs, then loop.
9272   movw    $0x8a00, %ax            # 0x8a00 -> port 0x8a00
9273   movw    %ax, %dx
9274   outw    %ax, %dx
9275   movw    $0x8ae0, %ax            # 0x8ae0 -> port 0x8a00
9276   outw    %ax, %dx
9277 spin:
9278   jmp     spin
9279 
9280 # Bootstrap GDT
9281 .p2align 2                                # force 4 byte alignment
9282 gdt:
9283   SEG_NULLASM                             # null seg
9284   SEG_ASM(STA_X|STA_R, 0x0, 0xffffffff)   # code seg
9285   SEG_ASM(STA_W, 0x0, 0xffffffff)         # data seg
9286 
9287 gdtdesc:
9288   .word   (gdtdesc - gdt - 1)             # sizeof(gdt) - 1
9289   .long   gdt                             # address gdt
9290 
9291 
9292 
9293 
9294 
9295 
9296 
9297 
9298 
9299 
