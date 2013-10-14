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
#include "TKN_Queue.h"
#include "rs232.h"

//#define TKN_DEBUG
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
static TKN_Queue RX_QUEUE;
static TKN_Queue TX_QUEUE;

/* Params */
static int PORT_NUM;
static BYTE MY_ID;

/* Stats */
static int TKN_TokenCount=0;

/* Status Variables */
static volatile int TKN_Running=0;
#ifdef __linux__
static pthread_t TKN_Thread;
#else
static HANDLE TKN_Thread;
#endif

/* Static Function Prototypes */
static int   TKN_SendAckPacket (BYTE to, BYTE from, BYTE pack_id);
static int   TKN_IsDataValid (BYTE *, BYTE);
static int   TKN_PrintDataPacket (BYTE *, int, int);
static int   TKN_PrintByte (BYTE c, int forceHex);
static char* stripLF(char *str);
#ifdef __linux__
static void* TKN_Run(void* params);
#else
static DWORD WINAPI TKN_Run (LPVOID params);
#endif

static void(*recTokenCallback)(void);
static void(*recDataCallback)(void);

/* Print Functions */

static char* stripLF(char *str)
{
    int len = strlen(str);
    if (len>0 && str[len-1] == '\n') str[len-1]= '\0';
    return str;
}

int TKN_PrintCols ()
{
    printf ("FROM\tTO\tID\tDATA \n");
    printf ("-------------------------------------------\n");
    return 0;
}

int TKN_PrintDataPacket (BYTE * buffer, int details, int bin)
{
    static char data[TKN_DATA_SIZE + 1];    // +1 for the zero char

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
        memcpy (data, buffer + TKN_OFFS_DATA_START, TKN_DATA_SIZE);
        data[TKN_DATA_SIZE] = '\0';
        printf ("%-16s\t", data);
    }
    else
    {
        int i;
        for (i = TKN_OFFS_DATA_START; i <= TKN_OFFS_DATA_STOP; i++)
            TKN_PrintByte (buffer[i], bin==2);
        printf("\t");
    }

    return 0;
}

