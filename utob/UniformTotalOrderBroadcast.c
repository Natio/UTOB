//
//  UniformTotalOrderBroadcast.c
//  utob
//
//  Created by Paolo Coronati on 26/02/15.
//  Copyright (c) 2015 Paolo Coronati. All rights reserved.
//

#include "UniformTotalOrderBroadcast.h"
#include "DummySet.h"
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

#define WAIT_TRUE 1
#define WAIT_FALSE 0

#define USE_LOCK 1

#ifdef USE_LOCK
#define LOCK(ptr)  pthread_mutex_lock(&(ptr)->lock)
#define UNLOCK(ptr) pthread_mutex_unlock(&(ptr)->lock)
#else
#define LOCK(ptr) /**/
#define UNLOCK(ptr) /**/
#endif

struct __UTOB{
    int wait;
    int round;
    DummySet unordered;
    DummySet delivered;
    Communicator beb;
    Consensus consensus;
    
    struct handler{
        UTOB_Handler handler;
        void * ctx;
    }handler;
#ifdef USE_LOCK
    pthread_mutex_t lock;
    pthread_mutexattr_t attr;
#endif
    
} __UTOB;

void UTOB_CommunicatorHandler(Message *, void*);
void UTOB_ConsensusHandler(Consensus, int value, void*);



UTOB UTOB_Init(int port, int processes){
    UTOB self = malloc(sizeof(__UTOB));
    
    
    DummySet correct = DummySetInit(NULL);
    for (int i = 0; i < processes; i++) {
        DummySetAddValue(correct, i);
    }
    
    self->beb = CommunicatorInit(port);
    self->consensus = ConsensusInit(self->beb, correct);
    
    DummySetDestroy(correct);
    
    self->delivered = DummySetInit(NULL);
    self->unordered = DummySetInit(NULL);
    self->wait = WAIT_FALSE;
    self->round = 1;
#ifdef USE_LOCK
    pthread_mutexattr_init(&self->attr);
    pthread_mutexattr_settype(&self->attr, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(&self->lock, &self->attr);
#endif
    return self;
}

void UTOB_Start(UTOB self, UTOB_Handler handler, void*ctx){
    LOCK(self);
    self->handler.handler = handler;
    self->handler.ctx = ctx;
    
    CommunicatorAddHandler(self->beb, UTOB_CommunicatorHandler, self);
    ConsensusSetHandler(self->consensus, UTOB_ConsensusHandler, self);
    CommunicatorStartClient(self->beb);
    UNLOCK(self);
}


void UTOB_Send(UTOB self, int toSend){
    LOCK(self);
    Message m;
    m.type = TYPE_UTOB_BEB;
    *(int*)m.payload = toSend;
    CommunicatorWrite(self->beb, &m);
    UNLOCK(self);
}


void UTOB_CheckAndStartConsensus(UTOB self){
    LOCK(self);
    if (self->wait == WAIT_FALSE && DummySetSize(self->unordered) != 0) {
        self->wait = WAIT_TRUE;
        
        DummySetLock(self->unordered);
        int count;
        int * values = DummySetGetBufferAndSize(self->unordered, &count);
        DummySetLock(self->unordered);
        
        ConsensusPropose(self->consensus, values, count);
        
    }
    UNLOCK(self);
}

void UTOB_CommunicatorHandler(Message * m, void * ctx){
    
    UTOB self = ctx;
    
    if (m->type != TYPE_UTOB_BEB) {
        return;
    }
    
    LOCK(self);
    int value = *(int*)m->payload;
    if (DummySetContainsValue(self->unordered, value) < 0) {
        DummySetAddValue(self->unordered, value);
    }
    
    UTOB_CheckAndStartConsensus(self);
    UNLOCK(self);
}




void UTOB_ConsensusHandler(Consensus consensus, int value, void* ctx){
    UTOB self = ctx;
    
    LOCK(self);
    
    //printf("Consensus %d\n", value);
    if (self->handler.handler != NULL) {
        self->handler.handler(self, value, self->handler.ctx);
    }
    
    ConsensusReset(consensus);
    DummySetAddValue(self->delivered, value);
    DummySetRemoveValue(self->unordered, value);
    self->round++;
    self->wait = WAIT_FALSE;
    UTOB_CheckAndStartConsensus(self);
    
    UNLOCK(self);
}







