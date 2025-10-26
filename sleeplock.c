// Sleeping locks

#include "types.h"
#include "defs.h"
#include "param.h"
#include "x86.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "spinlock.h"
#include "sleeplock.h"

void
initsleeplock(struct sleeplock *lk, char *name)
{
  initlock(&lk->lk, "sleep lock");
  lk->name = name;
  lk->locked = 0;
  lk->pid = 0;
}

/* Aryan:
     * - if lock is already taken, call sleep and pass the spinlock, to be released in sleep
     * - the while loop may cause MULTIPLE process to come out of sleep
     * - This causes a RACE for the lk->locked
*/
void
acquiresleep(struct sleeplock *lk)
{
  acquire(&lk->lk); // Aryan: acquiring the spinlock that is protecting the sleeplock, INTERRUPTS DISABLED
  while (lk->locked) {
    sleep(lk, &lk->lk);
    // Aryan: sleeplocks have interrupts enabled!, if T1 wins race... releases spinlock at the end
    // if T2 loses race, goes to sleep, sleep calls release() on &lk->lk, meaning enabled again
    // Therefore never use sleeplock if an interrupt handler requires a lock, only spinlock eg ideintr
  }
  lk->locked = 1;
  lk->pid = myproc()->pid;
  release(&lk->lk); // aryan: interrupts are enabled again
}

void
releasesleep(struct sleeplock *lk)
{
  acquire(&lk->lk);
  lk->locked = 0;
  lk->pid = 0;
  wakeup(lk);
  release(&lk->lk);
}

int
holdingsleep(struct sleeplock *lk)
{
  int r;

  acquire(&lk->lk);
  r = lk->locked && (lk->pid == myproc()->pid);
  release(&lk->lk);
  return r;
}



