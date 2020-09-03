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

int * files_remaining;
int * files_to_receive;
int * slave_counter;

sem_t sem_file;
sem_t sem_buffer;
sem_t sem_filesRemaining;
sem_t sem_filesReceived;
sem_t sem_slaveCounter;

int getFilesToReceive(){
    sem_wait(&sem_filesReceived);
    int retval = *files_to_receive;
    sem_post(&sem_filesReceived);
    return retval;
}

int getFilesRemaining(){
    sem_wait(&sem_filesRemaining);
    int retval = *files_remaining;
    sem_post(&sem_filesRemaining);
    return retval;
}

int getSlaveCounter(){
    sem_wait(&sem_slaveCounter);
    int retval = *slave_counter;
    sem_post(&sem_slaveCounter);
    return retval;
}


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

    int * slaves = malloc(sizeof(char)*argc);

    

    int fd = shm_open("/filesRemaining", O_CREAT | O_RDWR  , 0600); /* create s.m object*/
    ftruncate(fd, sizeof(int)); 
    files_remaining = mmap(NULL, sizeof(int), PROT_WRITE, MAP_SHARED, fd, 0);
    *files_remaining = argc-1;

    int fd2 = shm_open("/filesReceived", O_CREAT | O_RDWR  , 0600); /* create s.m object*/
    ftruncate(fd2, sizeof(int)); 
    files_to_receive = mmap(NULL, sizeof(int), PROT_WRITE, MAP_SHARED, fd2, 0);
    *files_to_receive = argc-1;

    int fd3 = shm_open("/slave_counter", O_CREAT | O_RDWR  , 0600); /* create s.m object*/
    ftruncate(fd3, sizeof(int)); 
    slave_counter = mmap(NULL, sizeof(int), PROT_WRITE, MAP_SHARED, fd3, 0);
    *slave_counter = 0;



    
    
    sem_init(&sem_buffer, 1/*porque es compartido entre procesos*/, 1);
    sem_init(&sem_file, 1,1 /*cantidad de puntos q empieza */);
    sem_init(&sem_slaveCounter, 1, 1);
    sem_init(&sem_filesReceived, 1,1 );
    sem_init(&sem_filesRemaining, 1,1 );

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


