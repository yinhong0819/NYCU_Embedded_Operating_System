#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <server_ip> <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(argv[2]));
    if (inet_pton(AF_INET, argv[1], &server_addr.sin_addr) <= 0) {
        perror("Invalid address or address not supported");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("Connection failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    char buffer[256];
    char recv_buffer[256];
    memset(buffer, 0, sizeof(buffer));

    // Send "shop list" command
    strcpy(buffer, "shop list");
    send(sockfd, buffer, 256, 0);

    // Receive and print the response
    memset(recv_buffer, 0, sizeof(buffer));
    recv(sockfd, recv_buffer, 256, 0);
    printf("Response from server:\n%s\n", recv_buffer);

    // Send "order cake 1" command
    strcpy(buffer, "order cake 1");
    send(sockfd, buffer, 256, 0);
   
    // Receive and print the response
    memset(recv_buffer, 0, sizeof(recv_buffer));
    recv(sockfd, recv_buffer, sizeof(recv_buffer), 0);
    printf("Response from server:\n%s\n", recv_buffer);

    // Send "order cookie 3" command
    strcpy(buffer, "order cookie 3");
    send(sockfd, buffer, 256, 0);

    // Receive and print the response
    memset(recv_buffer, 0, sizeof(buffer));
    recv(sockfd, recv_buffer, sizeof(buffer), 0);
    printf("Response from server:\n%s\n", recv_buffer);

    // Send "order tea 1" command
    strcpy(buffer, "order tea 1");
    send(sockfd, buffer, 256, 0);

    // Receive and print the response
    memset(recv_buffer, 0, sizeof(buffer));
    recv(sockfd, recv_buffer, sizeof(buffer), 0);
    printf("Response from server:\n%s\n", recv_buffer);

    // Send "confirm" command
    strcpy(buffer, "confirm");
    send(sockfd, buffer, 256, 0);

    // Receive and print the response
    memset(recv_buffer, 0, sizeof(buffer));
    recv(sockfd, recv_buffer, sizeof(buffer), 0);
    printf("Response from server:\n%s\n", recv_buffer);

    // Send "cancel" command
    strcpy(buffer, "cancel");
    send(sockfd, buffer, 256, 0);

    // Receive and print the response
    memset(recv_buffer, 0, sizeof(buffer));
    recv(sockfd, recv_buffer, sizeof(buffer), 0);
    printf("Response from server:\n%s\n", recv_buffer);

    close(sockfd);
    return 0;
}
