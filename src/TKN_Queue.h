#ifndef TKNQUEUE_H
#define TKNQUEUE_H

#include "TKN.h"

/* Circular buffer element */
typedef struct {
	TKN_Data data;	// 16 bytes
	BYTE id;		// The recipient or sender respectively
} TKN_Queue_Elem;

 /* Circular buffer object */
typedef struct {
	int size;   /* maximum number of elements           */
	int start;  /* index of oldest element              */
	int count;  /* index at which to write new element  */
	TKN_Queue_Elem *elems;  /* vector of elements       */
} TKN_Queue;


void TKN_Queue_Init(TKN_Queue *cb, int size);

void TKN_Queue_Free(TKN_Queue *cb);

int TKN_Queue_IsFull(TKN_Queue *cb);
 
int TKN_Queue_IsEmpty(TKN_Queue *cb);
 
void TKN_Queue_Push(TKN_Queue *cb, TKN_Data *data, BYTE id);
 
BYTE TKN_Queue_Pop(TKN_Queue *cb, TKN_Data *data);

#endif

