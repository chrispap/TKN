#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "TKN.h"

/* The necessary packet buffers */
static BYTE DATA_PACKET[TKN_OFFS_DATA_EOF + 1]    = { 0x00, TKN_TYPE_DATA, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF };
static BYTE ACK_PACKET[TKN_OFFS_ACK_EOF + 1]      = { 0x00, TKN_TYPE_ACK, 0x00, 0x00, 0x00, 0xFF };
static BYTE TOKEN_PACKET[TKN_OFFS_TOKEN_EOF + 1]  = { 0x00, TKN_TYPE_TOKEN, 0x00, 0x00, 0xFF };

int main(int argc, char **argv)
{
    int i,size;
    FILE *fd = NULL;
    char packetPath[50] = "packets/";
    char *str_ptr = packetPath + strlen(packetPath);

    strcpy(str_ptr, "DATA.hex");
    fd = fopen(packetPath, "w");
    size = sizeof(DATA_PACKET);
    for (i=0; i<size; i++) fputc(DATA_PACKET[i], fd);
    
    strcpy(str_ptr, "ACK.hex");
    fd = fopen(packetPath, "w");
    size = sizeof(ACK_PACKET);
    for (i=0; i<size; i++) fputc(ACK_PACKET[i], fd);
    
    strcpy(str_ptr, "TOKEN.hex");
    fd = fopen(packetPath, "w");
    size = sizeof(TOKEN_PACKET);
    for (i=0; i<size; i++) fputc(TOKEN_PACKET[i], fd);
    
    return 0;
}
