#include <avr/io.h>
#include <avr/interrupt.h>

/* TKN Functions */
void (*TKN_Reset)(void)		= (void(*)()) 0x7000;
void (*TKN_Return)(void)	= (void(*)()) 0x7394;
void (*__TKN_popPacket)(void)	= (void(*)()) 0x7098;
void (*__TKN_pushPacket)(void)	= (void(*)()) 0x7062;

char PacketBuffer[17];

char TKN_PushPacket(char *buf, char dest_id)
{
	asm ("push r16");
	asm ("push r17");
	asm ("push r18");
	asm ("push r28");
	asm ("push r29");
	
	asm ("mov r28, r24");
	asm ("mov r29, r25");
	asm ("mov r16, r22");
	__TKN_pushPacket();
	asm ("mov  r24, r16");
	
	asm ("pop r29");
	asm ("pop r28");
	asm ("pop r18");
	asm ("pop r17");
	asm ("pop r16");
}

char TKN_PopPacket(char *buf)
{
	asm ("push r16");
	asm ("push r17");
	asm ("push r18");
	asm ("push r28");
	asm ("push r29");

	__TKN_popPacket();
	asm ("mov  r24, r16");
	
	asm ("pop r29");
	asm ("pop r28");
	asm ("pop r18");
	asm ("pop r17");
	asm ("pop r16");
}

int main(void)
{
	asm("sei");
	
	while(!TKN_PopPacket(PacketBuffer));
	strcpy(PacketBuffer, "Hi World!");
	TKN_PushPacket(PacketBuffer, 1);
	
	TKN_Return();
}
