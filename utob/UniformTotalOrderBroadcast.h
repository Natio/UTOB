//
//  UniformTotalOrderBroadcast.h
//  utob
//
//  Created by Paolo Coronati on 26/02/15.
//  Copyright (c) 2015 Paolo Coronati. All rights reserved.
//

#ifndef __utob__UniformTotalOrderBroadcast__
#define __utob__UniformTotalOrderBroadcast__

#include "Consensus.h"
#include "Communicator.h"

typedef struct __UTOB* UTOB;

typedef void (*UTOB_Handler)(UTOB, int value, void*);

UTOB UTOB_Init(int port, int processes);

void UTOB_Start(UTOB, UTOB_Handler, void*ctx);
void UTOB_Send(UTOB, int  toSend);



#endif /* defined(__utob__UniformTotalOrderBroadcast__) */
