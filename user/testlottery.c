#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/pstat.h"
#include "user/user.h"

#define NFORK 3
#define WORKLOOP 3000000
#define NSAMPLE 15
#define SLEEP_TICKS 10

// CPU-intensive work function
static void
cpu_work(void) {
  volatile int i, j = 0;
  for(i = 0; i < WORKLOOP; i++){
    j += i * i * i;
    j %= 1000;
  }
}

int
main(int argc, char *argv[]) {
  struct pstat ps;
  int pid[NFORK];
  int tickets[NFORK] = {30, 20, 10};

  printf("Lottery Scheduler Test: 3:2:1 ratio\n");
  printf("Expected: A=50%%, B=33%%, C=17%%\n\n");

  // 1) Create child processes - NO printf in children to avoid mixed output
  for(int i = 0; i < NFORK; i++){
    pid[i] = fork();
    if(pid[i] == 0){
      // Child process - silent execution to avoid output mixing
      settickets(tickets[i]);
      while(1) cpu_work();  // Infinite loop until killed
      exit(0);
    } else {
      printf("Created child %d with PID %d (%d tickets)\n", i, pid[i], tickets[i]);
    }
  }

  printf("\nSampling results:\n");

  // 2) Sampling loop with proper printf formatting
  for(int s = 0; s < NSAMPLE; s++){
    sleep(SLEEP_TICKS);

    if(getpinfo(&ps) == 0){
      int t[3] = {0,0,0};
      for(int i = 0; i < NPROC; i++){
        if(ps.inuse[i]){
          for(int k = 0; k < NFORK; k++){
            if(ps.pid[i] == pid[k]){
              t[k] = ps.ticks[i];
            }
          }
        }
      }
      // Use simple integer formatting to avoid printf issues
      printf("Sample ");
      printf("%d", s+1);
      printf(": A=");
      printf("%d", t[0]);
      printf(" B=");
      printf("%d", t[1]);
      printf(" C=");
      printf("%d", t[2]);
      printf("\n");
    }
  }

  // 3) Collect final tick counts before killing children
  struct pstat final_ps;
  int final_ticks[NFORK] = {0, 0, 0};
  
  if(getpinfo(&final_ps) == 0){
    for(int k = 0; k < NFORK; k++){
      for(int j = 0; j < NPROC; j++){
        if(final_ps.inuse[j] && final_ps.pid[j] == pid[k]){
          final_ticks[k] = final_ps.ticks[j];
          break;
        }
      }
    }
  }

  // 4) Kill child processes
  for(int i = 0; i < NFORK; i++){
    kill(pid[i]);
    wait(0);
  }

  // 5) Display results with simple printf calls
  int total = final_ticks[0] + final_ticks[1] + final_ticks[2];
  
  printf("\n=== FINAL RESULTS ===\n");
  printf("Process A: ");
  printf("%d", tickets[0]);
  printf(" tickets, ");
  printf("%d", final_ticks[0]);
  printf(" ticks\n");
  
  printf("Process B: ");
  printf("%d", tickets[1]);
  printf(" tickets, ");
  printf("%d", final_ticks[1]);
  printf(" ticks\n");
  
  printf("Process C: ");
  printf("%d", tickets[2]);
  printf(" tickets, ");
  printf("%d", final_ticks[2]);
  printf(" ticks\n");
  
  printf("Total: ");
  printf("%d", total);
  printf(" ticks\n");

  if(total > 0) {
    int pctA = final_ticks[0] * 100 / total;
    int pctB = final_ticks[1] * 100 / total;
    int pctC = final_ticks[2] * 100 / total;
    
    printf("\nActual percentages:\n");
    printf("A: ");
    printf("%d", pctA);
    printf("%%, B: ");
    printf("%d", pctB);
    printf("%%, C: ");
    printf("%d", pctC);
    printf("%%\n");
    
    // ASCII bar chart
    printf("\nASCII Bar Chart:\n");
    printf("A (");
    printf("%d", pctA);
    printf("%%) ");
    for(int i = 0; i < pctA/2; i++) printf("#");
    printf("\n");
    
    printf("B (");
    printf("%d", pctB);
    printf("%%) ");
    for(int i = 0; i < pctB/2; i++) printf("#");
    printf("\n");
    
    printf("C (");
    printf("%d", pctC);
    printf("%%) ");
    for(int i = 0; i < pctC/2; i++) printf("#");
    printf("\n");
    
    // Success/failure determination
    if(final_ticks[0] > final_ticks[1] && final_ticks[1] > final_ticks[2]) {
      printf("\nSUCCESS: Lottery scheduler working!\n");
    } else {
      printf("\nWARNING: Lottery scheduler not working properly\n");
    }
  }

  exit(0);
}