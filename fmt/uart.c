8400 // Intel 8250 serial port (UART).
8401 
8402 #include "types.h"
8403 #include "defs.h"
8404 #include "param.h"
8405 #include "traps.h"
8406 #include "spinlock.h"
8407 #include "sleeplock.h"
8408 #include "fs.h"
8409 #include "file.h"
8410 #include "mmu.h"
8411 #include "proc.h"
8412 #include "x86.h"
8413 
8414 #define COM1    0x3f8
8415 
8416 static int uart;    // is there a uart?
8417 
8418 void
8419 uartinit(void)
8420 {
8421   char *p;
8422 
8423   // Turn off the FIFO
8424   outb(COM1+2, 0);
8425 
8426   // 9600 baud, 8 data bits, 1 stop bit, parity off.
8427   outb(COM1+3, 0x80);    // Unlock divisor
8428   outb(COM1+0, 115200/9600);
8429   outb(COM1+1, 0);
8430   outb(COM1+3, 0x03);    // Lock divisor, 8 data bits.
8431   outb(COM1+4, 0);
8432   outb(COM1+1, 0x01);    // Enable receive interrupts.
8433 
8434   // If status is 0xFF, no serial port.
8435   if(inb(COM1+5) == 0xFF)
8436     return;
8437   uart = 1;
8438 
8439   // Acknowledge pre-existing interrupt conditions;
8440   // enable interrupts.
8441   inb(COM1+2);
8442   inb(COM1+0);
8443   ioapicenable(IRQ_COM1, 0);
8444 
8445   // Announce that we're here.
8446   for(p="xv6...\n"; *p; p++)
8447     uartputc(*p);
8448 }
8449 
8450 void
8451 uartputc(int c)
8452 {
8453   int i;
8454 
8455   if(!uart)
8456     return;
8457   for(i = 0; i < 128 && !(inb(COM1+5) & 0x20); i++)
8458     microdelay(10);
8459   outb(COM1+0, c);
8460 }
8461 
8462 static int
8463 uartgetc(void)
8464 {
8465   if(!uart)
8466     return -1;
8467   if(!(inb(COM1+5) & 0x01))
8468     return -1;
8469   return inb(COM1+0);
8470 }
8471 
8472 void
8473 uartintr(void)
8474 {
8475   consoleintr(uartgetc);
8476 }
8477 
8478 
8479 
8480 
8481 
8482 
8483 
8484 
8485 
8486 
8487 
8488 
8489 
8490 
8491 
8492 
8493 
8494 
8495 
8496 
8497 
8498 
8499 
