#include <stdlib.h> // exit()
#include <stdio.h>
#include <signal.h> // signal()
#include <unistd.h> // close()
#include <string.h> //memset()
#include <pthread.h>
#include <semaphore.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/wait.h>
#include <errno.h>

#define SEM_KEY 890819
#define SEM_MODE 0666 /* rw(owner)-rw(group)-rw(other) permission */

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

sem_t sem, sem2; //1 for time, 2 for total
int sockfd, Revenue = 0, total_people = 0;
int first = 0, second = 0;
void *handle_client_command(void *arg);
void *delivery_timer_thread(void* arg);
void user_close(int signum);

int main(int argc, char *argv[]) {	
    int connfd, yes = 1;
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

    // Semaphore init
    if (sem_init(&sem, 0, 1) != 0){
        perror("sem_init");
        exit(EXIT_FAILURE);
    }

    if (sem_init(&sem2, 0, 1) != 0){
        perror("sem_init");
        exit(EXIT_FAILURE);
    }

    pthread_t timer_thread;
    if(pthread_create(&timer_thread, NULL, delivery_timer_thread, NULL) != 0) {
        fprintf(stderr, "Failed to create timer thread\n");
        exit(1);
    }

    signal(SIGINT, user_close);
    while (1){
        pthread_t thread;
        connfd = accept(sockfd, (struct sockaddr *)&clint_addr, &clint_len);
        if (connfd < 0) {
            perror("accept failed");
            continue;
        }

        int* new_connfd = malloc(sizeof(int));
        if (new_connfd == NULL) {
            perror("malloc failed");
            close(connfd);
            continue;
        }

        *new_connfd = connfd;

        int ret = pthread_create(&thread, NULL, handle_client_command, (void *)new_connfd);
        // printf("(thread:%ld, connfd: %d)\n",thread, connfd);
        if (ret != 0) {
            fprintf(stderr, "pthread_create failed: %s\n", strerror(ret));
            exit(1);  
        }

    }
    close(sockfd);
	return 0;
}

