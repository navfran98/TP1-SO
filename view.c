#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <stdio.h>
#include <dirent.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <math.h>
#include <semaphore.h>
#include <pthread.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/mman.h>
#include <sys/select.h>

#define BUFFER_SIZE 12000
#define SPECIAL_SYMBOL '$'

static void run_and_check_error(int error, char message[], int retval);

int main(int argc, char * argv[]){
    int files_to_process;
    if(argc == 2){
        files_to_process = atoi(argv[1]);
    }
    else if(argc == 1){
        scanf("%d", &files_to_process);
        printf("Received %d files_to_process\n", files_to_process);
    }
    else{
        perror("Error! Must receive an argument!\n");
        exit(EXIT_FAILURE);

    }
    int buffer_fd;
    run_and_check_error(buffer_fd = shm_open("/shared_memory", O_RDWR  , 0666), "Error when creating shm", -1); /* create s.m object*/   
    run_and_check_error(ftruncate(buffer_fd, BUFFER_SIZE*sizeof(char)), "Error when setting size of shm", -1);                                 /* resize memory object */
    char * buffer = mmap(NULL, BUFFER_SIZE*sizeof(char), PROT_WRITE, MAP_SHARED, buffer_fd, 0);
    sem_t * access_shm = sem_open("access_shm", O_RDWR);
    sem_t * file_counter = sem_open("files_ready_to_print", O_RDWR);

    int i = 0;

    while(files_to_process){

        sem_wait(file_counter);
        sem_wait(access_shm);


        for (; buffer[i]!='\0'; i++){
            printf("%c", buffer[i]);
        }
        printf("\n");

        sem_post(access_shm);
        files_to_process--;
    }
}

 

static void run_and_check_error(int error, char message[], int retval) {
    if(error == retval) {
        dprintf(STDERR_FILENO, "%s\n", message);
        printf("%d\n", errno);
        exit(EXIT_FAILURE);
    }
}

