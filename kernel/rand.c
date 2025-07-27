#include "types.h"
#include "param.h"
#include "riscv.h"
#include "defs.h"

// PCG32 state per CPU
static uint64 pcg_state[NCPU];
static uint64 pcg_inc[NCPU];   // stream selection (must be odd)

static inline uint32 
rotl32(uint32 x, unsigned r) {
    return (x << r) | (x >> (32 - r));
}

static inline uint32 
pcg32_step(int cpu) {
    uint64 oldstate = pcg_state[cpu];
    pcg_state[cpu] = oldstate * 6364136223846793005ULL + pcg_inc[cpu];
    
    // Output function (XSH RR): xorshift high 32 bits and rotate right
    uint32 xorshifted = (uint32)(((oldstate >> 18u) ^ oldstate) >> 27u);
    uint32 rot = (uint32)(oldstate >> 59u);
    return rotl32(xorshifted, rot);
}

int
rand(void)
{
    // Keep xv6 compatibility: return 15-bit value (0..32767)
    return (int)(pcg32_step(cpuid()) >> 17);
}

void
randinit(void)
{
    // Initialize each CPU's RNG with unique stream
    for(int i = 0; i < NCPU; i++) {
        uint64 seed = (uint64)r_time() ^ ((uint64)i * 0x9E3779B97F4A7C15ULL);
        
        // Generate initial state using splitmix64
        uint64 z = (seed + 0x9E3779B97F4A7C15ULL);
        z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ULL;
        z = (z ^ (z >> 27)) * 0x94D049BB133111EBULL;
        pcg_state[i] = z ^ (z >> 31);
        
        // Generate stream ID (must be odd)
        pcg_inc[i] = ((uint64)i << 1) | 1ULL;
    }
}