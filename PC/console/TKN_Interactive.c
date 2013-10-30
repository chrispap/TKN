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
    printf ("S  -> Send data \nR  -> Receive data \nE  -> Exit\n\n");
    TKN_PrintCols();

    /* Start the network */
    time_start = time(NULL);
    TKN_Start();

    int action;
    printf(">> ACTION:");
    while ( (action = getKey ("Ee Ss Rr")) != 'e' )
    {
        TKN_Data packet;
        int sender_id;

        switch (action) {
            case 's':
                printf("\n>> DATA  : ");
                scanf("%s", packet.data);
                while (TKN_PushData(&packet, dest_id));
                break;
            case 'r':
                sender_id = TKN_PopData(&packet);
                if (sender_id>0) printf("Received from: %2d Data: %16s. \n", sender_id, packet.data);
                else printf("Nothing to receive.\n");
                break;
            default:
                break;
        }
        printf(">> ACTION: ");
    }

    /* Shut downn the network */
    TKN_Stop();
    TKN_Close();
    time_end = time (NULL);
    printf (">> Ellapsed time: %ld sec \n", time_end - time_start);
    printf (">> Token Counter: %d \n", TKN_GetTokenCount() );

    return 0;
}