int TKN_PrintByte (BYTE c, int forceHex)
{
    if (!forceHex && isprint (c))
        printf ("%-4c", c);
    else
        printf ("$%-2X ", c);

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



/* TKN Protocol Implementation */

/**
 * Listen to the network channel
 * until to receive a packet which
 * is for us.
 *
 * @return BYTE type of received packet.
 */
int TKN_Receive ()
{
    static BYTE RX_Buffer[TKN_OFFS_DATA_EOF + 1];   // Receive buffer. Its size is determined by the largest packet which is the data packet.
    static TKN_Data rxdata;
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
        if ( !(RX_Buffer[0] == 0x00 && pType != TKN_TYPE_NONE)) {
            #ifdef TKN_DEBUG
            printf (" no response\n");
            #endif
    }
    else
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
                return TKN_TYPE_NONE;

            /* Analyze the packet */
            if (pType != TKN_TYPE_TOKEN) // The Token is always for the one who receives it, so no need to check receivers etc.
            {
                if (RX_Buffer[TKN_OFFS_SENDER] != MY_ID) // OK, I didnt send this packet!
                {
                    if (RX_Buffer[TKN_OFFS_RECEIVER] != MY_ID) // Packet NOT for me!! // Forward and keep receiving!!!
                    {
                        SendBuf (PORT_NUM, RX_Buffer, pLength);
                        #ifdef ECHO_EVENTS
                        printf ("F> ");
                        #endif
                        #ifdef ECHO_DATA
                        TKN_PrintDataPacket (RX_Buffer, 1, 2);
                        #endif
                        continue;
                    }
                }
                else // This packet was sent by me and apparently nobody received it!
                {
                    #ifdef ECHO_EVENTS
                    printf ("L< ");
                    #endif
                    #ifdef ECHO_DATA
                    TKN_PrintDataPacket (RX_Buffer, 1, 2);
                    #endif
                    return TKN_TYPE_DATA;
                }
            }

            /* Here I have a packet which is ~FOR ME~ */
            int i;
        switch (pType)
            {
            case TKN_TYPE_DATA:
                if (TKN_IsDataValid(RX_Buffer+TKN_OFFS_DATA_START, RX_Buffer[TKN_OFFS_CONTROL]))
                {
                    for (i = TKN_OFFS_DATA_START; i <= TKN_OFFS_DATA_STOP; i++)
                        rxdata.data[i-TKN_OFFS_DATA_START] = RX_Buffer [i]; //Extract the data from tha packet and store it in a buffer

                    if ( !TKN_Queue_IsFull(&RX_QUEUE))
                        TKN_Queue_Push(&RX_QUEUE, &rxdata, RX_Buffer[TKN_OFFS_SENDER]);

                    #ifdef ECHO_EVENTS
                    printf ("D< ");
                    #endif
                    #ifdef ECHO_DATA
                    TKN_PrintDataPacket (RX_Buffer, 1, 2);
                    #endif
                    if (recDataCallback) recDataCallback();
                    TKN_SendAckPacket (RX_Buffer[TKN_OFFS_SENDER], MY_ID, RX_Buffer[TKN_OFFS_PACKET_ID]);
                }
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
                if(recTokenCallback) recTokenCallback();
                break;
            }
        }

        // Either return TKN_TYPE_NONE which will mean timeout
        // or a valid TKN_TYPE_XXXX
        return (int) pType;

    } while (1);

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
int TKN_SendDataPacket (TKN_Data *data, BYTE to)
{
    /* Put the data into the DATA_PACKET */
    memcpy (DATA_PACKET + TKN_OFFS_DATA_START, data, TKN_DATA_SIZE);

    DATA_PACKET[TKN_OFFS_SENDER] = MY_ID;
    DATA_PACKET[TKN_OFFS_RECEIVER] = to;
    DATA_PACKET[TKN_OFFS_PACKET_ID] = PACKET_COUNTER++;

    /* Send it */
    SendBuf (PORT_NUM, DATA_PACKET, sizeof (DATA_PACKET));

    #ifdef ECHO_EVENTS
    printf ("D> ");
    #endif

    /* Edw perimenw na lavw EITE to ACK apo ton paralipti tou paketou pou esteila
       EITE to idio to paketo. */
    int ansPack = TKN_Receive ();

    if (ansPack == TKN_TYPE_ACK)
        return 0;
    else if (ansPack == TKN_TYPE_DATA)
        return 1;
    else if (ansPack == TKN_TYPE_NONE)
        return -1;
    else
        return -1;
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

/**
 * 1. Open the serial port
 * 2. Assign the node ID
 */
int TKN_Init (int port, int baud, BYTE id, void (*_recTokenCallback)(void), void (*_recDataCallback)(void) )
{
    PORT_NUM = port;
    MY_ID = id;
    PACKET_COUNTER = 0;

    if (OpenComport(port, baud, TKN_READ_TIMEOUT) == 1){
        #ifdef TKN_DEBUG
        printf ("Cannot open PORT%d\n", port);
        #endif
        return 1; //error
    }
    else
    {
        TKN_Queue_Init( &RX_QUEUE, TKN_QUEUE_SIZE);
        TKN_Queue_Init( &TX_QUEUE, TKN_QUEUE_SIZE);

        recTokenCallback = _recTokenCallback;
        recDataCallback = _recDataCallback;

        return 0; //success
    }
}

int TKN_InitWithArgs (int argc, char *argv[])
{
    int portNum, baud, node_id;

    if (argc > 1)
        portNum = atoi (argv[1]);   // In conversion error port_num=0
    else
        portNum = TKN_PORT_DEFAULT;

    if (argc > 2)
        baud = atoi (argv[2]);
    else
        baud = TKN_BAUD_DEFAULT;

    node_id = TKN_ID_DEFAULT;

    return TKN_Init (portNum, baud, node_id, NULL, NULL);
}

/**
 * Close the serial port.
 */
int TKN_Close ()
{
    CloseComport (PORT_NUM);
    #ifdef TKN_DEBUG
    printf ("\n>> PORT %d CLOSED.\n", PORT_NUM);
    #endif
    TKN_Queue_Free (&TX_QUEUE);
    TKN_Queue_Free (&RX_QUEUE);
    return 0;
}

BYTE* TKN_ListActiveNodes(BYTE maxID)
{
    BYTE *nodes = NULL;

    if ( !TKN_Running) //if running we cannot <<play>> with our nodes!
    {
        /* Discover which nodes exist on the network right now. */
        nodes = malloc(maxID*sizeof(BYTE));
        memset(nodes, 0, maxID*sizeof(BYTE));

        TKN_Data testData;
        strcpy(&testData, "V:");

        BYTE possibleNode;
        int i=0;
        for (possibleNode=2; possibleNode<maxID; possibleNode++){
            int ans = TKN_SendDataPacket(&testData , possibleNode);
            if (ans == 0)
                nodes[i++] = possibleNode; // TODO: check nodes' array bounds
            else if (ans>0)
                continue;
            else if (ans<0){ // in this case there is no connectivity, so its pointless to resume discovering.
                free(nodes);
                return NULL;
            }
        }
    }

    return nodes;
}

#ifdef __linux__ /* linux */

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

        return TKN_Running; // 0 if thread is succesfully stopped.
    }
    else return -1;
}


