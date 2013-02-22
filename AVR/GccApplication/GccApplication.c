#include <avr/io.h>
#include <avr/interrupt.h>

/* TKN Functions */
#define TKN_RESET_ADDR	0x7000
#define TKN_RETURN_ADDR 0x7394
#define TKN_POP_ADDR	0x7098
#define TKN_PUSH_ADDR	0x7062

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
	while (TKN_Send("Bad Interrupt!", 1) != 0);
	((void(*)()) TKN_RETURN_ADDR)();
}

int main(void)
{	
	asm("sei");
	
	strncpy(PacketBuffer, "From Gcc app!", 16);
	
	while (TKN_Receive( PacketBuffer) == 0);
		//while (TKN_Send(PacketBuffer, 1) != 0);
	
	((void(*)()) TKN_RETURN_ADDR)();
}
