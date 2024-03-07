#include <stdio.h>
#include <stdlib.h>
#include <string.h>   
#include <pthread.h>
#include <sys/sem.h>
#include <unistd.h> //fork()
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <signal.h> //SIGINT

#define MAX_DATA 256
#define SEM_KEY 890819
#define SEM_MODE 0666 /* rw(owner)-rw(group)-rw(other) permission */

int sem, server_fd, stop=0;
int total = 0;
int P(int);
int V(int);
void *client_handler(void*);
void user_close(int);
int total_amount = 0;

int main(int argc, char *argv[]) {

    int client_fd; 
    struct sockaddr_in server_addr, client_addr; 
    socklen_t client_len = sizeof(client_addr); 
    int yes = 1;
    
    signal(SIGINT, user_close);

    //get fd,SOCK_STREAM use TCP
    server_fd = socket(PF_INET, SOCK_STREAM, 0); 

    memset(&server_addr, 0, sizeof(server_addr)); // Initialization structure
    server_addr.sin_family = AF_INET;  // Set address family
    server_addr.sin_port = htons((unsigned short)atoi(argv[1]));  // Set port
    server_addr.sin_addr.s_addr = INADDR_ANY;  // Set address
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));
    bind(server_fd,(struct sockaddr *)&server_addr,sizeof(struct sockaddr)); // Convert when bind                
    listen(server_fd, 10); // maximum number of connection requests that can be handled simultaneously (10)
    
    // Semaphore init
    sem = semget(IPC_PRIVATE, 1, IPC_CREAT | IPC_EXCL | SEM_MODE);
    if (sem < 0){
        fprintf(stderr,"%s: creation of semaphore %d failed: %s\n", argv[0], SEM_KEY, strerror(errno));
        exit(1);
    }
    printf("Semaphore %d created\n", SEM_KEY);
    /* set semaphore (s[0]) value to initial value (val) */
    if(semctl(sem, 0, SETVAL, 1) < 0){
        fprintf(stderr,"%s: Unable to initialize semaphore: %s\n",argv[0], strerror(errno));
        exit(0);
    }
   

    while(!stop) {
        pthread_t thread;
        client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);

        int ret = pthread_create(&thread, NULL, client_handler, &client_fd);
        if (ret != 0) {
            fprintf(stderr, "pthread_create failed: %s\n", strerror(ret));
            exit(1);  
        }
        pthread_detach(thread);
    }

    semctl(sem, 0 , IPC_RMID);
    close(server_fd);
    return 0;
}

/* P (): lock, returns 0 if OK; -1 if there was a problem */
int P(int s){
    struct sembuf sop; /* the operation parameters */
    sop.sem_num = 0; /* access the 1st (and only) sem in the array */
    sop.sem_op = -1; /* wait..*/
    sop.sem_flg = 0; /* no special options needed */
    if (semop (s, &sop, 1) < 0) {
        fprintf(stderr,"P(): semop failed: %s\n",strerror(errno));
        return -1;
    } 
    else {
        return 0;
    }
}
/* V(): unlock, returns 0 if OK; -1 if there was a problem */
int V(int s){
    struct sembuf sop; /* the operation parameters */
    sop.sem_num = 0; /* the 1st (and only) sem in the array */
    sop.sem_op = 1; /* signal */
    sop.sem_flg = 0; /* no special options needed */
    if (semop(s, &sop, 1) < 0) {
        fprintf(stderr,"V(): semop failed: %s\n",strerror(errno));
        return -1;
    } else {
        return 0;
    }
}

void *client_handler(void* arg) {
    int client_fd = *(int*)arg;
    char rec[256];
    int recvBytes = 1;

    while (1){
        recvBytes = recv(client_fd, rec, 256, 0);
        if(recvBytes==0){
            break;
        }
        P(sem);
        if (rec[0] == 'D'){
        total_amount += atoi(rec + 2);
        printf("After deposit: %d\n", total_amount);
        }
        else if (rec[0] == 'W'){
        total_amount -= atoi(rec + 2);
        printf("After withdraw: %d\n", total_amount); 
        }
        memset(rec, 0, 256);
        V(sem); 
        
    }
    close(client_fd);
    pthread_exit(NULL);
    return NULL;
}

void user_close(int signum)
{
  stop = 1;
  printf("\nClosing server_fd\n");
  close(server_fd);
  if(semctl(sem, 0, IPC_RMID, 0) < 0){
    perror("Error removing sem\n");
    exit(0);
  }
  printf("Remove semaphore\n");
  exit(signum);
}
