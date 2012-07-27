#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "TKN.h"
extern BYTE dest_id;
extern pthread_t TKN_Thread;

int main (int argc, char *argv[])
{
    time_t time_start, time_end;

    if (init (argc, argv)) 
        exit (1);

    TKN_PushData("AAAAAAAAAAAAAAAA", dest_id);
    TKN_PushData("BBBBBBBBBBBBBBBB", dest_id);
    TKN_PushData("CCCCCCCCCCCCCCCC", dest_id);
    
    time_start = time(NULL);
    TKN_Start();
    sleep(2);
    TKN_Stop();
    TKN_Close();
    time_end = time (NULL);
    
    /* Print the count of tokens cycles that were completed. */
    printf ("\nEllapsed time: %ld sec \n", time_end - time_start);
    printf ("Token Counter: %d \n", TKN_TokenCount() );
    
    return 0;
}


    //~ void **rv;
    //~ pthread_join(TKN_Thread, rv);
