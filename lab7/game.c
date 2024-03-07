#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h> // shmget, shmat, shmdt, shmctl
#include <unistd.h>
#include <signal.h>  // sigaction

typedef struct {
    int guess;
    char result[8];
}data;

int answer;
int shmid;
void sigHandler(int);
void user_close(int);

int main(int argc, char *argv[]){

	key_t key = atoi(argv[1]);
	answer = atoi(argv[2]);
	struct sigaction my_action;

  	if (argc != 3){
		printf("Usage: %s <shm key> <answer>\n", argv[0]);
		exit(1);
	}

	printf("PID = %d\n", getpid());
	/* Create the segment */
	if ((shmid = shmget(key, sizeof(data), IPC_CREAT | 0666)) < 0) {
        perror("shmget");
        exit(1);
    }
	// printf("shmid = %d\n",shmid);
	
    memset(&my_action,0,sizeof(my_action));
    my_action.sa_handler = sigHandler;
    sigaction(SIGUSR1, &my_action, NULL);
    signal(SIGINT, user_close);

	while(1);

	return 0;
}

void sigHandler(int signum){
	data *shm;
	/* Now we attach the segment to our data space */
    if ((shm = shmat(shmid, NULL, 0)) == (data *) -1) {
        perror("shmat");
        exit(1);
    }
	printf("[game] Guess %d, ", shm->guess);
	if(answer > shm->guess){
		sprintf(shm->result, "bigger");
	}

	else if(answer < shm->guess){
		sprintf(shm->result, "smaller");
	}

	else{
		sprintf(shm->result, "bingo");
	}

  	printf("%s\n", shm->result);
	shmdt(shm);
}

void user_close(int signum){
	
    int retval = shmctl(shmid, IPC_RMID, NULL);
    if (retval < 0)
    {
        fprintf(stderr, "Server remove share memory failed\n");
        exit(1);
    }
	printf("\nServer destroy the share memory.\n");

	exit(0); //exit main function
}