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



int main(int argc, char * argv[]){
    int files = argc-1;
    int slavesAmmount = (files)/20; //esto me da la parte entera pero de abajo...
    slavesAmmount++;
    int * slaves = malloc(sizeof(char)*slavesAmmount);

    int  * pipes[2] = malloc(sizeof(char[2])*slavesAmmount);


    for(int i = 0; i < slavesAmmount; i++){
        if(pipe(pipes[i]) == -1){
            printf("Error creating pipe!\n");
            return -1;
        }; 
        //pipes[i][0] read
        // pipes[i][1] write
    }

    
    
    sem_t * slave_filesems = malloc(sizeof(sem_t)*slavesAmmount);

    for(int i = 0; i<slavesAmmount; i++){
        sem_init(slave_filesems + i*sizeof(sem_t),1,0);
    }




    
    

    int pid;
    // Creo mi cantidad de esclavos.
    for(int i = 0, done = 0; i < slavesAmmount && !done; i++){
        pid = fork();
        if((pid) == -1){
            // printh("Error: No se pudo crear un esclavo!\n",3); Queremos imprimir esto en el stderr...
            return -1;
        }
        else if(pid == 0){
            printf("%d created\n", getpid());
            done = 1;
        }
        else
        {
            slaves[i] = pid;
        }
        
    }



    if(pid == 0){
        

    
    }
    else
    {
        int i,j;
        for(i=0,j = 0; i<slavesAmmount && j < files-1; i++,j+=2){
            write(pipes[i][0], &argv + j*sizeof(char*),sizeof(argv[j]);
            write(pipes[i][0], &argv + (j+1)*sizeof(char*),sizeof(argv[j+1]));
            sem_post(slave_filesems + i*sizeof(sem_t));
            sem_post(slave_filesems + i*sizeof(sem_t));
        }
        if(i < slavesAmmount){
            write(pipes[i][0], &argv + i*sizeof(char*), sizeof(argv[j]));
            sem_post(slave_filesems + i*sizeof(sem_t));
            j++;
        }
        //thread para ir recibiendo buffers... CREO QUE SE HACE CON SELECT
        while(j < files){
            i++;
            if(i == slavesAmmount){
                i = 0;
            }
            write(pipes[i][0], &argv + j*sizeof(char*),sizeof(argv[j]));
            sem_post(slave_filesems + i*sizeof(sem_t));
            j++;
        }


    }

}




