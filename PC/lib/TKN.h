#ifndef TKN_H
#define TKN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Program default values */
#define TKN_BAUD_DEFAULT 	57600
#define TKN_PORT_DEFAULT 	16
#define TKN_ID_DEFAULT 		1
#define TKN_DEST_ID_DEFAULT 	2

/* TKN Parameters */
#define TKN_READ_TIMEOUT 	100
#define TKN_MAX_ATTEMPTS 	10
#define TKN_QUEUE_SIZE 		50

/* TKN Packet Types */ 
#define TKN_TYPE_NONE 		0x00
#define TKN_TYPE_DATA 		0xAA
#define TKN_TYPE_ACK 		0xBB
#define TKN_TYPE_TOKEN		0xCC

/* TKN Packet byte Offsets */ 
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
#define TKN_DATA_SIZE	  	(TKN_OFFS_DATA_STOP - TKN_OFFS_DATA_START + 1)

/* TKN Data types */
typedef unsigned char BYTE;
typedef struct {BYTE data[TKN_DATA_SIZE]; } TKN_Data;

/* TKN Function Prototypes */ 
int TKN_ExportPackets ();
int TKN_Init (int port, int baud, BYTE id);
int TKN_InitWithArgs (int argc, char *argv[]);
int TKN_Close ();
int TKN_Start ();
int TKN_Stop ();
int TKN_GetTokenCount ();
int TKN_PushData (TKN_Data * cpBuf, BYTE recipientId);
int TKN_PopData (TKN_Data *);
int TKN_SendString (char * str, BYTE dest_id);
int TKN_WaitString(char *ready_str);

/**
 * TODO:
 */
/* Thes funcs should become static !!! */
int TKN_SendDataPacket (TKN_Data * data, BYTE to);
int TKN_PassToken ();
int TKN_Receive ();


#ifdef __cplusplus
}
#endif

#endif