void *handle_client_command(void* arg) {
    int client_fd = *(int*)arg;
    free(arg);
    int totalAmount = 0;
    char buffer[256]= {0};
    int current_restaurant_index = -1;  
    char confirmation_msg[256] = {0};

    struct Restaurant client_order[3] = {
        {"Dessert shop", 3, {{"cookie", 60, 0}, {"cake", 80, 0}}},
        {"Beverage shop", 5, {{"tea", 40, 0}, {"boba", 70, 0}}},
        {"Diner", 8, {{"fried-rice", 120, 0}, {"Egg-drop-soup", 50, 0}}}
    };
    
    while(1) {
        
        //Receive input from clint
        int recvBytes = recv(client_fd,buffer,sizeof(buffer),0);
        if (recvBytes > 0) {
            printf("(%d,%s)", client_fd, buffer);
        }

        if (strncmp(buffer, "shop list", 10) == 0) {
            send(client_fd, "Dessert shop:3km\n- cookie:$60|cake:$80\nBeverage shop:5km\n- tea:$40|boba:$70\nDiner:8km\n- fried-rice:$120|Egg-drop-soup:$50\n", 256, 0);
            printf("Dessert shop:3km\n- cookie:$60|cake:$80\nBeverage shop:5km\n- tea:$40|boba:$70\nDiner:8km\n- fried-rice:$120|Egg-drop-soup:$50\n");
        } 
        else if (strncmp(buffer, "order", 5) == 0) {
            char item[256];
            int quantity; 
            sscanf(buffer, "order %s %d", item, &quantity);
    
            if (current_restaurant_index == -1) {
                for (int i = 0; i < 3; ++i) {
                    for (int j = 0; j < 2; ++j) {
                        if (strcmp(item, client_order[i].menu[j].name) == 0) {
                            current_restaurant_index = i;
                            break;
                        }
                    }
                }
            }

            if (current_restaurant_index != -1 && strcmp(item, client_order[current_restaurant_index].menu[0].name) == 0 ||
                    strcmp(item, client_order[current_restaurant_index].menu[1].name) == 0) {
                for (int j = 0; j < 2; ++j) {
                    if (strcmp(item, client_order[current_restaurant_index].menu[j].name) == 0) {
                        client_order[current_restaurant_index].menu[j].num += quantity;
                        totalAmount += client_order[current_restaurant_index].menu[j].price * quantity;
                        break;
                    }
                }
                if (client_order[current_restaurant_index].menu[0].num && client_order[current_restaurant_index].menu[1].num)
                    snprintf(confirmation_msg, sizeof(confirmation_msg), "%s %d|%s %d", client_order[current_restaurant_index].menu[0].name, client_order[current_restaurant_index].menu[0].num, client_order[current_restaurant_index].menu[1].name, client_order[current_restaurant_index].menu[1].num);
                else if (client_order[current_restaurant_index].menu[0].num)
                    snprintf(confirmation_msg, sizeof(confirmation_msg), "%s %d", client_order[current_restaurant_index].menu[0].name, client_order[current_restaurant_index].menu[0].num);
                else
                    snprintf(confirmation_msg, sizeof(confirmation_msg), "%s %d", client_order[current_restaurant_index].menu[1].name, client_order[current_restaurant_index].menu[1].num);

                send(client_fd, confirmation_msg, 256, 0); 
                printf("to client(%d,%s)\n",client_fd, confirmation_msg); 
            } 
            else {
                send(client_fd, confirmation_msg, 256, 0);
                printf("to client(%d,%s)\n",client_fd, confirmation_msg); 
            }
        } 
        else if (strcmp(buffer, "confirm") == 0) {
            // Handle confirm command
            char orderd[256];
            if (current_restaurant_index == -1) {
                send(client_fd, "Please order some meals", 256, 0);
                printf("(%d,Please order some meals)\n",client_fd);

            } 
            else {
                sem_wait(&sem);
                int min_wait = (first <= second) ? first : second;
                
                if(min_wait + client_order[current_restaurant_index].distance > 30){

                    send(client_fd, "Your delivery will take a long time, do you want to wait?", 256, 0);
                    printf("(%d,Your delivery will take a long time, do you want to wait?)\n",client_fd);
                    fflush(stdout);


                    char response[256] = {0};
                    int recvBytes = recv(client_fd, response, sizeof(response), 0);
                    if (strcmp(response, "Yes") == 0) {
                        printf("I want(%d,%s)",client_fd,response);
                    }
                    else if (strcmp(response, "No") == 0) {
                        printf("(%d,%s)",client_fd,response);
                        printf("I dont %d\n",min_wait + client_order[current_restaurant_index].distance);
                        fflush(stdout);
                        sem_post(&sem);
                        break;
                    }
                }
                else {
                    send(client_fd, "Please wait a few minutes...", 256, 0);
                    printf("(%d,Please wait a few minutes...)\n",client_fd);
                    fflush(stdout);
                }

                min_wait += client_order[current_restaurant_index].distance;
                if (first <= second) {
                    first = min_wait;
                } 
                else {
                    second = min_wait;
                }
                sem_post(&sem);

                sleep(min_wait);

                sprintf(orderd, "Delivery has arrived and you need to pay %d$", totalAmount);
                send(client_fd, orderd, 256, 0);

                sem_wait(&sem2);
                Revenue += totalAmount;
                total_people ++;
                sem_post(&sem2);

                printf("(%d,Delivery has arrived and you need to pay %d$)\n",client_fd, totalAmount);
                fflush(stdout);
                break;
            }
        } 
        else if (strcmp(buffer, "cancel") == 0) {
            // Your cancel handling logic here
            break;
        } 
        
    }
    close(client_fd);
}

void *delivery_timer_thread(void* arg) {
    while(1) {
        sleep(1);  // 每秒醒來一次
        sem_wait(&sem);
        if (first > 0) {
            first--;
            printf("1:%d\n",first);
            fflush(stdout);
        }
        if (second > 0) {
            second--;
            printf("2:%d\n",second);
            fflush(stdout);
        }
        sem_post(&sem);
    }
    return NULL;
}

void user_close(int signum) {
    printf("\n");
    printf("customer: %d\n", total_people);
    printf("income: %d$\n", Revenue);
    FILE *file = fopen("result.txt", "w");
    if (file != NULL) {
        fprintf(file, "customer: %d\n", total_people);
        fprintf(file, "income: %d$\n", Revenue);
        fclose(file);
    } else {
        fprintf(stderr, "Failed to create result.txt\n");
    }

    close(sockfd);
    printf("Remove semaphore\n");
    sem_destroy(&sem);
    sem_destroy(&sem2);
    exit(0);
}