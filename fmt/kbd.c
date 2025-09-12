7950 #include "types.h"
7951 #include "x86.h"
7952 #include "defs.h"
7953 #include "kbd.h"
7954 
7955 int
7956 kbdgetc(void)
7957 {
7958   static uint shift;
7959   static uchar *charcode[4] = {
7960     normalmap, shiftmap, ctlmap, ctlmap
7961   };
7962   uint st, data, c;
7963 
7964   st = inb(KBSTATP);
7965   if((st & KBS_DIB) == 0)
7966     return -1;
7967   data = inb(KBDATAP);
7968 
7969   if(data == 0xE0){
7970     shift |= E0ESC;
7971     return 0;
7972   } else if(data & 0x80){
7973     // Key released
7974     data = (shift & E0ESC ? data : data & 0x7F);
7975     shift &= ~(shiftcode[data] | E0ESC);
7976     return 0;
7977   } else if(shift & E0ESC){
7978     // Last character was an E0 escape; or with 0x80
7979     data |= 0x80;
7980     shift &= ~E0ESC;
7981   }
7982 
7983   shift |= shiftcode[data];
7984   shift ^= togglecode[data];
7985   c = charcode[shift & (CTL | SHIFT)][data];
7986   if(shift & CAPSLOCK){
7987     if('a' <= c && c <= 'z')
7988       c += 'A' - 'a';
7989     else if('A' <= c && c <= 'Z')
7990       c += 'a' - 'A';
7991   }
7992   return c;
7993 }
7994 
7995 void
7996 kbdintr(void)
7997 {
7998   consoleintr(kbdgetc);
7999 }
