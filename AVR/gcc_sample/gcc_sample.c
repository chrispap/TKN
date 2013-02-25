#include <avr/io.h>
#include <avr/interrupt.h>
#include "gcc_sample.h"

char TKN_Send(char *buf, char dest_id)
{
	asm ("push r16");
	asm ("push r17");
	asm ("push r18");
	asm ("push r28");
	asm ("push r29");
	
	asm ("mov r28, r24");
	asm ("mov r29, r25");
	asm ("mov r16, r22");
	((void(*)()) TKN_PUSH_ADDR)();
	asm ("mov  r24, r16");
	
	asm ("pop r29");
	asm ("pop r28");
	asm ("pop r18");
	asm ("pop r17");
	asm ("pop r16");
}

char TKN_Receive(char *buf)
{
	asm ("push r16");
	asm ("push r17");
	asm ("push r18");
	asm ("push r28");
	asm ("push r29");

	((void(*)()) TKN_POP_ADDR)();
	asm ("mov  r24, r16");
	
	asm ("pop r29");
	asm ("pop r28");
	asm ("pop r18");
	asm ("pop r17");
	asm ("pop r16");
}

char PacketBuffer[16];

ISR(BADISR_vect)
{
	TKN_Send("BAD_INTERRUPT!!!", 1);
}

int main(void)
{
	/* Enable interrupts */
	sei();
	
	strncpy(PacketBuffer, "From Gcc app!", 16);
	
	/* main loop */
	while (TKN_Receive( PacketBuffer) == 0)
	{
		while (TKN_Send(PacketBuffer, 1) != 0);	
	}
	
	/*Return to TKN_BOOT */
	((void(*)()) TKN_RETURN_ADDR)();
}
