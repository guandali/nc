#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include "commonProto.h"

int main(int argc, char **argv) {

    // This is some sample code feel free to delete it
    // This is the main program for the version of nc that uses Poll.

    int i;

    for (i = 0; i < argc; i++) {
      fprintf(stderr, "Arg %d is: %s\n", i, argv[i]);
      usage(argv[0]);
    }

    return 0;

}
