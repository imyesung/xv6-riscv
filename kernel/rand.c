#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "riscv.h"
#include "spinlock.h"
#include "proc.h"
#include "defs.h"

// Simple Linear Congruential Generator (LCG)
// Used by many standard libraries, good enough for lottery scheduler
static unsigned long next = 1;

// Initialize random number generator
void
randinit(void)
{
  // Use some system state as initial seed
  next = 1103515245;  // Standard LCG seed
}

// Generate pseudo-random number
int
rand(void)
{
  // LCG formula: next = (a * next + c) % m
  // Common values: a=1103515245, c=12345, m=2^31
  next = next * 1103515245 + 12345;
  return (unsigned int)(next / 65536) % 32768;
}