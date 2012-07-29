#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#ifdef __linux__
  #include <pthread.h>
  #include <sys/stat.h>
  #include <sys/types.h>
#else
  #include <windows.h>
#endif

#include "TKN.h"
#include "rs232.h"

#define TKN_DEBUG
#ifdef TKN_DEBUG
  #define ECHO_ATTEMPTS
  #define ECHO_TOKENS
  #define ECHO_EVENTS
  #define ECHO_DATA
#endif

/* The Packet Buffers */
static BYTE PACKET_COUNTER;
static BYTE DATA_PACKET[TKN_OFFS_DATA_EOF + 1]    = { 0x00, TKN_TYPE_DATA, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF };
static BYTE ACK_PACKET[TKN_OFFS_ACK_EOF + 1]      = { 0x00, TKN_TYPE_ACK, 0x00, 0x00, 0x00, 0xFF };
static BYTE TOKEN_PACKET[TKN_OFFS_TOKEN_EOF + 1]  = { 0x00, TKN_TYPE_TOKEN, 0x00, 0x00, 0xFF };

/* The Data Queues*/
static BYTE RX_QUEUE_DATA [TKN_PACKET_SIZE * TKN_QUEUE_SIZE];
static BYTE RX_QUEUE_ID [TKN_QUEUE_SIZE];
static int  RX_PENDING = 0;

static BYTE TX_QUEUE_DATA [TKN_PACKET_SIZE * TKN_QUEUE_SIZE];
static BYTE TX_QUEUE_ID [TKN_QUEUE_SIZE];
static int  TX_PENDING = 0;

/* Params */
static int PORT_NUM;
static BYTE MY_ID;

/* Stats */
static int TKN_TokenCount=0;

/* Status Variables */
static volatile int TKN_Running=0;
static pthread_t TKN_Thread;

/* Static Function Prototypes */
static void* TKN_Run(void* );
static int   TKN_SendDataPacket (BYTE * data, BYTE to, BYTE from, BYTE pack_id);
static int   TKN_SendAckPacket (BYTE to, BYTE from, BYTE pack_id);
static int   TKN_IsDataValid (BYTE *, BYTE);
static int   TKN_PrintDataPacket (BYTE *, int, int);
static int   TKN_PrintCols ();
static int   TKN_PrintByte (BYTE c, int forceHex);


/* Print Functions */
int TKN_PrintCols ()
{
    printf ("FROM\tTO\tID\tDATA \n");
    printf ("-------------------------------------------\n");

    return 0;
}

int TKN_PrintDataPacket (BYTE * buffer, int details, int bin)
{
    static char data[TKN_PACKET_SIZE + 1];	// +1 for the zero char

    printf ("\n");

    if (details)
    {
        printf ("%d%s \t", (int) buffer[TKN_OFFS_SENDER],
        buffer[TKN_OFFS_SENDER] == MY_ID ? "*" : " ");
        printf ("%d%s \t", (int) buffer[TKN_OFFS_RECEIVER],
        buffer[TKN_OFFS_RECEIVER] == MY_ID ? "*" : " ");
        printf ("%d \t", (int) buffer[TKN_OFFS_PACKET_ID]);
    }
    else
    {
        printf ("\t\t\t");
    }

    if (!bin)
    {
        memcpy (data, buffer + TKN_OFFS_DATA_START, TKN_PACKET_SIZE);
        data[TKN_PACKET_SIZE] = '\0';
        printf ("%-16s\t", data);
    }
    else
    {
        int i;
        for (i = TKN_OFFS_DATA_START; i <= TKN_OFFS_DATA_STOP; i++)
            TKN_PrintByte (buffer[i], 0);
        printf("\t");
    }

    return 0;
}

int TKN_PrintByte (BYTE c, int forceHex)
{
    if (!forceHex && isprint (c))
        printf ("%1c", c);
    else
        printf ("$%-2X", c);

    return 0;
}

