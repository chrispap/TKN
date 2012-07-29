#include <stdio.h>
#include <malloc.h>

#include "TKN_Queue.h"
#include "TKN.h"

void TKN_Queue_Init(TKN_Queue *cb, int size) {
    cb->size  = size;
    cb->start = 0;
    cb->count = 0;
    cb->elems = (TKN_Queue_Elem *)calloc(cb->size, sizeof(TKN_Queue_Elem));
}
 
void TKN_Queue_Free(TKN_Queue *cb) {
    free(cb->elems); /* OK if null */ }
 
int TKN_Queue_IsFull(TKN_Queue *cb) {
    return cb->count == cb->size; }
 
int TKN_Queue_IsEmpty(TKN_Queue *cb) {
    return cb->count == 0; }
 
void TKN_Queue_Push(TKN_Queue *cb, TKN_Data *data, BYTE id) {
    int end = (cb->start + cb->count) % cb->size;
    cb->elems[end].data = *data;
	cb->elems[end].id = id;
	
    if (cb->count == cb->size)
        cb->start = (cb->start + 1) % cb->size; /* full, overwrite */
    else
        ++ cb->count;
}
 
BYTE TKN_Queue_Pop(TKN_Queue *cb, TKN_Data *data) {
    *data = cb->elems[cb->start].data;
	BYTE sender_id = cb->elems[cb->start].id;
    cb->start = (cb->start + 1) % cb->size;
    -- cb->count;
	
	return sender_id;
}
