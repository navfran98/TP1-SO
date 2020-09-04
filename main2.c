#include <stdlib.h>
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
#include "queuelib.h"

#define FILE_LENGTH 100 
#define SPECIAL_SYMBOL '$'
#define SPECIAL_SYMBOL_STRING "$"
#define INITIAL_FILES_TO_GIVE_OUT 2

void create_pipes(int ** file_pipes, int ** buffer_pipes, int slavesAmmount){
    int height=slavesAmmount, width=2, i, j;

    file_pipes = malloc(height * sizeof(int *));
    buffer_pipes = malloc(height * sizeof(int *));

    for(i = 0; i < width; i++){
        file_pipes[i] = malloc(width * sizeof(int));
        buffer_pipes[i] = malloc(width * sizeof(int));
    }

  

    for(int i = 0; i < slavesAmmount; i++){
        if(pipe(file_pipes[i]) == -1){
            printf("Error creating file pipe!\n");
            return -1;
        }
        if(pipe(buffer_pipes[i]) == -1){
            printf("Error creating buffer pipe!\n");
            return -1;
        }
        //file_pipes[i][0] write
        // file_pipes[i][1] read
    }
}



int main(int argc, char * argv[]){
    int files = argc-1;
    int slavesAmmount = (files)/20; //esto me da la parte entera pero de abajo...
    slavesAmmount++;

    if(argc-1 < INITIAL_FILES_TO_GIVE_OUT){
        printf("jdajkdj");
        return -1;
    }
    int file_index = 0;
    
    int * slaves = malloc(sizeof(char)*slavesAmmount);

    int ** file_pipes;
    int ** buffer_pipes;
    create_pipes(file_pipes, buffer_pipes, slavesAmmount);

    
    sem_t aux_sem;
    sem_init(&aux_sem,1,1);
    sem_t index_sem;
    sem_init(&index_sem,1,0);
    
    sem_t * slave_filesems = malloc(sizeof(sem_t)*slavesAmmount);
    for(int i = 0; i<slavesAmmount; i++){
        sem_init(&(slave_filesems[i]),1,0);
        printf("Creando sem %d\n", i);
    }

    int pid;
    // Creo mi cantidad de esclavos.
    for(int i = 0, done = 0; i < slavesAmmount; i++){
        pid = fork();
        if((pid) == -1){
            // printh("Error: No se pudo crear un esclavo!\n",3); Queremos imprimir esto en el stderr...
            return -1;
        }
        else if(pid == 0){
            printf("%d created\n", getpid());
            run_and_check_error(close(fileno(stdin)),"Error closing STDIN\n", -1);                    
            run_and_check_error(dup(file_pipes[index][0]),"Error dupping file pipe\n", -1);        // new STDIN: read-end of master-to-slave pipe N°i
            run_and_check_error(close(fileno(stdout)),"Error closing STDOUT\n", -1);           
            run_and_check_error(dup(buffer_pipes[index][1]),"Error dupping buffer pipe\n", -1);        // new STDOUT: write-end of slave-to-master pipe N°i
            //CERRAR TODOS LOS PIPES Y EXECVEAR 
            //EXEC;;

        }
        else
        {

            // father_buffer_fd[i] = buffer_pipes[i][0]; para usar el select despues.... pero por ahora no.
            slaves[i] = pid;
            int files = argc-1;
            int file_index, i;
            for(i=0, file_index=0; i < slavesAmmount && file_index <= files-INITIAL_FILES_TO_GIVE_OUT; i++){
                send_files(file_pipes[i][0], INITIAL_FILES_TO_GIVE_OUT, &file_index, argv);
            }
            if(i < slavesAmmount){
                send_files(file_pipes[i][0], INITIAL_FILES_TO_GIVE_OUT - 1, &file_index, argv);
            }
            
        }
    }



    if(pid == 0){
        sem_wait(&aux_sem);
        int index;
        sem_getvalue(&index_sem,&index);
        sem_post(&index_sem);
        sem_post(&aux_sem);
        close(file_pipes[index][0]);

        
    }
    else
    {

    int files = argc-1;
    int file_index, i;
      for(i=0, file_index=0; i < slavesAmmount && file_index <= files-INITIAL_FILES_TO_GIVE_OUT; i++){
          send_files(file_pipes[i][0], INITIAL_FILES_TO_GIVE_OUT, &file_index, argv);
      }
      if(i < slavesAmmount){
          send_files(file_pipes[i][0], INITIAL_FILES_TO_GIVE_OUT - 1, &file_index, argv);
      }

    }

    //FINALMENTE HAY Q CERRAR TODO Y LIBERAR TODO LO USADO.

}

static void send_files(int fd, int filesAmmount,int * file_index, char * argv[]) {
    char * file;
    run_and_check_error((file=calloc(FILE_LENGTH,sizeof(char))),"No more space available for memory allocation\n", NULL);
    
    for(int i = 0; i < filesAmmount; i++) {
        if(argv[*file_index] == NULL){
            strcat(file, argv[*file_index]);
            (*file_index)++;     
            strcat(file, SPECIAL_SYMBOL);  
        }
    }
    
    //dprintf(fd, "%s", file);
    write(fd, file, strlen(file));    
}

static void run_and_check_error(int error, char message[], int retval){
    if(error == retval){
        dprintf(STDERR_FILENO, "%s", message);
        exit(EXIT_FAILURE);
    }
}






