//
//  main.c
//  utob
//
//  Created by Paolo Coronati on 25/02/15.
//  Copyright (c) 2015 Paolo Coronati. All rights reserved.
//

#include <stdio.h>
#include <mpi.h>

#include <pthread.h>

#include "BestEffortBroadcast.h"
#include "Common.h"
#include "Communicator.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>


pthread_t client_thread;
int world_rank;
int connected;
Beb beb;
Communicator communicator;

Message toForward;
int hasMessage = 0;
int crashed = 0;

pthread_mutex_t lock;


#define LOG(...) do{\
                if(world_rank == 0) printf(__VA_ARGS__);\
                }while(0)


void handler(Message * m, void * ctx){
    
    if (m == NULL && ctx == NULL) {
        while (1) {
            pthread_mutex_lock(&lock);
            if (!hasMessage) {
                LOG("Posto messaggio\n");
                Message crash;
                crash.sender = world_rank;
                crash.type = CRASH_TAG;
                crash.crashed_process = world_rank;
                connected = 0;
                crashed++;
                toForward = crash;
                hasMessage = 1;
                pthread_mutex_unlock(&lock);
                return;
            }
            LOG("Trovato vecchio messaggio continuo a looppare\n");
            pthread_mutex_unlock(&lock);
        }
        return;
    }
    else{
        while (1) {
            pthread_mutex_lock(&lock);
            if (!hasMessage) {
                LOG("Posto messaggio\n");
                toForward = *m;
                hasMessage = 1;
                pthread_mutex_unlock(&lock);
                return;
            }
            LOG("Trovato vecchio messaggio continuo a looppare\n");
            pthread_mutex_unlock(&lock);
        }
        //BebSend(beb, m, m->type);
    }
}




int main(int argc, char** argv) {
    // Initialize the MPI environment
    //int required = MPI_THREAD_MULTIPLE;
    //int provided;
    
    pthread_mutex_init(&lock, NULL);

    
    //MPI_Init_thread(&argc,&argv, required, &provided);
    MPI_Init(NULL, NULL);
   /*
    if (provided < required) {
        printf("Niente THREAD %d!!!!\n",provided);
        MPI_Abort(MPI_COMM_WORLD, 1);
        exit(1);
    }
    */
    
    MPI_Comm world = MPI_COMM_WORLD;
    
    // Get the number of processes
    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    beb = BebInit(&world);

    
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    int serverPort = SERVER_BASE_PORT + world_rank;
    
    //volatile int stop = 1;
    //swhile(stop == 1);
    
    communicator = CommunicatorInit(serverPort);
    CommunicatorAddHandler(communicator, handler, NULL);
    CommunicatorStartServer(communicator);
    connected = 1;
    
    printf("World Size: %d\n",world_size);
    printf("World Rank: %d\n", world_rank);
    
    MPI_Barrier(MPI_COMM_WORLD);//wait all processes to connect
    printf("Barrier superata %d\n",world_rank);
    
    while (connected) {
        MPI_Request request;
        Message m;
        MPI_Irecv(&m, sizeof(Message), MPI_CHAR, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &request);
        
        while (1) {
            pthread_mutex_lock(&lock);
            
            if (hasMessage) {
                LOG("Inoltro messaggio\n");
                BebSend(beb, &toForward, toForward.type);
                //CommunicatorWrite(communicator, &toForward);
                hasMessage = 0;
            }
            pthread_mutex_unlock(&lock);
            
            
            int flag;
            int count;
            MPI_Status status;
            MPI_Test(&request, &flag, &status);
            
            
            if (flag ) {
                LOG("Ricevuto BEB inoltro a strato superiore\n");
                m.sender = status.MPI_SOURCE;
                MPI_Get_count(&status, MPI_CHAR, &count);
                if (count > 0) {
                    if(CommunicatorWrite(communicator, &m) < 0){
                        connected = 0;
                        break ;
                    }
                }
                break;
            }
        }
        
        /*Message m;
        int out_tag;
        int size = BebRecieve(beb, &m, &out_tag);
        if (size > 0 && connected == 1) {
           
            CommunicatorWrite(communicator, &m);
        }*/
    }

    MPI_Finalize();
    
    
}
