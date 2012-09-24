#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
	

	BYTE i=0;
	char *msg  = "__From Laptop";
	char data[16];
	strcpy(data, msg);

    while ( (action = getKey ("Ee Dd .>")) != 'e' )
    {
        if (action == 'd'){
            sprintf(data+strlen("__From Laptop"), "%3d", (unsigned int) ++i);			
			TKN_SendDataPacket ((TKN_Data*) data, dest_id);
		}
		else {
			TKN_PassToken ();
			fflush (stdout);
			TKN_Receive ();
			fflush (stdout);
		}
    }

    TKN_Close ();
    return 0;
}
