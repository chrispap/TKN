#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "TKN.h"
  
#define HEXLINE_SIZE 1024

int readHexLine (FILE * hexFile, char *hexLine);int sendHexLine (BYTE * hexLine, BYTE dest_id);int waitForString (char *);


/* Main */ 
int main (int argc, char *argv[]) 
{      FILE * hexFile;
    char *flname = "data/test1.hex",          *MCU_ready_str = "----------------";
    BYTE hexLine[HEXLINE_SIZE], dest_id = 2;
    int txActive = 0, fileSent = 0, waitMCU = 0;

    /* Init network and open the hex file */ 
    if (init (argc, argv) != 0)
        exit (1);
    if ((hexFile = fopen (flname, "r")) == NULL) { 
        perror("Cannot open data file");
        exit (1);
    }
    /* Send file */ 
    while (!fileSent)
    {        if (!readHexLine (hexFile, hexLine)) 
            fileSent = 1;
                sendHexLine (hexLine, dest_id);
                if (!fileSent) 
            waitForString (MCU_ready_str);    }
    TKN_Close ();    return 0;}


/* Functions */ 
int readHexLine (FILE * hexFile, char *hexLine) 
{    memset (hexLine, 0, HEXLINE_SIZE);
    if (fgets (hexLine, HEXLINE_SIZE, hexFile) == NULL)
        return 0;
    else        return 1;}
 int sendHexLine (BYTE * hexLine, BYTE dest_id) 
{    BYTE lineBuf[TKN_PACKET_SIZE];    int rem;
    while ((rem = strlen (hexLine)) > 0)
    {
        if (rem > TKN_PACKET_SIZE)
        {
            memcpy (lineBuf, hexLine, TKN_PACKET_SIZE);
            hexLine += TKN_PACKET_SIZE;
        } 
        else
        {
            memset (lineBuf, 0, sizeof (lineBuf));            memcpy (lineBuf, hexLine, rem);            hexLine += rem;        }
        TKN_Send (lineBuf, dest_id);
        TKN_PassToken ();	// Pass the token... 
        TKN_Receive ();		// ...and wait to get it back.
    }
    
    return 0;}
int waitForString (char *str) 
{    BYTE recData[TKN_PACKET_SIZE + 1];    memset (recData, 0, sizeof (recData));

    do
    {
        TKN_PassToken ();        TKN_Receive ();        TKN_PopData (recData);    }
    while (strcmp (recData, str) != 0);
    return 0;
}
