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

int main (int argc, char *argv[]) 
{
    FILE *hexFile;
    char *flname = "data/codebig.hex";
    char *mcuReadyStr = "-MCU-READY------";
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
      TKN_SendString (hexLine, dest_id);
      TKN_WaitString(mcuReadyStr); 
    }
	
    /* Shut down the network */
    TKN_Stop();
    TKN_Close ();
	time_end = time (NULL);
    printf (">> Ellapsed time: %ld sec \n", time_end - time_start);
    printf (">> Token Counter: %d \n", TKN_GetTokenCount() );
    return 0;
}
