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

#define SLAVES 3
sem_t sem_file;
sem_t sem_buffer;
sem_t sem_filesRemaining;
sem_t sem_filesReceived;
sem_t sem_slaveCounter;




void * thread_routine(void * arg){
    int bp = open("/tmp/buffer_pipe", O_WRONLY);
    char buf[10];
    while(getFilesToReceive() != 0){

        read(bp,buf,sizeof(buf));
        printf("BUFFER PIPE SAYS = %s",buf);
        sem_wait(&sem_filesReceived);
        *files_to_receive--;
        sem_post(&sem_filesReceived);

    }

    close(bp);
    
};

int main(int argc, char * argv[]){
    int slavesAmmount = (argc-1)/20; //esto me da la parte entera pero de abajo...
    slavesAmmount++;

    int  * myPipes[2] = malloc(sizeof(char)*(argc-1)/20);
    

    int * slaves = malloc(sizeof(char)*argc);

    
    
    sem_init(&sem_buffer, 1/*porque es compartido entre procesos*/, 1);
    sem_init(&sem_file, 1,1 /*cantidad de puntos q empieza */);
    sem_init(&sem_slaveCounter, 1, 1);
    sem_init(&sem_filesReceived, 1,1 );
    sem_init(&sem_filesRemaining, 1,0);

    if( mkfifo("/tmp/buffer_pipe", 0777 /*permisos*/) == -1){
        if(errno != EEXIST){
            printf("Could not create the named pipe!\n");
            return -1;
        }
        
    };    

    if( mkfifo("/tmp/file_pipe", 0777 /*permisos*/) == -1){
        if(errno != EEXIST){
            printf("Could not create the named pipe!\n");
             return -1;
        }
       
    };
    
    

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
        int processed_files = 0;
        int fd = argc-1;
        int fp = open("/tmp/file_pipe", O_RDONLY);
        if( fp == -1){
            printf("Error opening pipe!\n");
            return -1;
        }

        while(1){
            sem_wait(&sem_filesRemaining);
            sem_wait(&sem_file);
            read(fp, &fd, sizeof(fd));
            processed_files++;
            printf("%d\n",fd);
            sem_post(&sem_file);
      
     
            //if(fork()==0){
            //Acá pense en hacer una cola con todas las cosas procesadas y bue..
                // execv(/*minisat*/);

            //}
            
            sem_wait(&sem_buffer); //uuuuu
            while(processed_files != 0){
                char buf[] = "This is a processed file!\n";
                int bp = open("/tmp/buffer_pipe", O_WRONLY);
                printf("%d escribe en el buffer pipe\n", getpid());
                write(bp, buf, sizeof(buf));
                processed_files--;
                close(bp);
            }
            sem_post(&sem_buffer);
            
        }
        close(fp);
    }
    else
    {
        pthread_t buffer_reciever;
        pthread_create(&buffer_reciever, NULL, thread_routine,  NULL/*arg &var*/); // 0 si se crea bien
        
        int files = argc-1;
        int x = files;
        while(files != 0){
            x = files;
            sem_post(&sem_filesRemaining);
            int fp = open("/tmp/file_pipe", O_WRONLY);
            printf("LE PASO FILE, QUEDAN: %d\n",x);
            write(fp, &x,sizeof(x));
            files--;
            close(fp);
        }

        printf("ESPERO BUFFER\n");
        pthread_join(buffer_reciever,NULL);


       for(int i = 0; i < slavesAmmount; i++){
            printf("Matando el proceso %d\n", slaves[i]);
            kill(slaves[i],SIGKILL);
        }


    }

}



// void * thread_routine(void * arg){
//     while(*files_to_receive > 0){
//         int bp = open("/tmp/buffer_pipe", O_RDONLY);
//         printf("BUFFER LEIDO: %d\n", *files_to_receive + 1);
//             //if(read(bp, &x, sizeof(x)) == -1){
//             //    return 2;
//             //
//             // guardo x en mi buffe
//         close(bp);
//     }

// }


