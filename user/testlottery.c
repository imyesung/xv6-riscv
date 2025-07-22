#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/param.h"
#include "kernel/pstat.h"
#include "user/user.h"

#define NCHILDREN 6
#define WORKLOOP  50000000

static void
cpu_work(void) {
    volatile int i, j = 0;
    for(i = 0; i < WORKLOOP; i++) {
        j += i * i * i;
        j %= 1000;
    }
}

static void
display_distribution(int round) {
    struct pstat ps;
    
    if(getpinfo(&ps) < 0) {
        printf("ERROR: getpinfo failed\n");
        return;
    }

    int group30 = 0, group20 = 0, group10 = 0;
    int total = 0;


    for(int i = 0; i < NPROC; i++) {
        if(ps.inuse[i] && ps.tickets[i] > 1) {
            total += ps.ticks[i];
            if(ps.tickets[i] == 30)    group30 += ps.ticks[i];
            else if(ps.tickets[i] == 20) group20 += ps.ticks[i];
            else if(ps.tickets[i] == 10) group10 += ps.ticks[i];
        }
    }
    
    if(total == 0) {
        printf("Round %d: Total ticks is still 0\n", round);
        return;
    }

    int pct30 = (group30 * 100) / total;
    int pct20 = (group20 * 100) / total;
    int pct10 = (group10 * 100) / total;

    // 간단한 출력 (ANSI 코드 없이)
    printf("\n--- Round %d ---\n", round);
    printf("30 tickets: %d ticks (%d%%)\n", group30, pct30);
    printf("20 tickets: %d ticks (%d%%)\n", group20, pct20);
    printf("10 tickets: %d ticks (%d%%)\n", group10, pct10);
    printf("Total ticks: %d\n", total);
    printf("Distribution: %d%% : %d%% : %d%%\n", pct30, pct20, pct10);
    printf("Expected: 50%% : 33%% : 17%%\n");
    
    printf("Graph:\n");
    printf("30 tickets: ");
    for(int i = 0; i < pct30/5; i++) printf("#");
    printf("\n");
    
    printf("20 tickets: ");
    for(int i = 0; i < pct20/5; i++) printf("#");
    printf("\n");
    
    printf("10 tickets: ");
    for(int i = 0; i < pct10/5; i++) printf("#");
    printf("\n");
}

int
main(int argc, char *argv[]) {
    int pid[NCHILDREN];
    int tickets[NCHILDREN] = {30, 20, 10, 30, 20, 10};

    printf("=== LOTTERY SCHEDULER TEST ===\n");
    printf("Creating 6 processes: 30*2, 20*2, 10*2 tickets\n");
    printf("Testing 3 groups with 2 processes each\n");
    printf("Press Ctrl+C to stop...\n\n");

    settickets(1);  // give parent minimal tickets

    // spawn worker processes
    for(int i = 0; i < NCHILDREN; i++){
        pid[i] = fork();
        if(pid[i] == 0) {
            settickets(tickets[i]);
            while(1) cpu_work();
        }
    }
    sleep(3);  // allow workers to start

    // live monitoring
    for(int round = 1; round <= 20; round++) {
        display_distribution(round);
        sleep(2);
    }

    // final summary
    printf("\n=== Test Completed ===\n");
    struct pstat ps;
    if(getpinfo(&ps) == 0) {
        int g30 = 0, g20 = 0, g10 = 0, tot = 0;
        for(int i = 0; i < NPROC; i++){
            if(ps.inuse[i] && ps.tickets[i] > 1){
                tot += ps.ticks[i];
                if(ps.tickets[i] == 30)    g30 += ps.ticks[i];
                else if(ps.tickets[i] == 20) g20 += ps.ticks[i];
                else if(ps.tickets[i] == 10) g10 += ps.ticks[i];
            }
        }
        printf("Final Groups: 30-ticket:%d  20-ticket:%d  10-ticket:%d\n",
               g30, g20, g10);
        printf("Percentages: %d%% : %d%% : %d%%\n",
               g30*100/tot, g20*100/tot, g10*100/tot);
        if(g30 > g20 && g20 > g10)
            printf("SUCCESS: Lottery scheduler working!\n");
        else
            printf("WARNING: Unexpected results\n");
    }

    // cleanup
    for(int i = 0; i < NCHILDREN; i++){
        kill(pid[i]);
        wait(0);
    }
    exit(0);
}