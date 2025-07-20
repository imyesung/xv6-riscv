#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"
#include "pstat.h"

// add extern declaration for proc array
// extern struct proc proc[NPROC];

uint64
sys_exit(void)
{
  int n;
  argint(0, &n);
  exit(n);
  return 0;  // not reached
}

uint64
sys_getpid(void)
{
  return myproc()->pid;
}

uint64
sys_fork(void)
{
  return fork();
}

uint64
sys_wait(void)
{
  uint64 p;
  argaddr(0, &p);
  return wait(p);
}

uint64
sys_sbrk(void)
{
  uint64 addr;
  int n;

  argint(0, &n);
  addr = myproc()->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

uint64
sys_sleep(void)
{
  int n;
  uint ticks0;

  argint(0, &n);
  if(n < 0)
    n = 0;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(killed(myproc())){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

uint64
sys_kill(void)
{
  int pid;

  argint(0, &pid);
  return kill(pid);
}

// return how many clock tick interrupts have occurred
// since start.
uint64
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

extern int read_count;  // Direct access to global counter

uint64
sys_getreadcount(void)
{
  return read_count;  // Simple and direct
}

uint64
sys_stacktrace(void)
{
  uint64 u_buf;
  int max;
  
  argaddr(0, &u_buf);
  argint(1, &max);

  if(max <= 0 || max > 16)
    return -1;
    
  struct proc *p = myproc();
  uint64 fp = (uint64)p->trapframe->s0;
  int depth = 0;
  
  while(fp && depth < max) {
    uint64 ra_addr = fp - 8;
    uint64 ra;
    
    if(copyin(p->pagetable, (char*)&ra, ra_addr, sizeof(ra)) < 0)
      break;
      
    if(copyout(p->pagetable, u_buf + depth * sizeof(uint64), 
               (char*)&ra, sizeof(ra)) < 0)
      return -1;
      
    uint64 prev_fp;
    if(copyin(p->pagetable, (char*)&prev_fp, fp - 16, sizeof(prev_fp)) < 0)
      break;
      
    fp = prev_fp;
    depth++;
  }
  
  return depth;
}

uint64
sys_settickets(void)
{
  int tickets;
  argint(0, &tickets);
  
  if(tickets < 1)
    return -1;
    
  struct proc *p = myproc();
  p->tickets = tickets;
  return 0;
}

uint64
sys_getpinfo(void)
{
  uint64 addr;
  argaddr(0, &addr);
  
  struct pstat ps;
  
  // Initialize the pstat structure
  for(int i = 0; i < NPROC; i++) {
    ps.inuse[i] = 0;
    ps.tickets[i] = 0;
    ps.pid[i] = 0;
    ps.ticks[i] = 0;
  }
  
  // safe access to the proc array
  extern struct proc proc[NPROC];
  struct proc *p;
  
  for(p = proc; p < &proc[NPROC]; p++) {
    acquire(&p->lock);
    int index = p - proc;
    if(p->state != UNUSED) {
      ps.inuse[index] = 1;
      ps.tickets[index] = p->tickets;
      ps.pid[index] = p->pid;
      ps.ticks[index] = p->ticks;
    }
    release(&p->lock);
  }
  
  // Copy to user space
  if(copyout(myproc()->pagetable, addr, (char*)&ps, sizeof(ps)) < 0)
    return -1;
    
  return 0;
}
