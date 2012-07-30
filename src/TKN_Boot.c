#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <windows.h>

#include "TKN.h"
#include "TKN_Util.h"

#define HEXLINE_SIZE 1024

extern BYTE dest_id;
static time_t time_start, time_end;

int sendHexLine (char * hexLine, BYTE dest_id) 
{
    TKN_Data lineBuf;
    int rem = strlen(hexLine);
	
    while (rem>0) {
		strncpy( (char*) &lineBuf, hexLine, sizeof(lineBuf));
		hexLine += sizeof(lineBuf);
		rem -= sizeof(lineBuf);
		TKN_PushData ( &lineBuf, dest_id);
    }
    
    return 0;
}

char* stripNL(char *str)
{
	int len = strlen(str); 
    if (len>0 && str[len-1] == '\n') str[len-1]= '\0';
    return str;
}

int waitforString(char *ready_str)
{
  char recData[sizeof(TKN_Data)+1];
  recData[sizeof(TKN_Data)] = '\0';
  
  do{
    while (TKN_PopData ( (TKN_Data*) recData) <0 );
	if (strlen(recData)) puts(stripNL(recData));
  } while (strncmp ( recData, ready_str, sizeof(TKN_Data)) != 0);
  
  return 0;	
}


int main (int argc, char *argv[]) 
{
    FILE *hexFile;
    char *flname = "data/test.hex";
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
	time_start = time(NULL);
	
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
	time_end = time (NULL);
    printf (">> Ellapsed time: %ld sec \n", time_end - time_start);
    printf (">> Token Counter: %d \n", TKN_GetTokenCount() );
    return 0;
}
