#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <semaphore.h>

#define MAX_CHILD 5
#define BUF_SIZE 100

int num_child = 0;
pid_t child_pids[MAX_CHILD]; //array to store children pid
int pipe_fds[MAX_CHILD][2]; //2D array to store pipe files

sem_t sem;


//Display message of signal Ctrl+C and terminate the program
void sigint_handler(int a) {
    printf("\nInterrupt signal received. Exiting...\n");
    sem_destroy(&sem);
    exit(0);
}


//Ask user to enter message to pipe to the connected child process
void parent_process() {
    sleep(1);
    fflush(stdin);
    printf("Enter a message to send to children: ");
    char message[200];
    if(fgets(message, sizeof(message), stdin) == NULL) {
      printf("\nError reading input.\n");
      exit(1); }

    sem_wait(&sem);

    for(int i=0; i<MAX_CHILD; i++) {
      printf("Parent writing to child %d\n", i+1);
      write(pipe_fds[i][1], message, sizeof(message));
      sleep(1);
    }
    sem_post(&sem);
}

void child_process(int id) {
  sem_wait(&sem);
  printf("Child %d reading from pipe\n", id+1);

  char message[200];
  read(pipe_fds[id][0], message, sizeof(message));
  printf("Child %d  received message: %s", id+1, message);
  sleep(1);
  sem_post(&sem);
}

int main() {
  signal(SIGINT, sigint_handler);
  sem_init(&sem, 0, 1);

  for (int i=0; i<MAX_CHILD; i++) {
    if (pipe(pipe_fds[i]) == -1) {
      printf("Error creating pipe for children %d\n", i+1);
      exit(1);
    }

    pid_t pid = fork();
    if (pid == -1) {
      printf("Error forking child %d\n", i+1);
      exit(1);
    } else if (pid == 0) {
      child_process(i);
      exit(0);
    } else {
      child_pids[i] = pid;
      num_child++;
      if (i == MAX_CHILD-1) {
        parent_process();
      }
    }
  }

  sem_destroy(&sem);
  return 0;

}






