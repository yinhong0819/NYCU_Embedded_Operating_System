#include <stdio.h>  // perror()
#include <stdlib.h> // exit()
#include <fcntl.h>  // open()
#include <unistd.h> // dup2()
#include <signal.h> // signal()
#include <unistd.h> // close()
#include <string.h> //memset()
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/wait.h>

int sockfd, connfd, yes = 1;

void handler(int signum) {
    while (waitpid(-1, NULL, WNOHANG) > 0);
}

void user_close(int signum) {
    close(sockfd);
}

int main(int argc, char *argv[]) {	

    struct sockaddr_in server_addr, clint_addr;
    socklen_t clint_len = sizeof(clint_addr); 
    pid_t child_pid;

    if (argc!=2){
        printf("Please usage: %s <port>\n", argv[0]);
        exit(-1);
    }
    
    //get fd,SOCK_STREAM use TCP
    sockfd = socket(PF_INET, SOCK_STREAM, 0); 
    
    memset(&server_addr, 0, sizeof(server_addr)); // Initialization structure
    server_addr.sin_family = AF_INET;  // Set address family
    server_addr.sin_port = htons((unsigned short)atoi(argv[1]));  // Set port
    server_addr.sin_addr.s_addr = INADDR_ANY;  // Set address
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));
    bind(sockfd,(struct sockaddr *)&server_addr,sizeof(struct sockaddr)); // Convert when bind                
    listen(sockfd, 10); // maximum number of connection requests that can be handled simultaneously (10)
    
    signal(SIGCHLD, handler);
    signal(SIGINT, user_close);

    while (1){
        
        connfd = accept(sockfd, (struct sockaddr *)&clint_addr, &clint_len);

        child_pid = fork();
        if (child_pid >= 0){   /* fork succeeded */
            if (child_pid == 0){ /* fork() returns 0 to the child process */
                dup2(connfd, STDOUT_FILENO);
                close(connfd);
                execlp("sl", "sl", "-l", NULL);
                exit(0);
            }
            else{  /* fork() returns new pid to the parent process */
                printf("Train ID: %d\n", (int)child_pid);
                // printf("Parent ID: %d\n", getpid());
            }
        }
        else { /* fork returns -1 on failure */
            perror("fork"); /* display error message */
            exit(0);
        }
    }

    close(sockfd);
	return 0;
}


