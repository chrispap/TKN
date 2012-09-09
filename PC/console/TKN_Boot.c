#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "../lib/TKN.h"
#include "../lib/TKN_Util.h"

#define HEXLINE_SIZE 1024

BYTE dest_id;
static time_t time_start, time_end;

int main (int argc, char *argv[]) 
{
    FILE *hexFile;
    char *flname = ((argc>4)? argv[4] : "data/test.hex");
    char *mcuReadyStr = "-MCU-READY------";
    char  hexLine[HEXLINE_SIZE];
    int   fileIsRead = 0, lineCounter=0;
    
    /* Open the file */
    if ((hexFile = fopen (flname, "r")) == NULL) {
        perror("Cannot open data file");
        exit (1);
    }

    /* Init network */ 
    if (TKN_InitWithArgs(argc, argv))
        exit (1);

    printf (">> TKN opened succesfully.\n");

    dest_id = (argc > 3)? atoi (argv[3]) : TKN_DEST_ID_DEFAULT;

    TKN_Start();
    time_start = time(NULL);

    /* Send file */ 
    while (!(fileIsRead = fgets(hexLine, HEXLINE_SIZE, hexFile)==NULL? 1:0))
    {
        lineCounter++;
        TKN_SendString (hexLine, dest_id);
        printf(">> Pushed line #%3d\n", lineCounter);
        TKN_WaitString(mcuReadyStr);
    }

    /* Shut down the network */
    TKN_Stop();
    TKN_Close ();
    time_end = time (NULL);
    printf (">> Ellapsed time: %ld sec \n", time_end - time_start);
    printf (">> Token Counter: %d \n", TKN_GetTokenCount() );
    printf (">> Lines sent   : %d \n", lineCounter );
    return 0;
}
