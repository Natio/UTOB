//
//  Consensus.c
//  utob
//
//  Created by Paolo Coronati on 26/02/15.
//  Copyright (c) 2015 Paolo Coronati. All rights reserved.
//

#include "Consensus.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include "MessageQueue.h"
#include <time.h>
#include <unistd.h>
#include "Common.h"

#define N 3
#define NOT_DECIDED INT32_MAX

struct __Consensus{
    
    DummySet correctSet;
    int round;
    int instance;
    int decision;
    DummySet proposalSet;
    DummySet recivedFrom;
    Communicator communicator;
    MessageQ queue;
    struct handler{
        ConsensusHandler handler;
        void * ctx;
    }handler;
    pthread_mutex_t lock;
    pthread_mutexattr_t attr;
    
} __Consensus;

void ConsensusMessageRecived(Message*, void*);

Consensus ConsensusInit(Communicator c, DummySet correct){
    Consensus self = malloc(sizeof(__Consensus));
    self->communicator = c;
    self->proposalSet = DummySetInit(NULL);
    self->correctSet = DummySetInit(correct);
    self->recivedFrom = DummySetInit(NULL);
    self->decision = NOT_DECIDED;
    self->round = 1;
    self->instance = 1;
    self->queue = MessageQ_Init();
    pthread_mutexattr_init(&self->attr);
    pthread_mutexattr_settype(&self->attr, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(&self->lock, &self->attr);
    
    CommunicatorAddHandler(c, ConsensusMessageRecived, self);
    
    return self;
}

void ConsensusReset(Consensus self){
    pthread_mutex_lock(&self->lock);
    self->round = 1;
    self->instance++;
    self->decision = NOT_DECIDED;
    MessageQ old = self->queue;
    self->queue = MessageQ_FilterInstanceLessThan(old, self->instance);
    MessageQ_Destroy(old);
    
    DummySetClear(self->proposalSet);
    DummySetClear(self->recivedFrom);
    pthread_mutex_unlock(&self->lock);
}

void ConsensusSetHandler(Consensus self, ConsensusHandler h, void * ctx){
    pthread_mutex_lock(&self->lock);
    self->handler.handler = h;
    self->handler.ctx = ctx;
    pthread_mutex_unlock(&self->lock);
}

void ConsensusReadValidMessage(Consensus self, Message * message){
    int  * payload = (int*)message->payload;
    int size = payload[0];
    
    for (int i = 0; i < size; i++) {
        DummySetAddValue(self->proposalSet, payload[i+1]);
    }
    
    DummySetAddValue(self->recivedFrom, message->sender);
}

void ConsensusEmptyMessageQueue(Consensus self){
    while (1) {
        int found;
        Message m = MessageQ_GetMessageWithRound(self->queue, self->round, self->instance, &found);
        
        if (!found) {
            break;
        }
        printf("Leggo messaggio da: %d round: %d\n",m.sender, self->round);
        ConsensusReadValidMessage(self, &m);
    }
}

void ConsensusMessageRecived(Message * message, void * ctx){
    Consensus self = ctx;
    pthread_mutex_lock(&self->lock);
    //printf("Message: %s from %d\n",message->payload,message->sender);
    
    int should_check = 0;
    
    //DLog("Entro in message recived\n");
    
    if (message->type == CRASH_TAG) {
        int crashedProcess = message->crashed_process;
        DLog("Detected crash: process: %d\n",crashedProcess);
        
        DummySetRemoveValue(self->correctSet, crashedProcess);
        ConsensusEmptyMessageQueue(self);
        should_check = 1;
    }
    else if(message->type == CONSENSUS_TAG){
        

        int round = message->round;
        
        if(round > self->round){
            DLog("Inserisco in coda round %d ma sono a %d sender: %d\n",round, self->round, message->sender);
            MessageQ_Add(self->queue, message);
            pthread_mutex_unlock(&self->lock);
            return;
        }
        else if(round < self->round){
            DLog("Messaggio dal futuro: round msg %d my round %d sender %d round(my %d msg %d)\n",round, self->round, message->sender, self->instance, message->instance);
            MessageQ_Add(self->queue, message);
            //pthread_mutex_unlock(&self->lock);
            //return;
        }
        else{
            //DLog("Ricevo correttamente da %d round %d\n",message->sender, round);
            ConsensusReadValidMessage(self, message);
            ConsensusEmptyMessageQueue(self);
            
        }
        should_check = 1;
       
        
    }
    
    
    if (should_check) {
        
        DummySet intersection = DummySetIntersection(self->recivedFrom, self->correctSet);
        int intersection_size = DummySetSize(intersection);
        int correct_size = DummySetSize(self->correctSet);
        
        if (intersection_size >= correct_size && self->decision == NOT_DECIDED) {
        
            if (self->round == N) {
                self->decision = DummySetGetMin(self->proposalSet);
                //DLog("Decided: %d\n",self->decision);
                
                if (self->handler.handler != NULL) {
                    self->handler.handler(self, self->decision, self->handler.ctx);
                }
                 pthread_mutex_unlock(&self->lock);
                return;
            }
            else{
                DLog("Inizio Round %d\n", (self->round + 1));
                self->round++;
                DummySetClear(self->recivedFrom);
                
                Message m;
                m.type = CONSENSUS_TAG;
                m.round = self->round;
                m.instance = self->instance;
                int * payload = (int*)m.payload;
                
                int size;
                
                DummySetLock(self->proposalSet);
                int * buffer = DummySetGetBufferAndSize(self->proposalSet, &size);
                DummySetUnlock(self->proposalSet);
                
                payload[0] = size;
                
                for (int i = 0; i < size; i++) {
                    payload[i+1] = buffer[i];
                }
                
                CommunicatorWrite(self->communicator, &m);
                
            }
        }
        
        
        DummySetDestroy(intersection);
    }
    //DLog("Esco da messageRecived\n");
    //sleep((rand() % 2));
    
    pthread_mutex_unlock(&self->lock);
    
    
    
}

void ConsensusPropose(Consensus self, int * value, int size){
    pthread_mutex_lock(&self->lock);
    for (int i = 0; i < size; i++) {
          DummySetAddValue(self->proposalSet, value[i]);
    }
  
    
    Message m;
    m.type = CONSENSUS_TAG;
    m.round = 1;
    m.instance = self->instance;
    int * message = (int*)m.payload;
    message[0] = size;
    for (int i = 0; i < size; i++) {
        message[i+1] = value[i];
    }
    
    CommunicatorWrite(self->communicator, &m);
    pthread_mutex_unlock(&self->lock);
    
    
}

void ConsensusDestroy(Consensus self){
    if (self->decision == INT32_MAX) {
        printf("Cannot destroy consensus while running\n");
        exit(-1);
    }
    
    DummySetDestroy(self->correctSet);
    DummySetDestroy(self->proposalSet);
    DummySetDestroy(self->recivedFrom);
    free(self);
    
}



