8000 // Console input and output.
8001 // Input is from the keyboard or serial port.
8002 // Output is written to the screen and serial port.
8003 
8004 #include "types.h"
8005 #include "defs.h"
8006 #include "param.h"
8007 #include "traps.h"
8008 #include "spinlock.h"
8009 #include "sleeplock.h"
8010 #include "fs.h"
8011 #include "file.h"
8012 #include "memlayout.h"
8013 #include "mmu.h"
8014 #include "proc.h"
8015 #include "x86.h"
8016 
8017 static void consputc(int);
8018 
8019 static int panicked = 0;
8020 
8021 static struct {
8022   struct spinlock lock;
8023   int locking;
8024 } cons;
8025 
8026 static void
8027 printint(int xx, int base, int sign)
8028 {
8029   static char digits[] = "0123456789abcdef";
8030   char buf[16];
8031   int i;
8032   uint x;
8033 
8034   if(sign && (sign = xx < 0))
8035     x = -xx;
8036   else
8037     x = xx;
8038 
8039   i = 0;
8040   do{
8041     buf[i++] = digits[x % base];
8042   }while((x /= base) != 0);
8043 
8044   if(sign)
8045     buf[i++] = '-';
8046 
8047   while(--i >= 0)
8048     consputc(buf[i]);
8049 }
8050 
8051 
8052 
8053 
8054 
8055 
8056 
8057 
8058 
8059 
8060 
8061 
8062 
8063 
8064 
8065 
8066 
8067 
8068 
8069 
8070 
8071 
8072 
8073 
8074 
8075 
8076 
8077 
8078 
8079 
8080 
8081 
8082 
8083 
8084 
8085 
8086 
8087 
8088 
8089 
8090 
8091 
8092 
8093 
8094 
8095 
8096 
8097 
8098 
8099 
8100 // Print to the console. only understands %d, %x, %p, %s.
8101 void
8102 cprintf(char *fmt, ...)
8103 {
8104   int i, c, locking;
8105   uint *argp;
8106   char *s;
8107 
8108   locking = cons.locking;
8109   if(locking)
8110     acquire(&cons.lock);
8111 
8112   if (fmt == 0)
8113     panic("null fmt");
8114 
8115   argp = (uint*)(void*)(&fmt + 1);
8116   for(i = 0; (c = fmt[i] & 0xff) != 0; i++){
8117     if(c != '%'){
8118       consputc(c);
8119       continue;
8120     }
8121     c = fmt[++i] & 0xff;
8122     if(c == 0)
8123       break;
8124     switch(c){
8125     case 'd':
8126       printint(*argp++, 10, 1);
8127       break;
8128     case 'x':
8129     case 'p':
8130       printint(*argp++, 16, 0);
8131       break;
8132     case 's':
8133       if((s = (char*)*argp++) == 0)
8134         s = "(null)";
8135       for(; *s; s++)
8136         consputc(*s);
8137       break;
8138     case '%':
8139       consputc('%');
8140       break;
8141     default:
8142       // Print unknown % sequence to draw attention.
8143       consputc('%');
8144       consputc(c);
8145       break;
8146     }
8147   }
8148 
8149 
8150   if(locking)
8151     release(&cons.lock);
8152 }
8153 
8154 void
8155 panic(char *s)
8156 {
8157   int i;
8158   uint pcs[10];
8159 
8160   cli();
8161   cons.locking = 0;
8162   // use lapiccpunum so that we can call panic from mycpu()
8163   cprintf("lapicid %d: panic: ", lapicid());
8164   cprintf(s);
8165   cprintf("\n");
8166   getcallerpcs(&s, pcs);
8167   for(i=0; i<10; i++)
8168     cprintf(" %p", pcs[i]);
8169   panicked = 1; // freeze other CPU
8170   for(;;)
8171     ;
8172 }
8173 
8174 
8175 
8176 
8177 
8178 
8179 
8180 
8181 
8182 
8183 
8184 
8185 
8186 
8187 
8188 
8189 
8190 
8191 
8192 
8193 
8194 
8195 
8196 
8197 
8198 
8199 
8200 #define BACKSPACE 0x100
8201 #define CRTPORT 0x3d4
8202 static ushort *crt = (ushort*)P2V(0xb8000);  // CGA memory
8203 
8204 static void
8205 cgaputc(int c)
8206 {
8207   int pos;
8208 
8209   // Cursor position: col + 80*row.
8210   outb(CRTPORT, 14);
8211   pos = inb(CRTPORT+1) << 8;
8212   outb(CRTPORT, 15);
8213   pos |= inb(CRTPORT+1);
8214 
8215   if(c == '\n')
8216     pos += 80 - pos%80;
8217   else if(c == BACKSPACE){
8218     if(pos > 0) --pos;
8219   } else
8220     crt[pos++] = (c&0xff) | 0x0700;  // black on white
8221 
8222   if(pos < 0 || pos > 25*80)
8223     panic("pos under/overflow");
8224 
8225   if((pos/80) >= 24){  // Scroll up.
8226     memmove(crt, crt+80, sizeof(crt[0])*23*80);
8227     pos -= 80;
8228     memset(crt+pos, 0, sizeof(crt[0])*(24*80 - pos));
8229   }
8230 
8231   outb(CRTPORT, 14);
8232   outb(CRTPORT+1, pos>>8);
8233   outb(CRTPORT, 15);
8234   outb(CRTPORT+1, pos);
8235   crt[pos] = ' ' | 0x0700;
8236 }
8237 
8238 
8239 
8240 
8241 
8242 
8243 
8244 
8245 
8246 
8247 
8248 
8249 
8250 void
8251 consputc(int c)
8252 {
8253   if(panicked){
8254     cli();
8255     for(;;)
8256       ;
8257   }
8258 
8259   if(c == BACKSPACE){
8260     uartputc('\b'); uartputc(' '); uartputc('\b');
8261   } else
8262     uartputc(c);
8263   cgaputc(c);
8264 }
8265 
8266 #define INPUT_BUF 128
8267 struct {
8268   char buf[INPUT_BUF];
8269   uint r;  // Read index
8270   uint w;  // Write index
8271   uint e;  // Edit index
8272 } input;
8273 
8274 #define C(x)  ((x)-'@')  // Control-x
8275 
8276 void
8277 consoleintr(int (*getc)(void))
8278 {
8279   int c, doprocdump = 0;
8280 
8281   acquire(&cons.lock);
8282   while((c = getc()) >= 0){
8283     switch(c){
8284     case C('P'):  // Process listing.
8285       // procdump() locks cons.lock indirectly; invoke later
8286       doprocdump = 1;
8287       break;
8288     case C('U'):  // Kill line.
8289       while(input.e != input.w &&
8290             input.buf[(input.e-1) % INPUT_BUF] != '\n'){
8291         input.e--;
8292         consputc(BACKSPACE);
8293       }
8294       break;
8295     case C('H'): case '\x7f':  // Backspace
8296       if(input.e != input.w){
8297         input.e--;
8298         consputc(BACKSPACE);
8299       }
8300       break;
8301     default:
8302       if(c != 0 && input.e-input.r < INPUT_BUF){
8303         c = (c == '\r') ? '\n' : c;
8304         input.buf[input.e++ % INPUT_BUF] = c;
8305         consputc(c);
8306         if(c == '\n' || c == C('D') || input.e == input.r+INPUT_BUF){
8307           input.w = input.e;
8308           wakeup(&input.r);
8309         }
8310       }
8311       break;
8312     }
8313   }
8314   release(&cons.lock);
8315   if(doprocdump) {
8316     procdump();  // now call procdump() wo. cons.lock held
8317   }
8318 }
8319 
8320 int
8321 consoleread(struct inode *ip, char *dst, int n)
8322 {
8323   uint target;
8324   int c;
8325 
8326   iunlock(ip);
8327   target = n;
8328   acquire(&cons.lock);
8329   while(n > 0){
8330     while(input.r == input.w){
8331       if(myproc()->killed){
8332         release(&cons.lock);
8333         ilock(ip);
8334         return -1;
8335       }
8336       sleep(&input.r, &cons.lock);
8337     }
8338     c = input.buf[input.r++ % INPUT_BUF];
8339     if(c == C('D')){  // EOF
8340       if(n < target){
8341         // Save ^D for next time, to make sure
8342         // caller gets a 0-byte result.
8343         input.r--;
8344       }
8345       break;
8346     }
8347     *dst++ = c;
8348     --n;
8349     if(c == '\n')
8350       break;
8351   }
8352   release(&cons.lock);
8353   ilock(ip);
8354 
8355   return target - n;
8356 }
8357 
8358 int
8359 consolewrite(struct inode *ip, char *buf, int n)
8360 {
8361   int i;
8362 
8363   iunlock(ip);
8364   acquire(&cons.lock);
8365   for(i = 0; i < n; i++)
8366     consputc(buf[i] & 0xff);
8367   release(&cons.lock);
8368   ilock(ip);
8369 
8370   return n;
8371 }
8372 
8373 void
8374 consoleinit(void)
8375 {
8376   initlock(&cons.lock, "console");
8377 
8378   devsw[CONSOLE].write = consolewrite;
8379   devsw[CONSOLE].read = consoleread;
8380   cons.locking = 1;
8381 
8382   ioapicenable(IRQ_KBD, 0);
8383 }
8384 
8385 
8386 
8387 
8388 
8389 
8390 
8391 
8392 
8393 
8394 
8395 
8396 
8397 
8398 
8399 
