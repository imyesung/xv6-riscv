#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "riscv.h"
#include "spinlock.h"
#include "proc.h"
#include "defs.h"

// Generate pseudo-random number for xv6
static unsigned int next = 1;
static struct spinlock rand_lock;

int
rand(void)
{
  acquire(&rand_lock);
  next = next * 1103515245 + 12345;
  int result = (unsigned int)(next / 65536) % 32768;
  release(&rand_lock);
  return result;
}

void
randinit(void)
{
  initlock(&rand_lock, "rand");
}