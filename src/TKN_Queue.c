#include <stdio.h>
#include <malloc.h>

#include "TKN_Queue.h"
#include "TKN.h"
 
 /* Circular buffer object */
typedef struct {
    int         size;   /* maximum number of elements           */
    int         start;  /* index of oldest element              */
    int         count;  /* index at which to write new element  */
    TKN_Data   *elems;  /* vector of elements                   */
} TKN_Queue;
 

void TKN_Queue_Init(TKN_Queue *cb, int size) {
    cb->size  = size;
    cb->start = 0;
    cb->count = 0;
    cb->elems = (TKN_Data *)calloc(cb->size, sizeof(TKN_Data));
}
 
void TKN_Queue_Free(TKN_Queue *cb) {
    free(cb->elems); /* OK if null */ }
 
int TKN_Queue_IsFull(TKN_Queue *cb) {
    return cb->count == cb->size; }
 
int TKN_Queue_IsEmpty(TKN_Queue *cb) {
    return cb->count == 0; }
 
void TKN_Queue_Push(TKN_Queue *cb, TKN_Data *elem) {
    int end = (cb->start + cb->count) % cb->size;
    cb->elems[end] = *elem;
    if (cb->count == cb->size)
        cb->start = (cb->start + 1) % cb->size; /* full, overwrite */
    else
        ++ cb->count;
}
 
void TKN_Queue_Pop(TKN_Queue *cb, TKN_Data *elem) {
    *elem = cb->elems[cb->start];
    cb->start = (cb->start + 1) % cb->size;
    -- cb->count;
}

