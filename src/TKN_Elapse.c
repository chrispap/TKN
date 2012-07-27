#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>
#include <unistd.h>

#include "TKN.h"
#include "TKN_Util.h"

extern BYTE dest_id;
static time_t time_start, time_end;

int main (int argc, char *argv[])
{
    if (init (argc, argv)) 
        exit (1);

    TKN_PushData((BYTE*)"__From Laptop___", dest_id);
    
    /* Start the network */
    time_start = time(NULL);
    TKN_Start();
    
    sleep(10);
    
    /* Shut downn the network */
    TKN_Stop();
    TKN_Close();
    time_end = time (NULL);
         
    printf ("\nEllapsed time: %ld sec \n", time_end - time_start);
    printf ("Token Counter: %d \n", TKN_TokenCount() );
    
    return 0;
}
