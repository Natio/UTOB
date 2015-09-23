//
//  Consensus.h
//  utob
//
//  Created by Paolo Coronati on 26/02/15.
//  Copyright (c) 2015 Paolo Coronati. All rights reserved.
//

#ifndef __utob__Consensus__
#define __utob__Consensus__
#include "Communicator.h"
#include "DummySet.h"

typedef struct __Consensus* Consensus;

typedef void (*ConsensusHandler)(Consensus, int value, void*);

Consensus ConsensusInit(Communicator c, DummySet correct);

void ConsensusPropose(Consensus c, int * value, int size);
void ConsensusSetHandler(Consensus self, ConsensusHandler h, void * ctx);
void ConsensusDestroy(Consensus c);
void ConsensusReset(Consensus);

#endif /* defined(__utob__Consensus__) */
