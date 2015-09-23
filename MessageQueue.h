//
//  MessageQueue.h
//  utob
//
//  Created by Paolo Coronati on 26/02/15.
//  Copyright (c) 2015 Paolo Coronati. All rights reserved.
//

#ifndef __utob__MessageQueue__
#define __utob__MessageQueue__
#include "Communicator.h"
#include <stdio.h>

typedef struct __MessageQ* MessageQ;

MessageQ MessageQ_Init(void);
void MessageQ_Add(MessageQ, Message*);
Message MessageQ_GetMessageWithRound(MessageQ, int,int, int*);
void MessageQ_Destroy(MessageQ);
MessageQ MessageQ_FilterInstanceLessThan(MessageQ,int);

#endif /* defined(__utob__MessageQueue__) */
