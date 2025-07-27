#include "kernel/types.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
    // 1) Initial read count
    uint64 before = getreadcount();
    printf("reads: %lu\n", (unsigned long)before);

    // 2) Open README
    int fd = open("README", 0);
    if (fd < 0) {
        printf("open failed\n");
        exit(1);
    }

    // 3) Perform 3 reads
    char buf[64];
    read(fd, buf, 10);
    read(fd, buf, 10);
    read(fd, buf, 10);
    close(fd);

    // 4) Read count after and difference
    uint64 after = getreadcount();
    printf("reads: %lu (diff: %lu)\n",
           (unsigned long)after,
           (unsigned long)(after - before));

    exit(0);
}