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

int*filesToRecieve;

void * thread_routine(void * arg)
{
    while(*filesToRecieve > 0){
        int bp = open("buffer pipe", O_RDONLY);
        printf("BUFFER LEIDO: %d\n", *filesToRecieve + 1);
            //if(read(bp, &x, sizeof(x)) == -1){
            //    return 2;
            //
            // guardo x en mi buffe
        close(bp);
    }

}




int main(int argc, char * argv[]){
    int slavesAmmount = (argc-1)/20; //esto me da la parte entera pero de abajo...
    slavesAmmount++;


    int fd = shm_open("/filesRemaining", O_CREAT | O_RDWR  , 0600); /* create s.m object*/
    ftruncate(fd, sizeof(int)); 
    int * files_remaining = mmap(NULL, sizeof(int), PROT_WRITE, MAP_SHARED, fd, 0);
    *files_remaining = argc-1;

    int fd2 = shm_open("/filesRecieved", O_CREAT | O_RDWR  , 0600); /* create s.m object*/
    ftruncate(fd2, sizeof(int)); 
    filesToRecieve = mmap(NULL, sizeof(int), PROT_WRITE, MAP_SHARED, fd2, 0);
    *filesToRecieve = argc-1;

    int fd3 = shm_open("/slavesCounter", O_CREAT | O_RDWR  , 0600); /* create s.m object*/
    ftruncate(fd3, sizeof(int)); 
    int * slavesCounter = mmap(NULL, sizeof(int), PROT_WRITE, MAP_SHARED, fd3, 0);
    *slavesCounter = slavesAmmount;

    

    printf("%d\n", *files_remaining);

    int * slaves = malloc(sizeof(char)*argc);

    int pid;
    int done=0;

    sem_t sem_file;
    sem_t sem_buffer;
    
    sem_init(&sem_buffer, 1/*porque es compartido entre procesos*/, 1);
    sem_init(&sem_file, 1,1 /*cantidad de puntos q empieza */);

    if( mkfifo("buffer pipe", 0777 /*permisos*/) == -1){
        if(errno != EEXIST){
            printf("Could not create the named pipe!\n");
            return -1;
        }
        
    };    

    if( mkfifo("file pipe", 0777 /*permisos*/) == -1){
        if(errno != EEXIST){
            printf("Could not create the named pipe!\n");
             return -1;
        }
       
    };
    
    


    // Creo mi cantidad de esclavos.
    for(int i = 0; i < slavesAmmount && !done; i++){
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
        int files_processed = 0;

        while(1){
            sem_wait(&sem_file);
            if(*files_remaining != 0){
                int fp = open("file pipe", O_RDONLY);
                printf("Esclavo %d leyendo\n", getpid());
                files_processed++;
                *files_remaining -= 1;
                close(fp);
            }
            sem_post(&sem_file);
     
            //if(fork()==0){
            //Acá pense en hacer una cola con todas las cosas procesadas y bue..
                // execv(/*minisat*/);

            //}
            
            sem_wait(&sem_buffer); //uuuuu
            int bp = open("buffer pipe", O_WRONLY);
            printf("Esclavo %d escribiendo\n", getpid());
            *filesToRecieve -= 1;
                //write(bp, &x, sizeof(int));
            close(bp);
            sem_post(&sem_buffer);
            
        }
    }
    else
    {
        pthread_t buffer_reciever;
        pthread_create(&buffer_reciever, NULL, thread_routine,  NULL/*arg &var*/); // 0 si se crea bien
        
        while(*files_remaining){
            int fp = open("file pipe", O_WRONLY);
            close(fp);
        }

       pthread_join(buffer_reciever,NULL);


       for(int i = 0; i < slavesAmmount; i++){
            printf("Matando el proceso %d\n", slaves[i]);
            kill(slaves[i],SIGKILL);
        }


    }

}

