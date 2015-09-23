//
//  Communicator.h
//  utob
//
//  Created by Paolo Coronati on 26/02/15.
//  Copyright (c) 2015 Paolo Coronati. All rights reserved.
//

#ifndef __utob__Communicator__
#define __utob__Communicator__

#define CRASH_TAG 1
#define CONSENSUS_TAG 2
#define TYPE_UTOB_BEB 3
#define TYPE_DECIDE 4

typedef struct Message {
    int sender;
    int type;
    int crashed_process;
    int round;
    int instance;
    char payload[500];
} Message;

#define mesgcpy(src, dest) memcpy((dest),(src),sizeof(Message))


typedef void (*MessageHandler)(Message *, void*) ;


typedef struct __Communicator* Communicator;

Communicator CommunicatorInit(int port);
void CommunicatorStartClient(Communicator c);
void CommunicatorStartServer(Communicator c);
void CommunicatorAddHandler(Communicator, MessageHandler hander, void * ctx);
void CommunicatorRemoveHandler(Communicator, void*);
long CommunicatorWrite(Communicator, Message *);
#endif /* defined(__utob__Communicator__) */