int TKN_ExportPackets ()
{
  /* Create packets dir if not exist */
  char packetDir[50] = "packets";
  
  #ifdef __linux__
  mkdir(packetDir, (mode_t) 0777);
  #else
  CreateDirectory( packetDir, NULL);
  #endif
  
  char* fileNames[3] = 
    { "DATA.hex", 
      "ACK.hex", 
      "TOKEN.hex" 
    };
    
  int lengths[3] = 
    { sizeof(DATA_PACKET), 
      sizeof(ACK_PACKET), 
      sizeof(TOKEN_PACKET) 
    };
    
  BYTE* packets[3] = 
    { DATA_PACKET, 
      ACK_PACKET, 
      TOKEN_PACKET 
    };
  
  /* Create the 3 packet files */    
  FILE *fd = NULL;
  int i,j;
  strcat(packetDir, "/");
  char *flnm_ptr = packetDir + strlen(packetDir);


  for (i=0; i<3; i++){
    strcpy(flnm_ptr, fileNames[i]);
    fd = fopen(packetDir, "w");
    for (j=0; j<lengths[i]; j++) fputc(packets[i][j], fd);
  }

  return 0;
}

/**
 * 1. Open the serial port
 * 2. Assign the node ID
 */
int TKN_Init (int port, int baud, BYTE id)
{
    PORT_NUM = port;
    MY_ID = id;
    PACKET_COUNTER = 0;

    if (!OpenComport (PORT_NUM, baud, TKN_READ_TIMEOUT) == 1){
        return 1; //error
    }
    else 
    {
      printf ("COM PORT %d OPENED SUCCESFULLY\n\n", port);
      printf ("D  -> Send data \n>  -> Send token\nE  -> Exit\n\n");
      TKN_PrintCols ();
      
      return 0; //success
    }
}

/**
 * Close the serial port.
 */
int TKN_Close ()
{
    CloseComport (PORT_NUM);
    printf ("\nPORT %d CLOSED.\n", PORT_NUM);
    return 0;
}


/* TKN Protocol Implementation */

/**
 * Listen to the network channel
 * until to receive a packet which
 * is for us.
 */
