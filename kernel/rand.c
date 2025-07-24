#include "types.h"
#include "param.h"
#include "riscv.h"
#include "spinlock.h"
#include "defs.h"

// Generate pseudo-random number for xv6
static unsigned int next[NCPU];
static struct spinlock rand_lock[NCPU];

int
rand(void)
{
  int cpu = cpuid();
  acquire(&rand_lock[cpu]);
  next[cpu] = next[cpu] * 1103515245 + 12345;
  int result = (unsigned int)(next[cpu] / 65536) % 32768;
  release(&rand_lock[cpu]);
  return result;
}

void
randinit(void)
{
  for(int i = 0; i < NCPU; i++) {
    initlock(&rand_lock[i], "rand");
    // Set the seed differently using timer value and CPU id
    next[i] = (uint64)r_time() ^ (i * 1234567);
  }
}