#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "TKN.h"
extern BYTE dest_id;

int main (int argc, char *argv[])
{
    int tokenCounter = 0;
    time_t time_start;

    if (init (argc, argv) != 0) 
        exit (1);

    time_start = time(NULL);
	
    while ( time(NULL) - time_start < 10 )
    {
        TKN_Send ("__From Laptop___", dest_id);
        TKN_PassToken ();
        TKN_Receive ();
        tokenCounter++;
    }
	
    time_t time_end = time (NULL);
    printf ("\nEllapsed time: %ld sec \n", time_end - time_start);
    printf ("Token Counter: %d", tokenCounter);

    TKN_Close ();
    return 0;
}
