// This is a simple loop which will print some chars
// It is a useful way to test that the scheduler is working

#include <inttypes.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char **argv) {
    char c;

    if (argc < 2)
        c = '*';
    else
        c = *argv[1];
    // int count = (int)((uintmax_t) strtoumax(*(argv[2]), NULL, 10));
    int count = atoi(argv[2]);

    for (int i = 0; i < count; i++) {
        printf("%c", c);
        fflush(stdout);
        usleep(100000);
    }
}