int TKN_Receive ()
{
    static BYTE RX_Buffer[TKN_OFFS_DATA_EOF + 1];	// Receive buffer. Its size is determined by the largest packet which is the data packet.
    int pLength, pAttempts;
    BYTE pType;

    do
    {
        pLength = 0;
        pAttempts = 0;
        pType = TKN_TYPE_NONE;
        memset (RX_Buffer, 0, sizeof (RX_Buffer));

        /* Receive the first byte */
        PollComport (PORT_NUM, &RX_Buffer[1], 1);

        /* Ensure that -> BYTE#0: HEADER && BYTE#2: valid PACKET_TYPE */
        do
        {
            #ifdef ECHO_ATTEMPTS
            printf (".");
            #endif
            RX_Buffer[0] = RX_Buffer[1];
            PollComport (PORT_NUM, &RX_Buffer[1], 1);
            pType = RX_Buffer[1];
            if (!
            (pType == TKN_TYPE_DATA || pType == TKN_TYPE_ACK
            || pType == TKN_TYPE_TOKEN))
            pType = TKN_TYPE_NONE;
        }
        while (pAttempts++ < TKN_MAX_ATTEMPTS && (!(RX_Buffer[0] == 0x00 && pType != TKN_TYPE_NONE)));

        /* Here either I detected a valid data type, or the max attempts were exhausted */
        if (RX_Buffer[0] == 0x00 && pType != TKN_TYPE_NONE)
        {
            /* Determine packet length */
            switch (pType)
            {
                case TKN_TYPE_DATA:
                pLength = TKN_OFFS_DATA_EOF + 1;
                break;
                case TKN_TYPE_ACK:
                pLength = TKN_OFFS_ACK_EOF + 1;
                break;
                case TKN_TYPE_TOKEN:
                pLength = TKN_OFFS_TOKEN_EOF + 1;
                break;
                default:
                pLength = 0;
            }

            /* Receive the rest of the packet */
            pAttempts = 0;
            int remaining = pLength - 2, rec = 0;
            BYTE *RX_Buffer_tmp = RX_Buffer + 2;
            while (remaining > 0 && pAttempts < (TKN_MAX_ATTEMPTS << 2)) // Be more tolerant compared to the first byte max-Attempts
                {
                #ifdef ECHO_ATTEMPTS
                printf (",");
                #endif
                rec = PollComport (PORT_NUM, RX_Buffer_tmp, remaining);
                if (rec < 0)
                break;
                remaining -= rec;
                RX_Buffer_tmp += rec;
                pAttempts++;
            }

            if (remaining > 0)
            return -2;

            /* Analyze the packet */
            if (pType != TKN_TYPE_TOKEN)	// The Token is always for the one who receives it, so no need to check receivers etc.
            {
                if (RX_Buffer[TKN_OFFS_SENDER] != MY_ID)	// OK, I didnt send this packet!
                {
                    if (RX_Buffer[TKN_OFFS_RECEIVER] != MY_ID)	// Packet NOT for me!! // Forward and keep receiving!!!
                    {
                        SendBuf (PORT_NUM, RX_Buffer, pLength);
                        #ifdef ECHO_EVENTS
                        printf ("F> ");
                        #endif
                        #ifdef ECHO_DATA
                        TKN_PrintDataPacket (RX_Buffer, 1, 1);
                        #endif
                        continue;
                    }
                }
                else		// This packet was sent by me and apparently nobody received it!
                {
                    #ifdef ECHO_EVENTS
                    printf ("L< ");
                    #endif
                    #ifdef ECHO_DATA
                    TKN_PrintDataPacket (RX_Buffer, 1, 0);
                    #endif
                    return -1;
                }
            }

            /* Here I have a packet which is ~FOR ME~ */
            int i;
            switch (pType)
            {
            case TKN_TYPE_DATA:
                for (i = TKN_OFFS_DATA_START; i <= TKN_OFFS_DATA_STOP; i++)
                    RX_QUEUE_DATA[i - TKN_OFFS_DATA_START] = RX_Buffer[i];	//Extract the data from tha packet and store it in a buffer
                
                RX_QUEUE_ID[RX_PENDING] = RX_Buffer[TKN_OFFS_SENDER];
                RX_PENDING++;
                
                #ifdef ECHO_EVENTS
                printf ("D< ");
                #endif
                #ifdef ECHO_DATA
                TKN_PrintDataPacket (RX_Buffer, 1, 1);
                #endif
                TKN_SendAckPacket (RX_Buffer[TKN_OFFS_SENDER], MY_ID,
                RX_Buffer[TKN_OFFS_PACKET_ID]);
                continue;
            case TKN_TYPE_ACK:
                #ifdef ECHO_EVENTS
                printf ("A< ");
                #endif
                break;
            case TKN_TYPE_TOKEN:
                #ifdef ECHO_TOKENS
                printf ("T< ");
                #endif
                break;
            }
        }
        else
        {
            printf (" no response\n");
        }

        return (int) pType;

    }
    while (1);
}

/**
 * Pass the token.
 */
int TKN_PassToken ()
{
    /* Send the TOKEN */
    SendBuf (PORT_NUM, TOKEN_PACKET, sizeof (TOKEN_PACKET));
    #ifdef ECHO_TOKENS
    printf ("T> ");
    #endif

    return 0;
}

/**
 * Send ACK
 */
int TKN_SendAckPacket (BYTE to, BYTE from, BYTE pack_id)
{
    /* Fill the ACK_PACKET */
    ACK_PACKET[TKN_OFFS_RECEIVER] = to;
    ACK_PACKET[TKN_OFFS_SENDER] = from;
    ACK_PACKET[TKN_OFFS_PACKET_ID] = pack_id;

    /* Send it */
    SendBuf (PORT_NUM, ACK_PACKET, sizeof (ACK_PACKET));

    #ifdef ECHO_EVENTS
    printf ("A> ");
    #endif

    return 0;
}

