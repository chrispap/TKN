#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#ifndef __linux__
    #include <conio.h>
    #include <windows.h>
#else
    int getch ();
#endif

#include "TKN.h"
#include "rs232.h"

#define TKN_DEBUG
#ifdef TKN_DEBUG
    #define ECHO_ATTEMPTS	    //Define this in order to print a dot in every attempt to detect valid packet.
    #define ECHO_TOKENS		    //Define this in order to print a little string in every sent/received token.
    #define ECHO_DATA		    //Define this in order to print a little string in every sent/received data.
    #define ECHO_ACKS		    //Define this in order to print a little string in every sent/received ack.
#endif

/* The serial port of the PC*/
static int PORT_NUM;

/* Every node MUST have an id! */
static BYTE MY_ID;

/* Working parameters */
static const int maxAttempts = 10;

/* The necessary packet buffers */
static BYTE DATA_PACKET[TKN_OFFS_DATA_EOF + 1]    = { 0x00, TKN_TYPE_DATA, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF };
static BYTE ACK_PACKET[TKN_OFFS_ACK_EOF + 1]      = { 0x00, TKN_TYPE_ACK, 0x00, 0x00, 0x00, 0xFF };
static BYTE TOKEN_PACKET[TKN_OFFS_TOKEN_EOF + 1]  = { 0x00, TKN_TYPE_TOKEN, 0x00, 0x00, 0xFF };
static BYTE RX_DATA[TKN_PACKET_SIZE];
static BYTE PACKET_COUNTER;
static int RX_PENDING = 0;  


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
    }

    fflush (stdout);
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


/* TKN Protocol Implementation */

/**
 * 1. Open the serial port
 * 2. Assign the node ID
 */
int TKN_Init (int port, int baud, BYTE id)
{
    PORT_NUM = port;
    MY_ID = id;
    PACKET_COUNTER = 0;

    /* Open the serial port */
    if (OpenComport (PORT_NUM, baud, TKN_READ_TIMEOUT) == 1)
        return 1; //error
    else
        return 0; //success
}

/**
 * Close the serial port.
 */
int TKN_Close ()
{
    /* Close the serial port */
    CloseComport (PORT_NUM);
    printf ("\nPORT %d CLOSED.\n", PORT_NUM);
    return 0;
}

/**
 * Receive one packet.
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
            fflush (stdout);
            #endif
            RX_Buffer[0] = RX_Buffer[1];
            PollComport (PORT_NUM, &RX_Buffer[1], 1);
            pType = RX_Buffer[1];
            if (!
            (pType == TKN_TYPE_DATA || pType == TKN_TYPE_ACK
            || pType == TKN_TYPE_TOKEN))
            pType = TKN_TYPE_NONE;
        }
        while (pAttempts++ < maxAttempts && (!(RX_Buffer[0] == 0x00 && pType != TKN_TYPE_NONE)));

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
            while (remaining > 0 && pAttempts < (maxAttempts << 2)) // Be more tolerant compared to the first byte max-Attempts
                {
                #ifdef ECHO_ATTEMPTS
                printf (",");
                fflush (stdout);
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
                        #ifdef ECHO_DATA
                        printf ("F> ");
                        #endif
                        TKN_PrintDataPacket (RX_Buffer, 1, 1);
                        continue;
                    }
                }
                else		// This packet was sent by me and apparently nobody received it!
                {
                    #ifdef ECHO_ACKS
                    printf ("L< ");
                    #endif
                    TKN_PrintDataPacket (RX_Buffer, 1, 0);
                    return -1;
                }
            }

            /* Here I have a packet which is ~FOR ME~ */
            int i;
            switch (pType)
            {
                case TKN_TYPE_DATA:
                for (i = TKN_OFFS_DATA_START; i <= TKN_OFFS_DATA_STOP; i++)
                RX_DATA[i - TKN_OFFS_DATA_START] = RX_Buffer[i];	//Extract the data from tha packet and store it in a buffer
                RX_PENDING = 1;
                #ifdef ECHO_DATA
                printf ("D<");
                #endif
                TKN_PrintDataPacket (RX_Buffer, 1, 1);
                TKN_SendAckPacket (RX_Buffer[TKN_OFFS_SENDER], MY_ID,
                RX_Buffer[TKN_OFFS_PACKET_ID]);
                continue;
                case TKN_TYPE_ACK:
                #ifdef ECHO_ACKS
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
            fflush (stdout);
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

    #ifdef ECHO_ACKS
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
    strncpy (DATA_PACKET + TKN_OFFS_DATA_START, data, TKN_PACKET_SIZE);

    DATA_PACKET[TKN_OFFS_SENDER] = from;
    DATA_PACKET[TKN_OFFS_RECEIVER] = to;
    DATA_PACKET[TKN_OFFS_PACKET_ID] = pack_id;

    /* Send it */
    SendBuf (PORT_NUM, DATA_PACKET, sizeof (DATA_PACKET));

    #ifdef ECHO_DATA
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
        memcpy (cpBuf, RX_DATA, TKN_PACKET_SIZE);
        RX_PENDING = 0;
        return RX_PENDING;
    }
    else
        return -1;
}
