#include <stdlib.h> // exit()
#include <stdio.h>
#include <signal.h> // signal()
#include <unistd.h> // close()
#include <string.h> //memset()
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/wait.h>
#include <errno.h>

int sockfd, connfd, yes = 1;

struct MenuItem {
    char *name;
    int price;
    int num; //Record the number of meals ordered
};

struct Restaurant {
    char *name;
    int distance;
    struct MenuItem menu[2];
};

void user_close(int signum) {
    close(sockfd);
    printf("\n");
    exit(0);
}

int main(int argc, char *argv[]) {	

    struct sockaddr_in server_addr, clint_addr;
    socklen_t clint_len = sizeof(clint_addr); 

    if (argc!=2){
        printf("Please usage: %s <port>\n", argv[0]);
        exit(-1);
    }
    
    //get fd,SOCK_STREAM use TCP
    sockfd = socket(PF_INET, SOCK_STREAM, 0); 
    
    if (sockfd == -1){
        printf("Fail to create a socket.");
    }

    memset(&server_addr, 0, sizeof(server_addr)); // Initialization structure
    server_addr.sin_family = AF_INET;  // Set address family
    server_addr.sin_port = htons((unsigned short)atoi(argv[1]));  // Set port
    server_addr.sin_addr.s_addr = INADDR_ANY;  // Set address
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));
    bind(sockfd,(struct sockaddr *)&server_addr,sizeof(struct sockaddr)); // Convert when bind                
    listen(sockfd, 10); // maximum number of connection requests that can be handled simultaneously (10)

    signal(SIGINT, user_close);

    while (1){
        connfd = accept(sockfd, (struct sockaddr *)&clint_addr, &clint_len);
        int totalAmount = 0;
        int current_restaurant_index = -1;  
        char confirmation_msg[256] = {0};

        struct Restaurant restaurants[3] = {
            {"Dessert shop", 3, {{"cookie", 60, 0}, {"cake", 80, 0}}},
            {"Beverage shop", 5, {{"tea", 40, 0}, {"boba", 70, 0}}},
            {"Diner", 8, {{"fried-rice", 120, 0}, {"Egg-drop-soup", 50, 0}}}
        };

        while(1) {
            //Receive input from clint
            char buffer[256]= {0};
            int recvBytes = recv(connfd,buffer,sizeof(buffer),0);

            if (strncmp(buffer, "shop list", 10) == 0) {
                send(connfd, "Dessert shop:3km\n- cookie:$60|cake:$80\nBeverage shop:5km\n- tea:$40|boba:$70\nDiner:8km\n- fried-rice:$120|Egg-drop-soup:$50\n", 256, 0);
            }

            else if (strncmp(buffer, "order", 5) == 0){
                char item[256];
                int quantity; 
                sscanf(buffer, "order %s %d", item, &quantity);
   
                if (current_restaurant_index == -1) {
                    // The first order, set the current restaurant
                    for (int i = 0; i < 3; ++i) {
                        for (int j = 0; j < 2; ++j) {
                            if (strcmp(item, restaurants[i].menu[j].name) == 0) {
                                current_restaurant_index = i;
                                break;
                            }
                        }
                    }
                }

                if (current_restaurant_index != -1 && strcmp(item,restaurants[current_restaurant_index].menu[0].name) == 0 ||
                        strcmp(item,restaurants[current_restaurant_index].menu[1].name) == 0) {
                        for (int j = 0; j < 2; ++j) {
                            if (strcmp(item, restaurants[current_restaurant_index].menu[j].name) == 0) {
                                restaurants[current_restaurant_index].menu[j].num += quantity;
                                totalAmount += restaurants[current_restaurant_index].menu[j].price * quantity;
                                break;
                            }
                        }
                        if (restaurants[current_restaurant_index].menu[0].num && restaurants[current_restaurant_index].menu[1].num)
                            snprintf(confirmation_msg, sizeof(confirmation_msg), "%s %d|%s %d\n", restaurants[current_restaurant_index].menu[0].name, restaurants[current_restaurant_index].menu[0].num, restaurants[current_restaurant_index].menu[1].name, restaurants[current_restaurant_index].menu[1].num);
                        else if (restaurants[current_restaurant_index].menu[1].num)
                            snprintf(confirmation_msg, sizeof(confirmation_msg), "%s %d\n", restaurants[current_restaurant_index].menu[1].name, restaurants[current_restaurant_index].menu[1].num);
                        else
                            snprintf(confirmation_msg, sizeof(confirmation_msg), "%s %d\n", restaurants[current_restaurant_index].menu[0].name, restaurants[current_restaurant_index].menu[0].num);

                        send(connfd, confirmation_msg, 256, 0);  
                }else {
                    send(connfd, confirmation_msg, 256, 0);
                }

            }

            else if (strcmp(buffer, "confirm") == 0) {
                // Handle confirm command
                char orderd[256];

                if(current_restaurant_index == -1){
                    send(connfd, "Please order some meals\n", 256, 0);
                }

                else{
                    send(connfd, "Please wait a few minutes...\n", 256, 0);
                    sleep(restaurants[current_restaurant_index].distance);
        
                    sprintf(orderd, "Delivery has arrived and you need to pay %d$\n", totalAmount);
                    send(connfd, orderd, 256, 0);

                    close(connfd);
                    break;
                }
            } 

            else if (strcmp(buffer, "cancel") == 0) {
                // Handle cancel command
                send(connfd, "Order canceled. Goodbye!\n", 256, 0);
                close(connfd);
                break;
            }
        }
    }
    close(sockfd);
	return 0;
}




