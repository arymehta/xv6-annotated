6850 #include "types.h"
6851 #include "defs.h"
6852 #include "param.h"
6853 #include "mmu.h"
6854 #include "proc.h"
6855 #include "fs.h"
6856 #include "spinlock.h"
6857 #include "sleeplock.h"
6858 #include "file.h"
6859 
6860 #define PIPESIZE 512
6861 
6862 struct pipe {
6863   struct spinlock lock;
6864   char data[PIPESIZE];
6865   uint nread;     // number of bytes read
6866   uint nwrite;    // number of bytes written
6867   int readopen;   // read fd is still open
6868   int writeopen;  // write fd is still open
6869 };
6870 
6871 int
6872 pipealloc(struct file **f0, struct file **f1)
6873 {
6874   struct pipe *p;
6875 
6876   p = 0;
6877   *f0 = *f1 = 0;
6878   if((*f0 = filealloc()) == 0 || (*f1 = filealloc()) == 0)
6879     goto bad;
6880   if((p = (struct pipe*)kalloc()) == 0)
6881     goto bad;
6882   p->readopen = 1;
6883   p->writeopen = 1;
6884   p->nwrite = 0;
6885   p->nread = 0;
6886   initlock(&p->lock, "pipe");
6887   (*f0)->type = FD_PIPE;
6888   (*f0)->readable = 1;
6889   (*f0)->writable = 0;
6890   (*f0)->pipe = p;
6891   (*f1)->type = FD_PIPE;
6892   (*f1)->readable = 0;
6893   (*f1)->writable = 1;
6894   (*f1)->pipe = p;
6895   return 0;
6896 
6897 
6898 
6899 
6900  bad:
6901   if(p)
6902     kfree((char*)p);
6903   if(*f0)
6904     fileclose(*f0);
6905   if(*f1)
6906     fileclose(*f1);
6907   return -1;
6908 }
6909 
6910 void
6911 pipeclose(struct pipe *p, int writable)
6912 {
6913   acquire(&p->lock);
6914   if(writable){
6915     p->writeopen = 0;
6916     wakeup(&p->nread);
6917   } else {
6918     p->readopen = 0;
6919     wakeup(&p->nwrite);
6920   }
6921   if(p->readopen == 0 && p->writeopen == 0){
6922     release(&p->lock);
6923     kfree((char*)p);
6924   } else
6925     release(&p->lock);
6926 }
6927 
6928 
6929 int
6930 pipewrite(struct pipe *p, char *addr, int n)
6931 {
6932   int i;
6933 
6934   acquire(&p->lock);
6935   for(i = 0; i < n; i++){
6936     while(p->nwrite == p->nread + PIPESIZE){  //DOC: pipewrite-full
6937       if(p->readopen == 0 || myproc()->killed){
6938         release(&p->lock);
6939         return -1;
6940       }
6941       wakeup(&p->nread);
6942       sleep(&p->nwrite, &p->lock);  //DOC: pipewrite-sleep
6943     }
6944     p->data[p->nwrite++ % PIPESIZE] = addr[i];
6945   }
6946   wakeup(&p->nread);  //DOC: pipewrite-wakeup1
6947   release(&p->lock);
6948   return n;
6949 }
6950 int
6951 piperead(struct pipe *p, char *addr, int n)
6952 {
6953   int i;
6954 
6955   acquire(&p->lock);
6956   while(p->nread == p->nwrite && p->writeopen){  //DOC: pipe-empty
6957     if(myproc()->killed){
6958       release(&p->lock);
6959       return -1;
6960     }
6961     sleep(&p->nread, &p->lock); //DOC: piperead-sleep
6962   }
6963   for(i = 0; i < n; i++){  //DOC: piperead-copy
6964     if(p->nread == p->nwrite)
6965       break;
6966     addr[i] = p->data[p->nread++ % PIPESIZE];
6967   }
6968   wakeup(&p->nwrite);  //DOC: piperead-wakeup
6969   release(&p->lock);
6970   return i;
6971 }
6972 
6973 
6974 
6975 
6976 
6977 
6978 
6979 
6980 
6981 
6982 
6983 
6984 
6985 
6986 
6987 
6988 
6989 
6990 
6991 
6992 
6993 
6994 
6995 
6996 
6997 
6998 
6999 
