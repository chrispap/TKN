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

  /* Start the network */
  time_start = time (NULL);
  TKN_Start ();

  int packC = 0;
  /* Wait 10 sec and then shut it down */
  // sleep (10);
  
  do
  {
    if (TKN_PushData ((TKN_Data *) "__From Laptop___", dest_id) == 0)
    packC++;
  } while ((time (NULL) - time_start) < 10);
  
  
  /* Shut downn the network */
  TKN_Stop ();
  TKN_Close ();
  time_end = time (NULL);

  printf ("\n>> Ellapsed time: %ld sec \n", time_end - time_start);
  printf (">> Token Counter: %d \n", TKN_GetTokenCount ());
  printf (">> Data Packets pushed: %d \n", packC);

  return 0;
}
