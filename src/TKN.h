#ifndef TKN_H
#define TKN_H

#define TKN_BAUD_DEFAULT 57600
#define TKN_PORT_DEFAULT 1
#define TKN_READ_TIMEOUT 100
#define TKN_ID_DEFAULT 1
  
/* Token Protocol Packet Types */ 
#define TKN_TYPE_NONE 		0x00
#define TKN_TYPE_DATA 		0xAA
#define TKN_TYPE_ACK 		0xBB
#define TKN_TYPE_TOKEN		0xCC
  
/* Packet byte Offsets */ 
#define TKN_OFFS_HEADER 	0
#define TKN_OFFS_TYPE		1
#define TKN_OFFS_RECEIVER 	2
#define TKN_OFFS_SENDER		3
#define TKN_OFFS_TOKEN_EOF	4
#define TKN_OFFS_PACKET_ID	4
#define TKN_OFFS_CONTROL	5
#define TKN_OFFS_ACK_EOF	5
#define TKN_OFFS_DATA_START	6
#define TKN_OFFS_DATA_STOP	21
#define TKN_OFFS_DATA_EOF	22
  
#define TKN_PACKET_SIZE		(TKN_OFFS_DATA_STOP - TKN_OFFS_DATA_START + 1)
  
/* Prototypes */ 
typedef unsigned char BYTE;
 int TKN_Init (int port, int baud, BYTE id);
int TKN_Close ();
int TKN_Receive ();
int TKN_PassToken ();
int TKN_Send (BYTE * data, BYTE to);
int TKN_SendDataPacket (BYTE * data, BYTE to, BYTE from, BYTE pack_id);
int TKN_SendAckPacket (BYTE to, BYTE from, BYTE pack_id);
int TKN_IsDataValid (BYTE *, BYTE);
int TKN_PrintDataPacket (BYTE *, int, int);
int TKN_PrintCols ();
int TKN_PrintByte (BYTE c, int forceHex);
int TKN_PopData (BYTE *);

int TKN_PushData (BYTE * cpBuf, BYTE recipientId); 
void* TKN_Run(void* );

int TKN_Start();

int TKN_Stop();

int TKN_TokenCount();
#endif
