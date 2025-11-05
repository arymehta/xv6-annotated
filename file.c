//
// File descriptors
//

#include "types.h"
#include "defs.h"
#include "param.h"
#include "fs.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "file.h"

struct devsw devsw[NDEV];
struct {
  struct spinlock lock;
  // struct file file[NFILE];
  struct file *freefilelist;
} ftable;


void
add_chunks_to_freefilelist()
{
    struct file *f;
    struct file *trav;
    char *pg;

    pg = kalloc();
    if(pg == 0)
    {
        cprintf("kalloc: error\n");
        return;
    }

    int files_per_page = 4096 / sizeof(struct file);
    cprintf("Files per page %d\n", files_per_page);
    trav = (struct file *) pg;

    acquire(&ftable.lock);
    for(int i = 0; i < files_per_page; i++)
    {
        f = &(trav[i]);
        f->type = FD_NONE;
        f->ref = 0;
        f->readable = 0;
        f->writable = 0;
        f->pipe = 0;
        f->ip = 0;
        f->off = 0;
        f->next = (i == files_per_page - 1) ? ftable.freefilelist : &trav[i + 1];
    }
    ftable.freefilelist = &(trav[0]);
    /*
    int tot = 0;
    for(tot = 0, trav = ftable.freefilelist; trav ; tot++) trav = trav->next;
    cprintf("Total: %d\n", tot);
    */
    release(&ftable.lock);
    return;
}
void
fileinit(void)
{
    cprintf("in file init\n");
    initlock(&ftable.lock, "ftable");
    ftable.freefilelist = 0;
}

// Allocate a file structure.
struct file*
filealloc(void)
{
   // cprintf("in file alloc");
    struct file *f;
    acquire(&ftable.lock);

    // if there is no current files allocated, add chunks
    if(ftable.freefilelist == 0)
    {
        release(&ftable.lock);
        add_chunks_to_freefilelist();
        acquire(&ftable.lock);
    }

    f = ftable.freefilelist;
    while(f)
    {
        if(f->ref == 0)
        {
            f->ref = 1;
            release(&ftable.lock);
            return f;
        }
        f = f->next;
    }

    // if reaches here, means that there is no free file(all refs are 1)
    // grow the list and find again
    add_chunks_to_freefilelist();
    filealloc();
    release(&ftable.lock);
}

// Increment ref count for file f.
struct file*
filedup(struct file *f)
{
  acquire(&ftable.lock);
  if(f->ref < 1)
    panic("filedup");
  f->ref++;
  release(&ftable.lock);
  return f;
}

// Close file f.  (Decrement ref count, close when reaches 0.)
void
fileclose(struct file *f)
{
  struct file ff;

  acquire(&ftable.lock);
  if(f->ref < 1)
    panic("fileclose");
  if(--f->ref > 0){
    release(&ftable.lock);
    return;
  }
  ff = *f;
  f->ref = 0;
  f->type = FD_NONE;
  release(&ftable.lock);

  if(ff.type == FD_PIPE)
    pipeclose(ff.pipe, ff.writable);
  else if(ff.type == FD_INODE){
    begin_op();
    iput(ff.ip);
    end_op();
  }
}

// Get metadata about file f.
int
filestat(struct file *f, struct stat *st)
{
  if(f->type == FD_INODE){
    ilock(f->ip);
    stati(f->ip, st);
    iunlock(f->ip);
    return 0;
  }
  return -1;
}

// Read from file f.
int
fileread(struct file *f, char *addr, int n)
{
  int r;

  if(f->readable == 0)
    return -1;
  if(f->type == FD_PIPE)
    return piperead(f->pipe, addr, n);
  if(f->type == FD_INODE){
    ilock(f->ip);
    if((r = readi(f->ip, addr, f->off, n)) > 0)
      f->off += r;
    iunlock(f->ip);
    return r;
  }
  panic("fileread");
}

//PAGEBREAK!
// Write to file f.
int
filewrite(struct file *f, char *addr, int n)
{
  int r;

  if(f->writable == 0)
    return -1;
  if(f->type == FD_PIPE)
    return pipewrite(f->pipe, addr, n);
  if(f->type == FD_INODE){
    // write a few blocks at a time to avoid exceeding
    // the maximum log transaction size, including
    // i-node, indirect block, allocation blocks,
    // and 2 blocks of slop for non-aligned writes.
    // this really belongs lower down, since writei()
    // might be writing a device like the console.
    int max = ((MAXOPBLOCKS-1-1-2) / 2) * 512;
    int i = 0;
    while(i < n){
      int n1 = n - i;
      if(n1 > max)
        n1 = max;

      begin_op();
      ilock(f->ip);
      if ((r = writei(f->ip, addr + i, f->off, n1)) > 0)
        f->off += r;
      iunlock(f->ip);
      end_op();

      if(r < 0)
        break;
      if(r != n1)
        panic("short filewrite");
      i += r;
    }
    return i == n ? n : -1;
  }
  panic("filewrite");
}

