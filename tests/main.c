//
//  main.c
//  tests
//
//  Created by Paolo Coronati on 26/02/15.
//  Copyright (c) 2015 Paolo Coronati. All rights reserved.
//

#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <pthread.h>

#include "Communicator.h"
#include "DummySet.h"
#include "Common.h"

#define ASS(X) {printf(#X); assert(X); printf(" PASSED\n");}

Communicator server;
Communicator client;

void handlerServer(Message * m, void * ctx){
    printf("Server: da %d\n",m->sender);
    CommunicatorWrite(server, m->payload);
}

void handler(Message * m, void * ctx){
    printf("from: %d %s %s\n",m->sender, (char*)ctx, m->payload);
}

void * startServer(void * c){
    CommunicatorStartServer(server);
    return NULL;
}

int main(int argc, const char * argv[]) {
    
    /*
    DummySet set = DummySetInit(NULL);
    int size = DummySetSize(set);
    ASS(size == 0);
    
    DummySetAddValue(set, 1);
    size = DummySetSize(set);
    ASS(size == 1);
    
    DummySetAddValue(set, 1);
    size = DummySetSize(set);
    ASS(size == 1);
    
    DummySetRemoveValue(set, 1);
    size = DummySetSize(set);
    ASS(size == 0);
    
    
    for (int i = 0; i < 100; i++) {
        DummySetAddValue(set, i);
    }
    size = DummySetSize(set);
    ASS(size == 100);
    
    DummySet other = DummySetInit(NULL);
    DummySetAddValue(other, 1);
    DummySetAddValue(other, 99);
    
    DummySet intersection = DummySetIntersection(other, set);
    size = DummySetSize(intersection);
    ASS(size == 2);*/
    
    server = CommunicatorInit(SERVER_BASE_PORT);
    client = CommunicatorInit(SERVER_BASE_PORT);
    
    
    CommunicatorAddHandler(server, handlerServer, "server_ctx");
    CommunicatorAddHandler(client, handler, "client_ctx");
    pthread_t t;
    pthread_create(&t, NULL, startServer, NULL);
    sleep(3);
    CommunicatorStartClient(client);
    char buffer[100];
    
    while (1) {
        CommunicatorWrite(client, gets(buffer)) ;
        printf("h: %s\n",buffer);
    }
    
    return 0;
}
