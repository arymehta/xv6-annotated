6700 #include "types.h"
6701 #include "param.h"
6702 #include "memlayout.h"
6703 #include "mmu.h"
6704 #include "proc.h"
6705 #include "defs.h"
6706 #include "x86.h"
6707 #include "elf.h"
6708 
6709 int
6710 exec(char *path, char **argv)
6711 {
6712   char *s, *last;
6713   int i, off;
6714   uint argc, sz, sp, ustack[3+MAXARG+1];
6715   struct elfhdr elf;
6716   struct inode *ip;
6717   struct proghdr ph;
6718   pde_t *pgdir, *oldpgdir;
6719   struct proc *curproc = myproc();
6720 
6721   begin_op();
6722 
6723   if((ip = namei(path)) == 0){
6724     end_op();
6725     cprintf("exec: fail\n");
6726     return -1;
6727   }
6728   ilock(ip);
6729   pgdir = 0;
6730 
6731   // Check ELF header
6732   if(readi(ip, (char*)&elf, 0, sizeof(elf)) != sizeof(elf))
6733     goto bad;
6734   if(elf.magic != ELF_MAGIC)
6735     goto bad;
6736 
6737   if((pgdir = setupkvm()) == 0)
6738     goto bad;
6739 
6740   // Load program into memory.
6741   sz = 0;
6742   for(i=0, off=elf.phoff; i<elf.phnum; i++, off+=sizeof(ph)){
6743     if(readi(ip, (char*)&ph, off, sizeof(ph)) != sizeof(ph))
6744       goto bad;
6745     if(ph.type != ELF_PROG_LOAD)
6746       continue;
6747     if(ph.memsz < ph.filesz)
6748       goto bad;
6749     if(ph.vaddr + ph.memsz < ph.vaddr)
6750       goto bad;
6751     if((sz = allocuvm(pgdir, sz, ph.vaddr + ph.memsz)) == 0)
6752       goto bad;
6753     if(ph.vaddr % PGSIZE != 0)
6754       goto bad;
6755     if(loaduvm(pgdir, (char*)ph.vaddr, ip, ph.off, ph.filesz) < 0)
6756       goto bad;
6757   }
6758   iunlockput(ip);
6759   end_op();
6760   ip = 0;
6761 
6762   // Allocate two pages at the next page boundary.
6763   // Make the first inaccessible.  Use the second as the user stack.
6764   sz = PGROUNDUP(sz);
6765   if((sz = allocuvm(pgdir, sz, sz + 2*PGSIZE)) == 0)
6766     goto bad;
6767   clearpteu(pgdir, (char*)(sz - 2*PGSIZE));
6768   sp = sz;
6769 
6770   // Push argument strings, prepare rest of stack in ustack.
6771   for(argc = 0; argv[argc]; argc++) {
6772     if(argc >= MAXARG)
6773       goto bad;
6774     sp = (sp - (strlen(argv[argc]) + 1)) & ~3;
6775     if(copyout(pgdir, sp, argv[argc], strlen(argv[argc]) + 1) < 0)
6776       goto bad;
6777     ustack[3+argc] = sp;
6778   }
6779   ustack[3+argc] = 0;
6780 
6781   ustack[0] = 0xffffffff;  // fake return PC
6782   ustack[1] = argc;
6783   ustack[2] = sp - (argc+1)*4;  // argv pointer
6784 
6785   sp -= (3+argc+1) * 4;
6786   if(copyout(pgdir, sp, ustack, (3+argc+1)*4) < 0)
6787     goto bad;
6788 
6789   // Save program name for debugging.
6790   for(last=s=path; *s; s++)
6791     if(*s == '/')
6792       last = s+1;
6793   safestrcpy(curproc->name, last, sizeof(curproc->name));
6794 
6795   // Commit to the user image.
6796   oldpgdir = curproc->pgdir;
6797   curproc->pgdir = pgdir;
6798   curproc->sz = sz;
6799   curproc->tf->eip = elf.entry;  // main
6800   curproc->tf->esp = sp;
6801   switchuvm(curproc);
6802   freevm(oldpgdir);
6803   return 0;
6804 
6805  bad:
6806   if(pgdir)
6807     freevm(pgdir);
6808   if(ip){
6809     iunlockput(ip);
6810     end_op();
6811   }
6812   return -1;
6813 }
6814 
6815 
6816 
6817 
6818 
6819 
6820 
6821 
6822 
6823 
6824 
6825 
6826 
6827 
6828 
6829 
6830 
6831 
6832 
6833 
6834 
6835 
6836 
6837 
6838 
6839 
6840 
6841 
6842 
6843 
6844 
6845 
6846 
6847 
6848 
6849 
