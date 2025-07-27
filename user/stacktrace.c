#include "kernel/types.h"
#include "user/user.h"

void c3(int lvl) {
  uint64 addrs[16];
  int depth = stacktrace(addrs, 16);
  printf("=== Stack Trace (depth: %d) ===\n", depth);
  for(int i = 0; i < depth; i++) {
    printf("  #%d: 0x%lx\n", i, addrs[i]);
  }
  printf("===============================\n");
}

void c2(int l) { c3(l); }
void c1(int l) { c2(l); }

int main() {
  printf("STACKTRACER!\n");
  c1(0);
  exit(0);
}