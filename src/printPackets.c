#include <stdio.h>
#include <stdlib.h>

int main (int argc, char *argv[]) 
{
  int i,j;
  char str[20];
  
  for (i=0; i<1000; i++) {
    sprintf(str, "-Line %4d -", i+1);
    puts(str);
  }
  
  return 0;
}
