#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h> //pid_t 
#include <sys/ipc.h>
#include <sys/time.h> //timer
#include <sys/shm.h> // shmget, shmat, shmdt, shmctl
#include <unistd.h>
#include <signal.h>  // sigaction

typedef struct {
    int guess;
    char result[8];
}data;

int shmid;
sig_atomic_t upper, lower = 0, stop = 0;
pid_t game_pid;
void timeHandler(int);

int main(int argc, char *argv[]){

    key_t  key=atoi(argv[1]);
    upper=atoi(argv[2]);
    game_pid=atoi(argv[3]);
    struct itimerval timer;
    struct sigaction sa;

    if (argc!=4)
    {
        printf("Please usage: %s <key> <upper_bound> <pid>\n", argv[0]);
        exit(-1);
    }

    //create share memory
    if ((shmid = shmget(key, sizeof(data), IPC_CREAT | 0666)) < 0)
    {
        perror("shmget");
        exit(-1);
    }

    memset(&sa,0,sizeof(sa));
    sa.sa_handler = timeHandler;
    sigaction (SIGVTALRM, &sa, NULL);

    /* Configure the timer to expire after 1 sec */
    timer.it_value.tv_sec = 1;
    timer.it_value.tv_usec = 0;

    /* Reset the timer back to 1 sec after expired */
    timer.it_interval.tv_sec = 1;
    timer.it_interval.tv_usec = 0;

    /* Start a virtual timer */
    setitimer (ITIMER_VIRTUAL, &timer, NULL);

    while(!stop);

    return 0;
}

void timeHandler(int signum){
    data *shm;
    /* Now we attach the segment to our data space */
    if ((shm = shmat(shmid, NULL, 0)) == (void *) -1){
        perror("shmat");
        exit(-1);
    }

    if (strcmp(shm->result,"bingo")==0){
        stop=1;
        return;
    }
    else if(strcmp(shm->result,"smaller")==0){
        upper=shm->guess;
    }
    else if(strcmp(shm->result,"bigger")==0){
        lower=shm->guess;
    }

    shm->guess=(upper+lower)/2;
    printf("[game] Guess: %d\n",shm->guess);
    
    kill(game_pid,SIGUSR1);
    shmdt(shm);
}