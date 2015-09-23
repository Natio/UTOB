//
//  MessageQueue.c
//  utob
//
//  Created by Paolo Coronati on 26/02/15.
//  Copyright (c) 2015 Paolo Coronati. All rights reserved.
//

#include "MessageQueue.h"
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

typedef struct __MessageQ {
    Message * buffer;
    int bufferSize;
    int size;
    pthread_mutex_t lock;
    pthread_mutexattr_t attr;
    
}__MessageQ;


MessageQ MessageQ_Init(void){
    MessageQ self = malloc(sizeof(__MessageQ));
    self->buffer = malloc(sizeof(__MessageQ) * 20);
    self->bufferSize = 20;
    self->size = 0;
    pthread_mutexattr_init(&self->attr);
    pthread_mutexattr_settype(&self->attr, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(&self->lock, &self->attr);
    return self;
}

MessageQ MessageQ_FilterInstanceLessThan(MessageQ self,int instance){
    MessageQ queue = MessageQ_Init();
    
    pthread_mutex_lock(&self->lock);
    
    for (int i = 0; i < self->size; i++) {
        Message * m = &self->buffer[i];
        if (m->instance >= instance) {
            MessageQ_Add(queue, m);
        }
        
    }
    
    pthread_mutex_unlock(&self->lock);
    
    return queue;
}

void MessageQ_Add(MessageQ self, Message * m){
    pthread_mutex_lock(&self->lock);
    if (self->size >= self->bufferSize) {
        int newSize = self->bufferSize * 1.25;
        self->bufferSize = newSize;
        self->buffer = realloc(self->buffer, self->bufferSize * sizeof(__MessageQ));
    }
    self->buffer[self->size] = *m;
    self->size++;
    pthread_mutex_unlock(&self->lock);
}
Message MessageQ_GetMessageWithRound(MessageQ self, int rank, int instance, int * found){
    pthread_mutex_lock(&self->lock);
    Message m;
    *found = 0;
    for (int i = 0; i < self->size; i++) {
        int current_round = self->buffer[i].round;
        int current_instance = self->buffer[i].instance;
        if (current_round == rank && instance == current_instance) {
            m = self->buffer[i];
            
            memmove(&self->buffer[i], &self->buffer[i+1], (self->size - 1 - i)*sizeof(__MessageQ));
            *found = 1;
            self->size--;
            
            break;
        }
    }
    pthread_mutex_unlock(&self->lock);
    return m;
}


void MessageQ_Destroy(MessageQ self){
    pthread_mutex_destroy(&self->lock);
    free(self->buffer);
    free(self);
}