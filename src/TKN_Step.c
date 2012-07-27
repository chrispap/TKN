#include <stdio.h>
#include <stdlib.h>

#include "TKN.h"

/* Gloabals */
extern BYTE dest_id;

int main (int argc, char *argv[])
{
    int action = 'x', tokenCounter = 0;
    
    if (init (argc, argv) != 0) 
        exit (1);
    
    while ( (action = getKey ("Ee Dd .>")) != 'e' )
    {
        if (action == 'd') 
            TKN_Send ("__From Laptop___", dest_id);
		
        TKN_PassToken ();
        TKN_Receive ();
        tokenCounter++;
    }

    TKN_Close ();
    return 0;
}
