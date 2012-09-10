#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "../lib/TKN.h"
#include "../lib/TKN_Util.h"

/* Gloabals */
BYTE dest_id;
static time_t time_start, time_end;

int main (int argc, char *argv[])
{
    if (TKN_InitWithArgs(argc, argv))
        exit (1);
	
	dest_id = (argc > 3)? atoi (argv[3]) : TKN_DEST_ID_DEFAULT;
	
    printf (">> TKN opened succesfully.\n");
    printf ("D  -> Send data \nE  -> Exit\n\n");
    TKN_PrintCols();

    /* Start the network */
    time_start = time(NULL);
    TKN_Start();
    
    int action;
    while ( (action = getKey ("Ee Dd")) != 'e' )
    {
        if (action == 'd') 
            TKN_PushData((TKN_Data*)"__From Laptop___", dest_id);
        
        int rid;
        BYTE rbuf[TKN_DATA_SIZE + 1]; // One extra byte for zero termination.
        rbuf[TKN_DATA_SIZE]= '\0';
        
        rid = TKN_PopData((TKN_Data*)rbuf);
        if (rid>0)
            printf("Received from: %2d Data: %16s \n", rid, rbuf);
        else printf("No-Data \n");
        
    }

    /* Shut downn the network */
    TKN_Stop();
    TKN_Close();
    time_end = time (NULL);
    printf (">> Ellapsed time: %ld sec \n", time_end - time_start);
    printf (">> Token Counter: %d \n", TKN_GetTokenCount() );
    
    return 0;
}