#else /* windows */

int TKN_Start()
{
    if (!TKN_Running) {
        TKN_Running=1;
        if ( !(TKN_Thread = CreateThread( NULL, 0, &TKN_Run, NULL, 0, NULL)))
            TKN_Running=0;
    }

    return !TKN_Running;
}

int TKN_Stop()
{
    if (TKN_Running) {
        TKN_Running=0;
        if (WaitForSingleObject (TKN_Thread, 1000))
            TKN_Running=1;
        else
            CloseHandle(TKN_Thread);
    }

    return TKN_Running;
}

#endif

/**
 * @return The id of the sender if a packet was received
 *         -1 if the queue was empty.
 */
int TKN_PopData (TKN_Data *cBuf)
{
    if ( !TKN_Queue_IsEmpty(&RX_QUEUE) )
    {
        return (int) TKN_Queue_Pop (&RX_QUEUE, cBuf);
    }

    return -1;
}

int TKN_PushData (TKN_Data * cBuf, BYTE recipientId)
{
    if ( !TKN_Queue_IsFull( &TX_QUEUE) )
    {
        TKN_Queue_Push (&TX_QUEUE, cBuf, recipientId);
        return 0;
    }

    return -1;
}

int TKN_PushDataOvw (TKN_Data * cBuf, BYTE recipientId)
{
    TKN_Queue_Push (&TX_QUEUE, cBuf, recipientId);
    return 0;
}

int TKN_SendString (char * str, BYTE dest_id)
{
    TKN_Data lineBuf;
    int rem = strlen(str);

    while (rem>0) {
        strncpy( (char*) &lineBuf, str, sizeof(lineBuf));
        str += sizeof(lineBuf);
        rem -= sizeof(lineBuf);
        TKN_PushData ( &lineBuf, dest_id);
    }

    return 0;
}

int TKN_WaitString(char *ready_str)
{
    char recData[sizeof(TKN_Data)+1];
    recData[sizeof(TKN_Data)] = '\0';

    do{
        while (TKN_PopData ( (TKN_Data*) recData) <0 );
        // dont print your tries ... if (strlen(recData)) puts(stripLF(recData));
    } while (strncmp ( recData, ready_str, sizeof(TKN_Data)) != 0);
    #ifdef TKN_DEBUG
    printf(">> Received string: +%s+\n",stripLF(recData));
    #endif
    return 0;
}

int TKN_GetTokenCount()
{
    return TKN_TokenCount;
}

#ifdef __linux__
void* TKN_Run(void* params)
#else
DWORD WINAPI TKN_Run (LPVOID params)
#endif
{
    TKN_Data data;
    BYTE rid;

    while (TKN_Running)
    {
        TKN_PassToken();

        if (TKN_Receive() == TKN_TYPE_TOKEN){
            TKN_TokenCount++;

            if ( !TKN_Queue_IsEmpty(&TX_QUEUE) ) {
                rid = TKN_Queue_Pop(&TX_QUEUE, &data);
                TKN_SendDataPacket ( &data, rid);
            }
        }

        fflush (stdout);
    }

    #ifdef TKN_DEBUG
    printf("\n\n>> TKN_Thread exited normally. \n");
    printf(">> TX_PENDING: %d \n", TX_QUEUE.count);
    printf(">> RX_PENDING: %d \n", RX_QUEUE.count);
    #endif
    fflush (stdout);
    return 0;
}

