//
//  BestEffortBroadcast.h
//  utob
//
//  Created by Paolo Coronati on 25/02/15.
//  Copyright (c) 2015 Paolo Coronati. All rights reserved.
//

#ifndef __utob__BestEffortBroadcast__
#define __utob__BestEffortBroadcast__

#include <stdio.h>
#include <mpi.h>
#include "Communicator.h"


typedef struct __Beb* Beb;


Beb BebInit(MPI_Comm *);

void BebSend(Beb b, Message * m, int tag);
int BebRecieve(Beb b, Message * buffer, int * tag);

#endif /* defined(__utob__BestEffortBroadcast__) */
