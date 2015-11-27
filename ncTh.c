#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include "commonProto.h"

#include <unistd.h>

///////////////////////
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>




int l_flag;
#define PORT "3490"
#define BACKLOG 10
#define MAXDATASIZE 100
void* sending(int new_fd);
void* receiving(int new_fd);

void sigchld_handler(int s)
{
    // waitpid() might overwrite errno, so we save and restore it:
    int saved_errno = errno;
    
    while(waitpid(-1, NULL, WNOHANG) > 0);
    
    errno = saved_errno;
}

void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}



int main(int argc, char **argv) {

    // This is some sample code feel free to delete it
    // This is the main program for the thread version of nc
    
//    int i;
//
//    for (i = 0; i < argc; i++) {
//      fprintf(stderr, "Arg %d is: %s\n", i, argv[i]);
//      usage(argv[0]);
//    }
//
//    return 0;
    int sockfd, new_fd;  // listen on sock_fd, new connection on new_fd
    struct addrinfo hints, *servinfo, *p;
    struct sockaddr_storage their_addr; // connector's address information
    socklen_t sin_size;
    struct sigaction sa;
    int yes=1;
    char s[INET6_ADDRSTRLEN];
    int rv;
    
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; // use my IP
    
    if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }
    
    // loop through all the results and bind to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                             p->ai_protocol)) == -1) {
            perror("server: socket");
            continue;
        }
        
        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
                       sizeof(int)) == -1) {
            perror("setsockopt");
            exit(1);
        }
        
        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("server: bind");
            continue;
        }
        
        break;
    }
    
    freeaddrinfo(servinfo); // all done with this structure
    
    if (p == NULL)  {
        fprintf(stderr, "server: failed to bind\n");
        exit(1);
    }
    
    if (listen(sockfd, BACKLOG) == -1) {
        perror("listen");
        exit(1);
    }
    
    sa.sa_handler = sigchld_handler; // reap all dead processes
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("sigaction");
        exit(1);
    }
    
    printf("server: waiting for connections...\n");
    
    while(1) {  // main accept() loop
        sin_size = sizeof their_addr;
        new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
        if (new_fd == -1) {
            perror("accept");
            continue;
        }
        
        inet_ntop(their_addr.ss_family,
                  get_in_addr((struct sockaddr *)&their_addr),
                  s, sizeof s);
        printf("server: got connection from %s\n", s);
//        
//        if (!fork()) { // this is the child process
//            close(sockfd); // child doesn't need the listener
////            if (send(new_fd, "Hello, world!\n", 13, 0) == -1)
//                perror("send");
//            //close(new_fd);
//            char str[100];
//            scanf("%s", str);
//            send(new_fd, str, strlen(str), 0);
//            exit(0);
//        }
//        //close(new_fd);  // parent doesn't need this
//        while (1) {
//            int numbytes;
//            char buf[512];
//            int count;
//            char str[100];
//            scanf("%s", str);
//            send(new_fd, str, strlen(str), 0);
////            count = recv(new_fd,buf,512,0);
////            if(count>0){
////                printf("%s",buf);
////            
////            }
//            if ((numbytes = recv(new_fd, buf, MAXDATASIZE-1, 0)) == -1) {
//                perror("recv");
//
//            }
//            else if (numbytes == 0) {
//                //stop = 1;
//                return 0;
//            
//            }
//            buf[numbytes] = '\0';
//            printf("client: received '%s'\n",buf);
//            
//
////           
//            
//        }
//         exit(0);
//        sending(new_fd);
//        receiving(new_fd);
// creating thread for 
        pthread_t threads[2];
        int t_1 =  pthread_create(&threads[0],NULL,sending,new_fd);
        int t_2 =  pthread_create(&threads[1],NULL,receiving,new_fd);
        
    }
    
    return 0;
    
    

}


//    for (i = 0; i < argc; i++) {
//      fprintf(stderr, "Arg %d is: %s\n", i, argv[i]);
//      usage(argv[0]);
//    }

///////////////////////// handling arguments ////////////////////////////
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

//////////////////////////////////////////////////////////////////////////////////////

void* sending(int new_fd){
    while (1) {
        int numbytes;
        char str[512];
        scanf("%s",str);
        send(new_fd, str, strlen(str), 0);
    }
    return 0;
}

void* receiving(int new_fd){
    while (1) {
        int numbytes;
        char buf[512];
        if ((numbytes = recv(new_fd, buf, MAXDATASIZE-1, 0)) == -1) {
            perror("recv");

        }
        else if (numbytes == 0) {
            //stop = 1;
            return 0;

        }
        buf[numbytes] = '\0';
        printf("%s",buf);
        
        
    }
    return 0;
}
