#include <stdlib.h>
#include <sys/stat.h>
#include <stdio.h>
#include <dirent.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include<math.h>
#include <semaphore.h>
#include <pthread.h>
#include <errno.h>
#include <fcntl.h>



int main(int argc, char * argv[]){
    //se supone que esto lo voy a usar para el semaforo
    int slavesAmmount = argc/20; //esto me da la parte entera pero de abajo...
    slavesAmmount++;
    int counter = slavesAmmount;
    int pid;
    int done=0;

    sem_t sem_file;
    sem_t sem_buffer;

    sem_init(&sem_buffer, 1/*porque es compartido entre procesos*/, 1);
    sem_init(&sem_file, 1,1);

    if( mkfifo("buffer pipe", 0777 /*permisos*/) == -1){
        if(errno != EEXIST){
            printf("Could not create the named pipe!\n");
        }
        return -1;
    };

    if( mkfifo("file pipe", 0777 /*permisos*/) == -1){
        if(errno != EEXIST){
            printf("Could not create the named pipe!\n");
        }
        return -1;
    };

    //int mkfifo (const char *__path,d __mode_t __mode)

    //Creo mi cantidad de esclavos.
    for(int i = 0; i < slavesAmmount && !done; i++){
        pid = fork();
        if((pid) == -1){
            //printh("Error: No se pudo crear un esclavo!\n",3); Queremos imprimir esto en el stderr...
            return -1;
        }
        else if(pid == 0){
            done = 1;
        }
    }

    if(!pid){
        while(counter != 0){

            int fp = open("file pipe", O_RDONLY);
            int bp = open("buffer pipe", O_WRONLY);
            sem_wait(&sem_file);
            //read..... 
            sem_post(&sem_file);

            //hago lo del minisat...

            sem_wait(&sem_buffer);
            //write...
            sem_post(&sem_buffer);
        }

    }

    

    int fp = open("file pipe", O_WRONLY);
    int bp = open("buffer pipe", O_RDONLY);

    while(argc != 0 && counter != 0){
        if(argc >= 2){
            write(/*....*/); 
            argc -= 2;
        }
        else{
            write(/*....*/);
            argc--;
        }
        
    }

    while(argc != 0){
        int fd = open("buffer",O_RDONLY);
        if(fd == -1){
            return 1;
        }

        if(read(fd, &x, sizeof(x)) == -1){
            return 2;
        }
        //guardo x en mi buffer
        argc--;
        close(fd);
    }

    for(int i = 0; i < slavesAmmount; i++){
        wait(0);
    }

    return 0;
}

