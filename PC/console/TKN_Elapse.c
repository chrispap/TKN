#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#ifdef __linux
#include <signal.h>
#include <unistd.h>
#else
#include <windows.h>
#define sleep(x) Sleep(x*1000)
#endif

#include "../lib/TKN.h"
#include "../lib/TKN_Util.h"

#define FULL_LOAD 1
#define SECONDS_TO_RUN 2

BYTE dest_id;
static time_t time_start, time_end;

int main (int argc, char *argv[])
{
    if (TKN_InitWithArgs(argc, argv))
        exit (1);

    printf (">> TKN opened succesfully.\n");

    dest_id = (argc > 3)? atoi (argv[3]) : TKN_DEST_ID_DEFAULT;

    /* Start the network */
    time_start = time (NULL);
    TKN_Start ();
	TKN_PrintCols();
    int packC = 0;
    
    if (FULL_LOAD)  // Send DATA whenever possible!
    {
        do{
            char myData[TKN_DATA_SIZE] = {0};
            while (TKN_PushData ((TKN_Data *) myData, dest_id));
            packC++;
        } while (packC<10); //(time (NULL) - time_start) < SECONDS_TO_RUN);
        
        sleep(1);
        printf("\n>> Receive back \n");

        int i;
        for (i=0; i<packC; i++) {
            TKN_Data recData;
            while (TKN_PopData(&recData) < 0 );
        }
        
    }
    else  // Send nothing
    {
		sleep (SECONDS_TO_RUN);
    }

    /* Shut downn the network */
    TKN_Stop ();
    TKN_Close ();
    time_end = time (NULL);

    printf ("\n>> Ellapsed time: %ld sec \n", time_end - time_start);
    printf (">> Token Counter: %d \n", TKN_GetTokenCount ());
    printf (">> Data Packets pushed: %d \n", packC);

    return 0;
}
