#include <avr/io.h>
#include <avr/interrupt.h>
#include "tkn_wrappers.h"

#define NODE_ID 1
char packet_buf[16];
volatile unsigned char counter = 0;

ISR(PCINT2_vect)//, ISR_NAKED)
{
	//asm("push r24");
	//asm("push r25");
	counter++;
	//asm("pop r25");
	//asm("pop r24");
	//reti();
}

int main(void)
{
	sei(); // enable interrupts
	
	// Send greeting
	strcpy(packet_buf, "SPEEDOMETER: on");
	TKN_Send(packet_buf, NODE_ID);
	
	// Configure PORT C as input 
	// and enable PCINT16
	PORTC = 0xFF;
	DDRC = 0x00;
	PCMSK2 |= (1 << PCINT16);
	PCICR |= (1 << PCIE2);
	
	// Flush RX buffer
	// while (TKN_Receive(packet_buf));
	
	// Print `counter` continuously
	unsigned char counter_last = 0;
	while ( TKN_Receive(packet_buf) == 0 ) {
		if (counter_last != counter) {
			// sprintf(packet_buf, "Counter: %d", counter);
			// TKN_Send(packet_buf, NODE_ID);
			counter_last = counter;
		}
	}

	// Send appropriate msg and exit
	strcpy(packet_buf, "SPEEDOMETER: off");
	TKN_Send(packet_buf, NODE_ID);
	
	// Return to TKN_BOOT
	((void(*)()) TKN_RETURN_ADDR)(); 
}
