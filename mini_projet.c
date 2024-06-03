#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/wait.h>

#define NUM_CHILDREN 4

sem_t *sem_start;


void child_task(int signum) {
    printf("Le Fils %d a reçu le signal numero %d\n", getpid(), signum);
    if (signum == SIGUSR1) {
  
        sleep(1);
        printf("Le Fils %d a complété sa tâche.\n", getpid());

        kill(getppid(), SIGUSR2);
    }
}


void parent_confirmation_handler(int signum) {
    printf("Le Parent a reçu le signal de confirmation numero %d\n", signum);
}

void setup_signal_handlers() {
    struct sigaction sa;
    sa.sa_handler = child_task;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGUSR1, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);
}

int main() {
    pid_t pid;
    pid_t child_pids[NUM_CHILDREN];
    sem_start = sem_open("/sem_start", O_CREAT | O_EXCL, 0644, 0);

    for (int i = 0; i < NUM_CHILDREN; i++) {
        if ((pid = fork()) == 0) {
            
            setup_signal_handlers();
            sem_wait(sem_start);
            pause(); 
            exit(0);
        } else {
            
            child_pids[i] = pid;
        }
    }


    for (int i = 0; i < NUM_CHILDREN; i++) {
        sem_post(sem_start);
    }


    sleep(1);
    for (int i = 0; i < NUM_CHILDREN; i++) {
        kill(child_pids[i], SIGUSR1);
    }


    struct sigaction sa;
    sa.sa_handler = parent_confirmation_handler;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGUSR2, &sa, NULL);

    for (int i = 0; i < NUM_CHILDREN; i++) {
        wait(NULL); 
    }

    sem_close(sem_start);
    sem_unlink("/sem_start");

    return 0;
}
