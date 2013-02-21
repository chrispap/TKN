#include <avr/io.h>
#include <avr/interrupt.h>

/* TKN Functions */
int (*TKN_Reset)(void) = 0x7000;
int (*TKN_Return)(void) = 0x7397;
int (*TKN_pushPacket)(void) = 0x7062;
int (*TKN_popPacket)(void) = 0x7098;

int main(void)
{
	asm("sei");
	
	while (0){
		
	}
	
	TKN_Return();
}