/**
 * Send the 16 bytes of a data buffer.
 */
int TKN_Send (BYTE * data, BYTE to)
{
    return TKN_SendDataPacket (data, to, MY_ID, PACKET_COUNTER++);
}

/**
 * Send the 16 bytes of a data buffer.
 */
int TKN_SendDataPacket (BYTE * data, BYTE to, BYTE from, BYTE pack_id)
{
    /* Put the data into the DATA_PACKET */
    memcpy (DATA_PACKET + TKN_OFFS_DATA_START, data, TKN_PACKET_SIZE);

    DATA_PACKET[TKN_OFFS_SENDER] = from;
    DATA_PACKET[TKN_OFFS_RECEIVER] = to;
    DATA_PACKET[TKN_OFFS_PACKET_ID] = pack_id;

    /* Send it */
    SendBuf (PORT_NUM, DATA_PACKET, sizeof (DATA_PACKET));

    #ifdef ECHO_EVENTS
    printf ("D> ");
    #endif

    /* Edw perimenw na lavw EITE to ACK apo ton paralipti tou paketou pou esteila EITE to idio to paketo. */
    int ansPack = TKN_Receive ();

    if (ansPack == TKN_TYPE_ACK)
        return 0;
    else
        return 1;
}

/**
 * Calculate the CRC and check the integrity of a data packet.
 */
int TKN_IsDataValid (BYTE * data, BYTE checkByte)
{
    /* Implement the check */
    return 1;
}


/* TKN Client functions */
int TKN_PopData (BYTE * cpBuf)
{
    if (RX_PENDING > 0)
    {
        memcpy (cpBuf, RX_QUEUE_DATA, TKN_PACKET_SIZE);
        BYTE senderId = RX_QUEUE_ID [RX_PENDING-1];
        RX_PENDING--;
        
        return (int) senderId;
    }
    
    return -1;
}

int TKN_PushData (BYTE * cpBuf, BYTE recipientId)
{
    if (TX_PENDING < TKN_QUEUE_SIZE)
    {
        memcpy (TX_QUEUE_DATA + (TKN_PACKET_SIZE * TX_PENDING), cpBuf, TKN_PACKET_SIZE);
        TX_QUEUE_ID [TX_PENDING] = recipientId;
        TX_PENDING++;
        return TX_PENDING;
    }
    
    return -1;
}

static void* TKN_Run(void* params)
{
    while (TKN_Running) 
    {
        if (TX_PENDING > 0) {
            TKN_Send ( TX_QUEUE_DATA + (TKN_PACKET_SIZE * (TX_PENDING-1)), TX_QUEUE_ID [TX_PENDING-1]);
            TX_PENDING--;
        }
        
        TKN_PassToken();
        if (TKN_Receive() == TKN_TYPE_TOKEN)
            TKN_TokenCount++;
        else
            printf("\n>> Did not get the token back! \n");
            
        fflush (stdout);
    }
    
    #ifdef TKN_DEBUG
    printf("\n\n>> TKN_Thread exited normally. \n");
    printf(">> TX_PENDING: %d \n", TX_PENDING);
    printf(">> RX_PENDING: %d \n", RX_PENDING);
    #endif
    
    return 0;
}

int TKN_Start()
{
    if (!TKN_Running)
    {
        TKN_Running=1;
        if ( pthread_create (&TKN_Thread, NULL, &TKN_Run, NULL))
            TKN_Running=0;
        return !TKN_Running;
    }
    else return -1;
}

int TKN_Stop()
{
    if (TKN_Running)
    {
        TKN_Running=0;
        if ( pthread_join(TKN_Thread, NULL)) 
            TKN_Running=1;
        
        return TKN_Running;
    }
    else return -1;
}

int TKN_GetTokenCount()
{
    return TKN_TokenCount;
}
