#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>

#include "TKN.h"

extern BYTE dest_id;
static time_t time_start, time_end;
static volatile int running;

void interruptHandler (int signum)
{
    running=0;
}

int main (int argc, char *argv[])
{
    if (init (argc, argv)) 
        exit (1);
        
    /* Handler for ctl+C */
    struct sigaction new_action;
    new_action.sa_handler = interruptHandler;
    sigemptyset (&new_action.sa_mask);
    new_action.sa_flags = 0;
    sigaction (SIGINT, &new_action, NULL);

    /* Enter a loop */
    running=1;
    time_start = time(NULL);
    TKN_Start();
    while(running){
        sleep(1);
        
        
    }
    
    /* Shut downn the network */
    TKN_Stop();
    TKN_Close();
    time_end = time (NULL);
         
    printf ("\nEllapsed time: %ld sec \n", time_end - time_start);
    printf ("Token Counter: %d \n", TKN_TokenCount() );
    
    return 0;
}
