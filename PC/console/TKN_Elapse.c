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
#define SECONDS_TO_RUN 10

BYTE dest_id;
static time_t time_start, time_end;

int main (int argc, char *argv[])
{
    if (TKN_InitWithArgs(argc, argv))
        exit (1);

    printf (">> TKN opened succesfully.\n");

    dest_id = (argc > 3)? atoi (argv[3]) : TKN_DEST_ID_DEFAULT;

    /* Start the network */
    TKN_Start ();
    TKN_PrintCols();
    int packC = 0;

    time_start = time (NULL);

    if (FULL_LOAD)  // Send DATA whenever possible!
    {
        do{
            char myData[TKN_DATA_SIZE] = "test msg";
            while (TKN_PushData ((TKN_Data *) myData, dest_id));
            packC++;
        } while ((time (NULL) - time_start) < SECONDS_TO_RUN);

        time_end = time (NULL);
        sleep(4); // wait for all the Tx Queue to be sent
        char recData[TKN_DATA_SIZE];
        while (TKN_PopData(recData) > 0 ); // Pop all the data out of the Rx Queue
    }
    else  // Send nothing
    {
        sleep (SECONDS_TO_RUN);
        time_end = time (NULL);
    }

    /* Shut downn the network */
    TKN_Stop ();
    TKN_Close ();


    printf ("\n>> Ellapsed time: %ld sec \n", time_end - time_start);
    printf (">> Token Counter: %d \n", TKN_GetTokenCount ());
    printf (">> Data Packets pushed: %d - (size of tx_queue)\n", packC);

    return 0;
}
