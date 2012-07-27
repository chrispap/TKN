#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#ifndef __linux__
    #include <conio.h>
#else
    #include <termios.h>
    #include <unistd.h>
    #include <sys/ioctl.h>
#endif

#include "TKN.h"
#include "TKN_Util.h"

/* Globals */
BYTE dest_id=0;

/* UI Functions */
int init (int argc, char *argv[])
{
    int portNum, baud, node_id;

    if (argc > 1)
        portNum = atoi (argv[1]);	// In conversion error port_num=0
    else
        portNum = TKN_PORT_DEFAULT;

    if (argc > 2)
        baud = atoi (argv[2]);
    else
        baud = TKN_BAUD_DEFAULT;

    if (argc > 3)
        dest_id = atoi (argv[3]);
    else
        dest_id = TKN_DEST_ID_DEFAULT;

    if (argc > 4)
        node_id = atoi (argv[4]);
    else
        node_id = TKN_ID_DEFAULT;

    if (TKN_Init (portNum, baud, node_id) != 0)
    {
        printf ("Cannot open PORT%d\n", portNum);
        return 1;
    }
    else
    {
        printf ("COM PORT %d OPENED SUCCESFULLY\n\n", portNum);
        printf ("D  -> Send data \n>  -> Send token\nE  -> Exit\n\n");
        TKN_PrintCols ();
        return 0;
    }
    
}

int getKey (char *acceptKeys)
{
    char key, *c;

    if (strlen (acceptKeys) < 1)
    return getch ();

    do
    {
        while (!isprint (key = getch ()));
        c = acceptKeys;
        while (*c != '\0' && key != *(c++));
    }
    while (*(c - 1) != key);

    return tolower (key);
}

#ifdef __linux__
int getch ()
{
    struct termios oldT, newT;
    char c = 0;

    ioctl (0, TCGETS, &oldT);
    newT = oldT;                //Copy current mode
    newT.c_lflag &= ~ECHO;      //echo off
    newT.c_lflag &= ~ICANON;    //one char @ a time
    ioctl (0, TCSETS, &newT);
    int r = read (0, &c, 1);            // Read 1 char
    ioctl (0, TCSETS, &oldT);   //Restore normaal mode
    
    return r<0? r: (int) c;
}
#endif
