#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include "commonProto.h"
#include <unistd.h>

int main(int argc, char **argv) {

    // This is some sample code feel free to delete it
    // This is the main program for the thread version of nc

    int i;

//    for (i = 0; i < argc; i++) {
//      fprintf(stderr, "Arg %d is: %s\n", i, argv[i]);
//      usage(argv[0]);
//    }

    int kflag, lflag, vflag, rflag, pflag, wflag, opt;
    int source_port;
    int timeout;
    char * hostname = NULL;
    int port = 0;

    while ((opt = getopt(argc, argv, "klvrp:w:")) != -1) {

        switch (opt) {
        case 'k':
        	kflag = 1;
        	break;
        case 'l':
        	lflag = 1;
        	break;
        case 'v':
        	vflag = 1;
        	break;
        case 'r':
        	rflag = 1;
        	break;
        case 'p':
        	pflag = 1;
        	source_port = atoi(optarg);
        	break;
        case 'w':
        	wflag = 1;
        	timeout = atoi(optarg);
        	break;
        default:
        	usage("");
        	exit(EXIT_FAILURE);
        }
    }

    if (optind + 2 == argc) {
    	hostname = argv[optind++];
    } 
    port = atoi(argv[optind]);

    if (kflag) {
    	printf("%s\n", "K");
    }
    if (lflag) {
    	printf("%s\n", "L");
    }
    if (vflag) {
    	printf("%s\n", "V");
    }
    if (rflag) {
    	printf("%s\n", "R");
    }
    if (pflag) {
    	printf("%s\n", "P");
    }
    if (wflag) {
    	printf("%s\n", "W");
    }
    printf("%i\n", source_port);
    printf("%i\n", timeout);
    printf("%s\n", hostname);
    printf("%i\n", port);
    if((kflag && !lflag) || (lflag && pflag) || (rflag && !lflag) || (lflag && (hostname != NULL)) || (!lflag && (hostname == NULL)) || (port == 0)) {
    	usage("");
        exit(EXIT_FAILURE);
    }

    return 0;

}
