#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "TKN.h"
#include "TKN_Util.h"

#define HEXLINE_SIZE 1024

extern BYTE dest_id;

int sendHexLine (char * hexLine, BYTE dest_id) 
{
    BYTE lineBuf[TKN_DATA_SIZE];
    int rem;

    while ((rem = strlen ((char*)hexLine)) > 0)
    {
	if (rem > TKN_DATA_SIZE)
	{
	    memcpy (lineBuf, hexLine, TKN_DATA_SIZE);
	    hexLine += TKN_DATA_SIZE;
	} 
	else
	{
	    memset (lineBuf, 0, sizeof (lineBuf));
	    memcpy (lineBuf, hexLine, rem);
	    hexLine += rem;
	}

	TKN_PushData ((TKN_Data*) lineBuf, dest_id);
    }
    
    return 0;
}

int waitforString(char *ready_str)
{
  char recData[sizeof(TKN_Data)+1];
  recData[sizeof(TKN_Data)] = '\0';
  
  do{
    while (TKN_PopData ( (TKN_Data*) recData) <0 );
    /* Strip trailing new lines */
    int len = strlen(recData); 
    if (len>0 && recData[len-1] == '\n') recData[len-1]=0;
    len = strlen(recData); 
    if (len>0) puts(recData);
    
  } while (strncmp ( recData, ready_str, sizeof(TKN_Data)) != 0);
  
  return 0;	
}
      
int main (int argc, char *argv[]) 
{
    FILE *hexFile;
    char *flname = "data/100packets.hex";
    char *mcuReadyStr = "----------------";
    char  hexLine[HEXLINE_SIZE];
    int   fileIsRead = 0;
    
    /* Open the file */
    if ((hexFile = fopen (flname, "r")) == NULL) { 
	perror("Cannot open data file");
	exit (1);
    }

    /* Init network */ 
    if (init (argc, argv) != 0)
	exit (1);
    TKN_Start();

    /* Send file */ 
    while (!(fileIsRead = fgets(hexLine, HEXLINE_SIZE, hexFile)==NULL? 1:0))
    {
      /* Send the line */
      sendHexLine (hexLine, dest_id);
      
      /* Wait MCU */
      waitforString(mcuReadyStr); 
    }

    /* Shut down the network */
    TKN_Stop();
    TKN_Close ();
    return 0;
}
