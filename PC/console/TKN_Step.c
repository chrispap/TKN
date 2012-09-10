#include <stdio.h>
#include <stdlib.h>

#include "../lib/TKN.h"
#include "../lib/TKN_Util.h"

/* Gloabals */
BYTE dest_id;

int main (int argc, char *argv[])
{
    int action = 'x';
    
    if (TKN_InitWithArgs (argc, argv) != 0) 
        exit (1);
    
	dest_id = (argc > 3)? atoi (argv[3]) : TKN_DEST_ID_DEFAULT;
	
	printf (">> TKN opened succesfully.\n");	
	printf ("D  -> Send data \n>  -> Send token\nE  -> Exit\n\n");
	TKN_PrintCols();
	
    while ( (action = getKey ("Ee Dd .>")) != 'e' )
    {
        if (action == 'd') 
            TKN_SendDataPacket ((TKN_Data*)"__From Laptop___", dest_id);

        TKN_PassToken ();
        TKN_Receive ();
    }

    TKN_Close ();
    return 0;
}
