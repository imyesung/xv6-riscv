#include "kernel/types.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  int before, after;
  char buf[64];  // Smaller buffer - don't waste memory
  
  before = getreadcount();
  printf("reads: %d\n", before);
  
  int fd = open("README", 0);
  if(fd < 0) {
    printf("open failed\n");
    exit(1);
  }
  
  // Just do a few reads - simple test
  read(fd, buf, 10);
  read(fd, buf, 10);
  read(fd, buf, 10);
  close(fd);
  
  after = getreadcount();
  printf("reads: %d (diff: %d)\n", after, after - before);
  
  exit(0);
}