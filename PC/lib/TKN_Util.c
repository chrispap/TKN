#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#ifdef __linux__
  #include <termios.h>
  #include <unistd.h>
  #include <sys/ioctl.h>
#else
  #include <conio.h>
#endif

#include "TKN.h"
#include "TKN_Util.h"

/* UI Functions */
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
    int r = read (0, &c, 1);    // Read 1 char
    ioctl (0, TCSETS, &oldT);   //Restore normaal mode
    
    return r<0? r: (int) c;
}
#endif
