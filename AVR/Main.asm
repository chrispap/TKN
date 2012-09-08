/* Entry point */

.include "m644PAdef.inc"

/*==============================================================
=== Register definitions ===
=== Constant definitions ===
================================================================*/
.equ LEDS_OUT	 = PORTB
.equ LEDS_IN	 = PINB
.equ SWITCHES_IN = PINA

.def rsreg = r16	;Register usage - Inside ISRs
.def itemp0= r17	;
.def itemp1= r18	;
.def itemp2= r19	;
.def temp0 = r20	;Register usage - Inside main thread
.def temp1 = r21	;
.def temp2 = r22	;

.def lineCount = r3

/*==============================================================
=== inclusion of Source Files === 
=== !!! >> Vectors.asm should be included first of ALL << !!!
================================================================*/
.include "Vectors.asm"
.include "TKN.asm"
.include "Usart.asm"
.include "Utils.asm"
.include "Flash.asm"


/*==============================================================
=== Reset - Initializations ===
================================================================*/
Reset:
	;Init Stack Pointer
	ldi  r16,low(RAMEND)
	out  SPL,r16
	ldi  r16,high(RAMEND)
	out  SPH,r16

	;Configure PORTB as output  (LEDs are connected to this port)
	;All bits have logic zero output
	ldi  r16,0b11111111
	out  PORTB,r16
	ldi  r16,0b11111111	// All bits are set to OUTPUT
	out  DDRB,r16

	;Configure PORTA as input (Switches are connected to this port)
	ldi  r16,0b11111111
	out  PORTA,r16
	ldi  r16,0b00000000
	out  DDRA,r16

	/*Procceed to the application*/
	jmp main
 
/*==============================================================
=== Main ===
================================================================*/
.dseg
dataPacket:
	.byte TKN_PACKET_SIZE
.org $300
hexLine:
	.byte $FF

.cseg
tempBuff:
.db "-MCU-READY------"

main:
	call ledInit
	call TKN_init
	call Enable_PcInt7

	;Fill the data packet.
	ldi ZL, LOW(tempBuff<<1)
	ldi ZH, HIGH(tempBuff<<1)

	ldi YL,LOW(dataPacket)
	ldi YH,HIGH(dataPacket)
	
	ldi temp1, TKN_PACKET_SIZE
fillPacketLoop:
		lpm temp0, Z+
		st Y+, temp0
		dec temp1;
	brne fillPacketLoop

	;Move Interrupts in boot section
	call Move_interrupts_BootSec

	clr lineCount
	ldi YL,LOW(hexLine)
	ldi YH,HIGH(hexLine)
	
	;Enable Interrupts and enter the main loop.
	sei

recv_loop:
	/* Poll for incoming packets */
	call TKN_popPacket
	and temp0, temp0
	breq recv_loop

	rjmp recv_loop // DEBUG (Short-circuit loop)

	/* Here, a new packet has been received in the hexline buffer */
	/* Send it back (Debug reasons) */
	/*
	push temp0
	push_loop1:
	call TKN_pushPacket
	and temp0, temp0
	brne push_loop1
	pop temp0
	*/

	/* Detect '\n' or '\0' in the last character of the packet */
	ldd temp1, Y + TKN_PACKET_SIZE - 1
	and temp1, temp1 //detect zero byte
	breq hexLine_complete
	cpi temp1, '\n'
	breq hexLine_complete //detect new line
	ldi temp1, TKN_PACKET_SIZE
	add YL, temp1
	brcc notOverflow
	inc YH
	notOverflow:
	rjmp recv_loop

hexLine_complete:
	/**
	* HERE I should analyze the hex line ...
	* Just delay for now... 
	ldi temp1, 1
	procDelay:
	call delay
	dec temp1
	brne procDelay
	*/

	/* Send the ready singaling string */
	ldi YL,LOW(dataPacket)
	ldi YH,HIGH(dataPacket)
	push_loop2:
	call TKN_pushPacket
	and temp0, temp0
	brne push_loop2

	inc lineCount
	mov temp0, lineCount
	call setLeds1

	ldi YL,LOW(hexLine)
	ldi YH,HIGH(hexLine)
	rjmp recv_loop


/*==============================================================
=== Interrupts ===
=== (ISRs / Enable routines / move to BOOT_SEC / ...
================================================================*/
Move_interrupts_BootSec:
	; Get MCUCR
	in temp0, MCUCR
	mov temp1, temp0
	; Enable change of Interrupt Vectors
	ori temp0, (1<<IVCE)
	out MCUCR, temp0
	; Move interrupts to Boot Flash section
	ori temp1, (1<<IVSEL)
	out MCUCR, temp1

	ret

/*==============================================================
=== Enable the PCINT7 (switch)
================================================================*/
Enable_PcInt7:
	cbi EIMSK, INT0

	lds temp0, EICRA
	sbr temp0, (2<<ISC00)
	sts EICRA, temp0

	sbi EIMSK, INT0
	
	lds temp0, PCICR
	sbr temp0, (1<<PCIE0)
	sts PCICR, temp0

	lds temp0, PCMSK0
	sbr temp0, (1<<PCINT7)
	sts PCMSK0, temp0

	ret

/*==============================================================
=== Handle SWITCH 7 Pin Change Interrupt
===   var+
===   Transmit(var)
================================================================*/

PCI0_ISR:
	;Save SREG and main thread's temp registers
	in rsreg, SREG
	mov itemp0, temp0
	mov itemp1, temp1
	mov itemp2, temp2
	push YL
	push YH

	;Copy the data to the Transmission buffer
	ldi YH,HIGH(dataPacket) ; void* data_ptr
	ldi YL,LOW(dataPacket)  ;
	ldi temp0, 1			; HardCode the receiver
	call TKN_pushPacket
	
	;Restore SREG and main thread's temp registers
	pop YH
	pop YL
	mov temp2, itemp2
	mov temp1, itemp1
	mov temp0, itemp0
	out SREG, rsreg

	reti
