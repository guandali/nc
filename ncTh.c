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

int kflag, lflag, vflag, rflag, pflag, wflag, opt;
int count = 0;
char* source_port;
int timeout;
#define PORT "3490"
#define BACKLOG 10
#define MAXDATASIZE 100

int local_listen(char* hostname,char* port, struct addrinfo);
int client_connect(char* hostname,char* port, struct addrinfo);
void* readWrite(int new_fd);

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

    int sockfd, new_fd;  // listen on sock_fd, new connection on new_fd
    struct addrinfo hints, *servinfo, *p;
    struct sockaddr_storage their_addr; // connector's address information
    socklen_t sin_size;
    struct sigaction sa;
    int yes=1;
    char s[INET_ADDRSTRLEN];

    ///////////////////////// handling arguments ////////////////////////////

    
    char * hostname = NULL;
    char* port ;
    
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
                source_port = optarg;
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
    port = argv[optind];

    if((kflag && !lflag) || (lflag && pflag) || (rflag && !lflag) || (lflag && (hostname != NULL)) || (!lflag && (hostname == NULL)) || (port == NULL)) {
        usage("");
        exit(EXIT_FAILURE);
    }

    
    //////////////////////////////////////////////////////////////////////////////////////

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    // hints.ai_flags = AI_PASSIVE; // use my IP
    if(lflag){
        local_listen(hostname,port,hints);
    }
    else{
        client_connect( hostname,port,  hints);
    }
        
    return 0;
}

void handle_timeout(int sig) {
    exit(0);
}

void* readWrite(int new_fd) {
    int pid;
    if(!(pid = fork())) {
        while (1) {
            if (wflag && !lflag) {
                signal(SIGALRM, handle_timeout);
                alarm(timeout);
            }

            //int numbytes;
            char str[512] = "";
            int len = scanf("%s",str);
            str[strlen(str)] = '\n';
            str[strlen(str)] = '\0';
            int id = 0;
            if ((id = send(new_fd, str, strlen(str), 0)) == -1) {
                close(new_fd);
                exit(0);
            } else if (id == 0) {
                close(new_fd);
                exit(0);
            }
            if (count < 0 && !lflag) {
                close(new_fd);
                exit(0);
            }
        }
    }
    
    while (1) {
        if (wflag && !lflag) {
            signal(SIGALRM, handle_timeout);
            alarm(timeout);
        }
        int numbytes;
        char buf[512];
        if ((numbytes = recv(new_fd, buf, MAXDATASIZE-1, 0)) == -1) {
            count--;
            perror("recv");
            if (count == 0 && !kflag) {
                exit(0);
            }
            kill(pid, SIGKILL);
            return 0;
        }
        else if (numbytes == 0) {
            count--;
            if (count == 0 && !kflag) {
                exit(0);
            }
            kill(pid, SIGKILL);
            return 0;

        }
        buf[numbytes] = '\0';
        printf("%s",buf);
                
    }

}

int client_connect(char* hostname,char* port, struct addrinfo hints){
    int rv;
    int numbytes;
    char buf[100];
    int sockfd;
    struct addrinfo *servinfo, *p;
    char s[INET_ADDRSTRLEN];
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;


    if ((rv = getaddrinfo(hostname, port, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                             p->ai_protocol)) == -1) {
            perror("client: socket");
            continue;
        }
        
        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("client: connect");
            continue;
        }
        
        break;
    }

    if (p == NULL) {
        fprintf(stderr, "client: failed to connect\n");
        return 2;
    }

    if (pflag) {
		struct sockaddr_in addrForSourcePort;
		addrForSourcePort.sin_family =AF_INET;
		addrForSourcePort.sin_port = htons(atoi(source_port));
		addrForSourcePort.sin_addr.s_addr = INADDR_ANY;
		bind(sockfd, (struct sockaddr*)&addrForSourcePort, sizeof addrForSourcePort);
	}  
    
    inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),
              s, sizeof s);
    printf("client: connecting to %s\n", s);
    
    freeaddrinfo(servinfo); // all done with this structure
    
    pthread_t thread_id;
    pthread_create( &thread_id , NULL ,  readWrite , sockfd);
    
    while (1) {
    
    }
    
    close(sockfd);
    
    return 0;
    
}

int local_listen(char*hostname,char*port, struct addrinfo hints){
    int sockfd;  // listen on sock_fd, new connection on new_fd
    struct addrinfo  *servinfo, *p;
    struct sockaddr_storage their_addr; // connector's address information
    socklen_t sin_size;
    struct sigaction sa;
    int yes=1;
    char s[INET_ADDRSTRLEN];
    int rv;
    hints.ai_flags = AI_PASSIVE;
    if ((rv = getaddrinfo(NULL, port, &hints, &servinfo)) != 0) {
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
        
        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
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
    
         // main accept() loop
    
    int max = 1;
    if (rflag)
        max = 10;
    while (1) {

        while(count < max) {
            int new_fd;
            sin_size = sizeof their_addr;
            new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
            count++;
            if (new_fd == -1) {
                perror("accept");
                continue;
            }
            
            inet_ntop(their_addr.ss_family,
                      get_in_addr((struct sockaddr *)&their_addr),
                      s, sizeof s);
            printf("server: got connection from %s\n", s);

            pthread_t thread_id;
            pthread_create( &thread_id , NULL ,  readWrite , new_fd);
    
            if (vflag) {
                if (lflag) {
                    printf("Connection from %s %s port [%s/%s] succeeded!\n",
                           s, port, "udp" , "tcp",
                           servinfo ? p : "*");
                    fprintf(stderr,"Connection from %s %s port [%s/%s] succeeded!\n",
                            s, port, "udp" , "tcp",
                            servinfo ? p : "*");
                }
                else {
                    printf("Connection to %s %s port [%s/%s] succeeded!\n",
                           hostname, port, "udp" , "tcp",
                           servinfo ? p : "*");
                    fprintf(stderr, "Connection to %s %s port [%s/%s] succeeded!\n",
                            hostname, port, "udp" , "tcp",
                            servinfo ? p :"*");
                    
                }
            }
        }
    }
    close(sockfd);  // close
    return 0;            
}

