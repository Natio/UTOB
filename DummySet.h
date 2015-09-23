//
//  DummySet.h
//  
//
//  Created by Paolo Coronati on 24/02/15.
//
//

#ifndef ____DummySet__
#define ____DummySet__


typedef struct __DummySet* DummySet;

DummySet DummySetInit(DummySet s);
int DummySetContainsValue(DummySet s, int value);
int DummySetAddValue(DummySet s, int value);
int* DummySetGetBufferAndSize(DummySet s, int* size);
int DummySetRemoveValue(DummySet s, int value);
void DummySetDestroy(DummySet s);
void DummySetClear(DummySet s);
char* DummySetToString(DummySet s, char * );
void DummySetPrint(DummySet s);

DummySet DummySetUnion(DummySet s1, DummySet s2);
DummySet DummySetIntersection(DummySet s1, DummySet s2);
int DummySetSize(DummySet);
int DummySetGetMin(DummySet);

void DummySetLock(DummySet s);
void DummySetUnlock(DummySet s);


#endif /* defined(____DummySet__) */
