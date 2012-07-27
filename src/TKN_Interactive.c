#include <stdio.h>
#include <stdlib.h>

#include "TKN.h"

/* Gloabals */
extern BYTE dest_id;
static time_t time_start, time_end;

int main (int argc, char *argv[])
{
    if (init (argc, argv) != 0) 
        exit (1);

    /* Start the network */
    time_start = time(NULL);
    TKN_Start();
    
    int action;
    while ( (action = getKey ("Ee Dd")) != 'e' )
    {
        if (action == 'd') 
            TKN_PushData("__From Laptop___", dest_id);
    
    }

    /* Shut downn the network */
    TKN_Stop();
    TKN_Close();
    time_end = time (NULL);
         
    printf ("\nEllapsed time: %ld sec \n", time_end - time_start);
    printf ("Token Counter: %d \n", TKN_TokenCount() );
    
    return 0;
}
