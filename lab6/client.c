#include <stdio.h>  
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h> // socket(), connect()
#include <arpa/inet.h> // inet_addr()
   
#define MAX_DATA 256
   
int main(int argc, char *argv[]) {

    if(argc != 6) {
        printf("Usage: %s <IP> <port> <op> <amount> <times>\n", argv[0]);
        exit(1); 
    }
   
    int sockfd;
    struct sockaddr_in server_addr; 
    char buffer[MAX_DATA];
    
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    server_addr.sin_family = AF_INET;    
    server_addr.sin_addr.s_addr = inet_addr(argv[1]);   
    server_addr.sin_port = htons(atoi(argv[2]));

    connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr));
      
    char *op = argv[3]; 
    int amount = atoi(argv[4]);
    int times = atoi(argv[5]);

    for (int i = 0; i < times; i++) {
        
        if (strcmp(op, "deposit") == 0) {
            sprintf(buffer, "D %d", amount);
        }
        else if (strcmp(op, "withdraw") == 0) { 
            sprintf(buffer, "W %d", amount);  
        }
        
        send(sockfd, buffer, MAX_DATA, 0);
        usleep(1000);  //The unit is microseconds
    }

    close(sockfd);
    return 0;
}