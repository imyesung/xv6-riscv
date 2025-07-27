#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/param.h"
#include "kernel/pstat.h"
#include "user/user.h"

// Configuration for oversubscription test
#define NWORKERS_PER_GROUP 3  // Number of workers per ticket group
#define NGROUPS 3            // Number of different ticket groups
#define NWORKERS (NWORKERS_PER_GROUP * NGROUPS)  // Total number of workers
#define NNOISE 4             // Reduced noise processes
#define NROUNDS 20           // Number of measurement rounds
#define INTERVAL 300         // Increased measurement interval
#define WORKLOOP 100000000   // Increased work iterations

static void
cpu_work(void) {
    volatile int i, j = 0;
    for(i = 0; i < WORKLOOP; i++) {
        j += i * i * i;
        j %= 1000;
    }
}

// Add abs() function implementation
static int
abs(int x) {
    return x < 0 ? -x : x;
}

static void
display_distribution(int round) {
    struct pstat ps;
    
    if(getpinfo(&ps) < 0) {
        printf("ERROR: getpinfo failed\n");
        return;
    }

    // Track ticks for each worker group (excluding noise processes)
    int group30 = 0, group20 = 0, group10 = 0;
    int total = 0;
    
    // Count ticks only for worker processes (tickets > 1)
    for(int i = 0; i < NPROC; i++) {
        if(ps.inuse[i] && ps.tickets[i] > 1) {
            total += ps.ticks[i];
            if(ps.tickets[i] == 30)      group30 += ps.ticks[i];
            else if(ps.tickets[i] == 20) group20 += ps.ticks[i];
            else if(ps.tickets[i] == 10) group10 += ps.ticks[i];
        }
    }
    
    if(total == 0) {
        printf("Round %d: Total ticks is still 0\n", round);
        return;
    }

    // Calculate percentages
    int pct30 = (group30 * 100) / total;
    int pct20 = (group20 * 100) / total;
    int pct10 = (group10 * 100) / total;

    // Display results
    printf("\n--- Round %d ---\n", round);
    printf("30 tickets: %d ticks (%d%%)\n", group30, pct30);
    printf("20 tickets: %d ticks (%d%%)\n", group20, pct20);
    printf("10 tickets: %d ticks (%d%%)\n", group10, pct10);
    printf("Total ticks: %d\n", total);
    printf("Distribution: %d%% : %d%% : %d%%\n", pct30, pct20, pct10);
    printf("Expected: 50%% : 33%% : 17%%\n");
    
    // Visualize distribution
    printf("\nGraph (|==== real, ---- expected):\n");

    // 30 tickets graph
    printf("30t: |");
    for(int i = 0; i < 20; i++) {
        if(i < pct30/5) printf("=");
        else printf(" ");
    }
    printf("| %d%%", pct30);
    printf("\n     |");
    for(int i = 0; i < 20; i++) {
        if(i < 10) printf("-");  // 50% = 10 bars
        else printf(" ");
    }
    printf("| 50%%\n");
    
    // 20 tickets graph
    printf("20t: |");
    for(int i = 0; i < 20; i++) {
        if(i < pct20/5) printf("=");
        else printf(" ");
    }
    printf("| %d%%", pct20);
    printf("\n     |");
    for(int i = 0; i < 20; i++) {
        if(i < 7) printf("-");  // 33% = 6.6 bars
        else printf(" ");
    }
    printf("| 33%%\n");
    
    // 10 tickets graph
    printf("10t: |");
    for(int i = 0; i < 20; i++) {
        if(i < pct10/5) printf("=");
        else printf(" ");
    }
    printf("| %d%%", pct10);
    printf("\n     |");
    for(int i = 0; i < 20; i++) {
        if(i < 3) printf("-");  // 17% = 3.3 bars
        else printf(" ");
    }
    printf("| 17%%\n");
}

int
main(int argc, char *argv[]) {
    int wpid[NWORKERS];   // Worker process IDs
    int npid[NNOISE];     // Noise process IDs
    int tickets[NGROUPS] = {30, 20, 10};  // 3:2:1 ratio

    printf("=== LOTTERY SCHEDULER OVERSUBSCRIPTION TEST ===\n");
    printf("Workers per group: %d\n", NWORKERS_PER_GROUP);
    printf("Groups: %d (30:20:10 tickets)\n", NGROUPS);
    printf("Total workers: %d\n", NWORKERS);
    printf("Total group tickets: %d (90:60:30, total 180)\n", 
           NWORKERS_PER_GROUP * (30 + 20 + 10));
    printf("Noise processes: %d (1 ticket each, total %d)\n", NNOISE, NNOISE);
    printf("Total tickets in system: %d\n", (NWORKERS_PER_GROUP * (30 + 20 + 10)) + NNOISE);
    printf("Noise ratio: %.1f%%\n", 
           (NNOISE * 100.0)/((NWORKERS_PER_GROUP * (30 + 20 + 10)) + NNOISE));
    printf("Expected ratio - 50%% : 33%% : 17%%\n\n");

    settickets(1);  // Parent process gets minimal tickets

    // Create worker processes for each group
    int worker_idx = 0;
    for(int group = 0; group < NGROUPS; group++) {
        for(int i = 0; i < NWORKERS_PER_GROUP; i++) {
            wpid[worker_idx] = fork();
            if(wpid[worker_idx] == 0) {
                settickets(tickets[group]);
                while(1) cpu_work();
            }
            worker_idx++;
        }
    }

    // Create noise processes (1 ticket each)
    for(int i = 0; i < NNOISE; i++) {
        npid[i] = fork();
        if(npid[i] == 0) {
            settickets(1);
            while(1) cpu_work();
        }
    }

    sleep(5);  // Allow processes to start

    // Monitor distribution for multiple rounds
    for(int round = 1; round <= NROUNDS; round++) {
        display_distribution(round);
        sleep(3);
    }

    // Show final results
    printf("\n=== Test Completed ===\n");
    struct pstat ps;
    if(getpinfo(&ps) == 0) {
        int g30 = 0, g20 = 0, g10 = 0, tot = 0;
        for(int i = 0; i < NPROC; i++) {
            if(ps.inuse[i] && ps.tickets[i] > 1) {
                tot += ps.ticks[i];
                if(ps.tickets[i] == 30)      g30 += ps.ticks[i];
                else if(ps.tickets[i] == 20) g20 += ps.ticks[i];
                else if(ps.tickets[i] == 10) g10 += ps.ticks[i];
            }
        }
        printf("Final Groups: 30-ticket:%d  20-ticket:%d  10-ticket:%d\n",
               g30, g20, g10);
        printf("Percentages: %d%% : %d%% : %d%%\n",
               g30*100/tot, g20*100/tot, g10*100/tot);
        
        // Calculate convergence
        int delta = abs((g30*100/tot) - 50) + 
                   abs((g20*100/tot) - 33) + 
                   abs((g10*100/tot) - 17);
        printf("Final convergence delta: %d%%\n", delta);
        
        if(delta < 15)
            printf("SUCCESS: Good convergence to expected ratios!\n");
        else
            printf("WARNING: Distribution shows high deviation\n");
    }

    // Cleanup all processes
    for(int i = 0; i < NWORKERS; i++) {
        kill(wpid[i]);
    }
    for(int i = 0; i < NNOISE; i++) {
        kill(npid[i]);
    }
    while(wait(0) != -1);  // Wait for all children
    exit(0);
}