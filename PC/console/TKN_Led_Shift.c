#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef __linux
#include <signal.h>
#include <unistd.h>
#define Sleep(x) usleep((x)*1000)
#else
#include <windows.h>
#endif

#include "../lib/TKN.h"
#include "../lib/TKN_Util.h"

#define N 3

int main (int argc, char *argv[])
{
    /* Start the network */
    if (TKN_InitWithArgs(argc, argv)) exit (1);
    else printf (">> TKN opened succesfully.\n");
    TKN_Start ();
    TKN_PrintCols();

    BYTE nodes[N] = {2,3,4};
    BYTE led[N] = {0,0,0};
    TKN_Data packet;
    const unsigned long long pat0=0x07;
    unsigned long long pat=pat0;
    int i;

    memset(&packet,0,sizeof(packet));

    for (i=0; i<N; ++i) {
        TKN_SendString("R:", nodes[i]); // Send `Run` command
        while (TKN_PushData (&packet, nodes[i])); // Send a zero packet
    }

    do
    {
        /* Update LEDs in all nodes */
        for (i=0; i<N; ++i) {
            packet.data[0] = *(((BYTE*)&pat)+i)  |  *(((BYTE*)&pat)+(i+3));
            if (led[i] != packet.data[0]) {
                led[i] = packet.data[0];
                while (TKN_PushData (&packet, nodes[i]));
            }
        }

        /* Advance pattern */
        pat = pat==pat0<<23? pat0 : pat<<1;
        Sleep(100);

    } while (1);

    while (TKN_PopData(&packet) > 0 ); // Pop all the data out of the Rx Queue

    /* Shut downn the network */
    TKN_Stop ();
    TKN_Close ();

    return 0;
}
