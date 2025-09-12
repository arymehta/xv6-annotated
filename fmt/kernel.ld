9450 /* Simple linker script for the JOS kernel.
9451    See the GNU ld 'info' manual ("info ld") to learn the syntax. */
9452 
9453 OUTPUT_FORMAT("elf32-i386", "elf32-i386", "elf32-i386")
9454 OUTPUT_ARCH(i386)
9455 ENTRY(_start)
9456 
9457 SECTIONS
9458 {
9459 	/* Link the kernel at this address: "." means the current address */
9460         /* Must be equal to KERNLINK */
9461 	. = 0x80100000;
9462 
9463 	.text : AT(0x100000) {
9464 		*(.text .stub .text.* .gnu.linkonce.t.*)
9465 	}
9466 
9467 	PROVIDE(etext = .);	/* Define the 'etext' symbol to this value */
9468 
9469 	.rodata : {
9470 		*(.rodata .rodata.* .gnu.linkonce.r.*)
9471 	}
9472 
9473 	/* Include debugging information in kernel memory */
9474 	.stab : {
9475 		PROVIDE(__STAB_BEGIN__ = .);
9476 		*(.stab);
9477 		PROVIDE(__STAB_END__ = .);
9478 	}
9479 
9480 	.stabstr : {
9481 		PROVIDE(__STABSTR_BEGIN__ = .);
9482 		*(.stabstr);
9483 		PROVIDE(__STABSTR_END__ = .);
9484 	}
9485 
9486 	/* Adjust the address for the data segment to the next page */
9487 	. = ALIGN(0x1000);
9488 
9489 	/* Conventionally, Unix linkers provide pseudo-symbols
9490 	 * etext, edata, and end, at the end of the text, data, and bss.
9491 	 * For the kernel mapping, we need the address at the beginning
9492 	 * of the data section, but that's not one of the conventional
9493 	 * symbols, because the convention started before there was a
9494 	 * read-only rodata section between text and data. */
9495 	PROVIDE(data = .);
9496 
9497 
9498 
9499 
9500 	/* The data segment */
9501 	.data : {
9502 		*(.data)
9503 	}
9504 
9505 	PROVIDE(edata = .);
9506 
9507 	.bss : {
9508 		*(.bss)
9509 	}
9510 
9511 	PROVIDE(end = .);
9512 
9513 	/DISCARD/ : {
9514 		*(.eh_frame .note.GNU-stack)
9515 	}
9516 }
9517 
9518 
9519 
9520 
9521 
9522 
9523 
9524 
9525 
9526 
9527 
9528 
9529 
9530 
9531 
9532 
9533 
9534 
9535 
9536 
9537 
9538 
9539 
9540 
9541 
9542 
9543 
9544 
9545 
9546 
9547 
9548 
9549 
