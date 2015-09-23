//
//  BestEffortBroadcast.c
//  utob
//
//  Created by Paolo Coronati on 25/02/15.
//  Copyright (c) 2015 Paolo Coronati. All rights reserved.
//

#include "BestEffortBroadcast.h"
#include <stdlib.h>
#include <string.h>

typedef struct __Beb {
    MPI_Comm * communicator;
    int communicator_size;
    int my_rank;
}__Beb;


Beb BebInit(MPI_Comm * com){
    Beb self = malloc(sizeof(__Beb));
    self->communicator = com;
    MPI_Comm_size(*com, &self->communicator_size);
    MPI_Comm_rank(*com, &self->my_rank);
    return self;
}

void BebSend(Beb b, Message * buffer, int tag){
    
    
    
    for (int i = 0; i < b->communicator_size; i++) {
        //if(i != b->my_rank){
            MPI_Send(buffer, sizeof(Message), MPI_CHAR, i, tag, *b->communicator);
        //}
    }
        
}


int BebRecieve(Beb b, Message * buffer, int * tag){
    if (buffer == NULL) {
        printf("Null message\n");
        return -1;
    }
    if (tag == NULL) {
        printf("Tag NULL");
    }
    //printf("Ricevo da %d type %d crashed %d \n", buffer->sender, buffer->type, buffer->crashed_process);
    MPI_Status status;
    MPI_Recv(buffer, sizeof(Message), MPI_CHAR, MPI_ANY_SOURCE, MPI_ANY_TAG, *b->communicator, &status);
    int count;
    if (tag != NULL) {
        *tag = status.MPI_TAG;
    }
    /*
    if (*tag == CONSENSUS_TAG) {
        printf("Ricevo da %d type %d round %d \n", buffer->sender, buffer->type, ((int*)buffer->payload)[0]);
    }
    */
    
    buffer->sender = status.MPI_SOURCE;
    MPI_Get_count(&status, MPI_CHAR, &count);
    return count;
    
}













