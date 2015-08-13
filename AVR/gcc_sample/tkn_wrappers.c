#include "tkn_wrappers.h"

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

	asm ("mov r28, r24");
	asm ("mov r29, r25");
	((void(*)()) TKN_POP_ADDR)();
	asm ("mov  r24, r16");
	
	asm ("pop r29");
	asm ("pop r28");
	asm ("pop r18");
	asm ("pop r17");
	asm ("pop r16");
}
