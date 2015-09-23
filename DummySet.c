//
//  DummySet.c
//  
//
//  Created by Paolo Coronati on 24/02/15.
//
//

#include "DummySet.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <pthread.h>



typedef struct __DummySet {
    int * buffer;
    int bufferSize;
    int size;
    pthread_mutex_t lock;
    pthread_mutexattr_t attr;
    
}__DummySet;

static inline int binSearch(int * array, int size, int search);



DummySet DummySetInit(DummySet s){
    __DummySet * self = (__DummySet*)malloc(sizeof (__DummySet));
    
    pthread_mutexattr_init(&self->attr);
    pthread_mutexattr_settype(&self->attr, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(&self->lock, &self->attr);
    
    if(s == NULL){
        self->bufferSize = 20;
        self->size = 0;
        self->buffer = (int*)malloc(sizeof(int) * self->bufferSize);
    }
    else{
        self->bufferSize = s->bufferSize;
        self->size = s->size;
        self->buffer = (int*)malloc(sizeof(int) * self->bufferSize);
        memcpy(self->buffer, s->buffer, s->size * sizeof(int));
    }
    
    return self;
}


void DummySetLock(DummySet s){
    pthread_mutex_lock(&s->lock);
}

void DummySetUnlock(DummySet s){
    pthread_mutex_unlock(&s->lock);
}

void DummySetClear(DummySet s){
    s->size = 0;//todo make more memory efficient
}

int DummySetSize(DummySet s){
    return s->size;
}

int DummySetGetMin(DummySet s){
    pthread_mutex_lock(&s->lock);
    if (s->size == 0) {
        pthread_mutex_unlock(&s->lock);
        return INT32_MAX;
    }
    int toRet = s->buffer[0];
    pthread_mutex_unlock(&s->lock);
    return toRet;
}


void DummySetDestroy(DummySet s){
    pthread_mutex_destroy(&s->lock);
    free(s->buffer);
    free((__DummySet*)s);
    
}


int* DummySetGetBufferAndSize(DummySet s, int* size){
    *size = s->size;
    return s->buffer;
}


void DummySetPrint(DummySet s){
    pthread_mutex_lock(&s->lock);
    int size;
    int * buff = DummySetGetBufferAndSize(s, &size);
    printf("Size %d\n", size);
    for(int i =0; i< size; i++){
        printf("%d\n", buff[i]);
    }
    pthread_mutex_unlock(&s->lock);
}

char* DummySetToString(DummySet s, char * buffer){
    pthread_mutex_lock(&s->lock);
    int size;
    int * buff = DummySetGetBufferAndSize(s, &size);
    buffer[0] = '\0';
    char tmp[50];
    for(int i =0; i< size; i++){
        if (i == 0) {
            sprintf(tmp, "%d",buff[i]);
        }
        else{
            sprintf(tmp, ",%d",buff[i]);
        }
        strcat(buffer, tmp);
        //printf("%d\n", buff[i]);
    }
    pthread_mutex_unlock(&s->lock);
    return buffer;
}

int DummySetContainsValue(DummySet s, int value){
    pthread_mutex_lock(&s->lock);
    int indexOf = binSearch(s->buffer, s->size, value);
    pthread_mutex_unlock(&s->lock);
    return indexOf > -1 ? 1 : -1;
}

int DummySetAddValue(DummySet s, int value){
    pthread_mutex_lock(&s->lock);
    if(s->size == 0){
        s->buffer[0] = value;
        s->size=1;
        pthread_mutex_unlock(&s->lock);
        return 1;
    }
    __DummySet * self = (__DummySet*)s;
    int indexOf = binSearch(s->buffer, s->size, value);
    if(indexOf >= 0){
        //printf("Trovato %d a indice %d\n",value, indexOf);
        pthread_mutex_unlock(&s->lock);
        return indexOf;
    }
    
    if (s->bufferSize == s->size) {
        int newSize = s->bufferSize * 1.25;
        //printf("DummySet: realloco new size %d old size %d\n", newSize, self->bufferSize);
        self->bufferSize = newSize;
        self->buffer = (int*)realloc(s->buffer, newSize * sizeof(int));
    }
    
    int insertIndex = 0;
    for (int i = 0; i < s->size; i++) {
        if(value < s->buffer[i]){
            insertIndex = i;
            break;
        }
        insertIndex++;
    }
    //DummySetPrint(s);
    //printf("Adding: %d\n",value);
    memmove(&s->buffer[insertIndex+1], &s->buffer[insertIndex], (s->size - insertIndex) * sizeof(int));
    self->buffer[insertIndex] = value;
    self->size++;
    //DummySetPrint(s);
    //printf("Added: %d\n\n\n",value);
    pthread_mutex_unlock(&s->lock);
    return insertIndex;
}


int DummySetRemoveValue(DummySet s, int value){
    pthread_mutex_lock(&s->lock);
    __DummySet * self = (__DummySet*)s;
    int indexOf = binSearch(s->buffer, s->size, value);
    if(indexOf < 0){
        pthread_mutex_unlock(&s->lock);
        return -1;
    }
    
    memmove(&s->buffer[indexOf], &s->buffer[indexOf+1], (s->size -1- indexOf)*sizeof(int));
    
    self->size--;
    pthread_mutex_unlock(&s->lock);
    return indexOf;
    
}

DummySet DummySetUnion(DummySet s1, DummySet s2){
    
    DummySet clone = NULL;
    DummySet other = NULL;
    
    DummySetLock(s1);
    DummySetLock(s2);
    
    if(s1-> size > s2->size){
        clone = DummySetInit(s1);
        other = s2;
    }
    else{
        clone = DummySetInit(s2);
        other = s1;
    }
    
    
    
    
    for (int i = 0; i< other->size ; i++) {
        DummySetAddValue(clone, other->buffer[i]);
    }
    
    DummySetUnlock(s1);
    DummySetUnlock(s2);
    
    return clone;
}


DummySet DummySetIntersection(DummySet s1, DummySet s2){
    
    DummySet d = DummySetInit(NULL);
    
    DummySetLock(s1);
    DummySetLock(s2);
    
    DummySet big = s2;
    DummySet small = s1;
    
    if(s1->size > s2->size){
        big = s1;
        small = s2;
    }
    
    for (int i = 0; i < small->size; i++) {
        int item = small->buffer[i];
        if(DummySetContainsValue(big, item) > 0){
            DummySetAddValue(d, item);
        }
    }
    
    DummySetUnlock(s1);
    DummySetUnlock(s2);
    
    return d;
}



static inline int binSearch(int * array, int size, int search){
    int first = 0;
    int last = size - 1;
    int middle = (first+last)/2;
    
    while( first <= last )
    {
        if ( array[middle] < search )
            first = middle + 1;
        else if ( array[middle] == search )
        {
            return middle;
            break;
        }
        else
            last = middle - 1;
        
        middle = (first + last)/2;
    }
    return -1;
}