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

	;Copy the data to the Transmission buffer
	ldi YH,HIGH(dataPacket) ; void* data_ptr
	ldi YL,LOW(dataPacket)  ;
	ldi temp0, 1			; HardCode the receiver
	call TKN_pushPacket
	
	;Restore SREG and main thread's temp registers
	mov temp2, itemp2
	mov temp1, itemp1
	mov temp0, itemp0
	out SREG, rsreg

	reti
