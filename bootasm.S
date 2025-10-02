#include "asm.h"
#include "memlayout.h"
#include "mmu.h"

# Start the first CPU: switch to 32-bit protected mode, jump into C.
# The BIOS loads this code from the first sector of the hard disk into
# memory at physical address 0x7c00 and starts executing in real mode
# with %cs=0 %ip=7c00.

/* Aryan
 * Starting point of the bootloader, part of bootblock, 0x7C00 onwards
 * BIOS (qemu) will initialise the IP to start at 0x7C00
 * */
.code16                       # Assemble for 16-bit mode
.globl start
start:
    /*
     * Aryan
     * hardware interrupts disabled
     * this is done so that bios will not respond to any hardware interrupts like keyboard press, etx
     * Later re enabled in scheduler code of kernel
     * BIOS set CS to  0 and IP to 0x7C00 (line 14)
     */

  cli                         # BIOS enabled interrupts; disable

  # Zero data segment registers DS, ES, and SS.
  xorw    %ax,%ax             # Set %ax to zero
  movw    %ax,%ds             # -> Data Segment
  movw    %ax,%es             # -> Extra Segment
  movw    %ax,%ss             # -> Stack Segment

  # Physical address line A20 is tied to zero so that the first PCs
  # with 2 MB would run software that assumed 1 MB.  Undo that.
seta20.1:
  inb     $0x64,%al               # Wait for not busy
  testb   $0x2,%al
  jnz     seta20.1

  movb    $0xd1,%al               # 0xd1 -> port 0x64
  outb    %al,$0x64

seta20.2:
  inb     $0x64,%al               # Wait for not busy
  testb   $0x2,%al
  jnz     seta20.2

  movb    $0xdf,%al               # 0xdf -> port 0x60
  outb    %al,$0x60

  # Switch from real to protected mode.  Use a bootstrap GDT that makes
  # virtual addresses map directly to physical addresses so that the
  # effective memory map doesn't change during the transition.

  /* Aryan
   * lgtd -- load gdt register
   * gdt: stores base of the SEGMENT table
   * indexes to segment table given by segment registers DS,CS ES.. etc (all set to 0 above)
   */
  lgdt    gdtdesc


# Aryan: sets the protection enable  bit: means ready to shift to 32 bit mode (running in 16 bit till now)
  movl    %cr0, %eax
  orl     $CR0_PE, %eax
  movl    %eax, %cr0

//PAGEBREAK!
  # Complete the transition to 32-bit protected mode by using a long jmp
  # to reload %cs and %eip.  The segment descriptors are set up with no
  # translation, so that the mapping is still the identity mapping.

  ljmp    $(SEG_KCODE<<3), $start32

.code32  # Tell assembler to generate 32-bit code now.
start32:

  # Set up the protected-mode data segment registers
  movw    $(SEG_KDATA<<3), %ax    # Our data segment selector
  movw    %ax, %ds                # -> DS: Data Segment
  movw    %ax, %es                # -> ES: Extra Segment
  movw    %ax, %ss                # -> SS: Stack Segment
  movw    $0, %ax                 # Zero segments not ready for use
  movw    %ax, %fs                # -> FS
  movw    %ax, %gs                # -> GS

  # Set up the stack pointer and call into C.

  movl    $start, %esp
  # Aryan: stack ptr esp = 0x7C00 means usable stack right now is 0x7C00 to 0
  call    bootmain

  # If bootmain returns (it shouldn't), trigger a Bochs
  # breakpoint if running under Bochs, then loop.
  movw    $0x8a00, %ax            # 0x8a00 -> port 0x8a00
  movw    %ax, %dx
  outw    %ax, %dx
  movw    $0x8ae0, %ax            # 0x8ae0 -> port 0x8a00
  outw    %ax, %dx
spin:
  jmp     spin

# Bootstrap GDT
.p2align 2                                # force 4 byte alignment

/* Aryan:
 * - 3 entries in GDT, null segment, code and data segment
 * - set code and data values Base = 0 Limit = 4GB
 * - essentially disabling segmentation
 * - xv6 has segmentation + paging, but segmentation has no effect, because all segments erfectly overlap
 */
gdt:
  SEG_NULLASM                             # null seg
  SEG_ASM(STA_X|STA_R, 0x0, 0xffffffff)   # code seg
  SEG_ASM(STA_W, 0x0, 0xffffffff)         # data seg

gdtdesc:
  .word   (gdtdesc - gdt - 1)             # sizeof(gdt) - 1
  .long   gdt                             # address gdt
/* Aryan: gdtdesc .word part contains last index of table, that is why gdtdesc - gdt (-1)
 * Suppose gdt is @ a
 * a + 1
 * a + 2
 * a + 3
 * gdtdesc @ a + 4
 * gdtdesc - gdt = 3, 3 -  1 = 2 --> last index of gdt table
*/



/* SUMMARY of bootasm.S
 *
 *  - Switches from real(16 bit) mode to protected mode and sets up segmentation
 *
 *  STEPS:
 *  1. Disables interrupts (re enabled in scheduler)
 *  2. Initialises segment registers DS, ES, SS to 0
 *  3. Some miscellaneous code to enable 21st bit.
 *  4. Initialises GDT register, with HARDCODED GDT table
 *  5. Sets protection bit in CR0
 *  6. Jumps to 32 bit (long jump)
 *  7. Sets up protected mode segment registers
 *  8. Sets esp = 7C00 and usable stack 7c00 to 0
 *  9. Jumps to bootmain
 */
