#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "riscv.h"
#include "spinlock.h"
#include "proc.h"
#include "defs.h"

// Generate pseudo-random number for xv6
static unsigned int next = 1;

int
rand(void)
{
  next = next * 1103515245 + 12345;
  return (unsigned int)(next / 65536) % 32768;
}

void
srand(unsigned int seed)
{
  next = seed;
}