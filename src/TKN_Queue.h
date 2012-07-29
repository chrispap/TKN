void TKN_Queue_Init(TKN_Queue *cb, int size);

void TKN_Queue_Free(TKN_Queue *cb);

int TKN_Queue_IsFull(TKN_Queue *cb);
 
int TKN_Queue_IsEmpty(TKN_Queue *cb);
 
void TKN_Queue_Push(TKN_Queue *cb, TKN_Data *elem);
 
void TKN_Queue_Pop(TKN_Queue *cb, TKN_Data *elem);
