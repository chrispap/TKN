/* Entry point */

.include "m644PAdef.inc"

/*==============================================================
=== - Register definitions
=== - Constant definitions
================================================================*/
.equ LEDS_OUT   = PORTB
.equ LEDS_IN    = PINB
.equ SWITCHES_IN= PINA

; Register used for store-restore inside ISRs
.def itemp0 = r12
.def itemp1 = r13
.def itemp2 = r14
.def rsreg  = r15

; Working registers
.def temp0  = r16
.def temp1  = r17
.def temp2  = r18

; r19 - r25 are available for the main program.
.def pageCount = r19
.def wordCount = r20
.def var0	   = r21
.def var1	   = r22
.def var2	   = r23
.def var3	   = r24
.def var4	   = r25

/*==============================================================
=== - Inclusion of Source Files
=== - Vectors.asm should be included *first*
================================================================*/
.include "Vectors.asm"
.include "TKN.asm"
.include "Usart.asm"
.include "Utils.asm"
.include "Flash.asm"

/*==============================================================
=== - Reset 
=== - Initializations
================================================================*/
Reset:
	; Init Stack Pointer
	ldi  r16,low(RAMEND)
	out  SPL,r16
	ldi  r16,high(RAMEND)
	out  SPH,r16

	; Configure PORTB as output  (LEDs are connected to this port)
	; All bits have logic zero output
	ldi  r16,0b11111111
	out  PORTB,r16
	ldi  r16,0b11111111	; All bits are set to OUTPUT
	out  DDRB,r16

	; Configure PORTA as input (Switches are connected to this port)
	ldi  r16,0b11111111
	out  PORTA,r16
	ldi  r16,0b00000000
	out  DDRA,r16

	; Procceed to the application
	jmp main
 
/*==============================================================
=== - Main ===
================================================================*/
.dseg
pageBuff:	.byte (PAGESIZE*2)
packetBuff:	.byte TKN_PACKET_SIZE

.cseg
ver_str:	.db "AVR TKN BOOT v0 "
err_str:	.db "Error           "
int_str:	.db "Interrupt fired "
run_str:	.db "Run user        "
ret_str:	.db "Return from user"
rdy_str:	.db "-MCU-READY------"

main:
    call ledInit
    call TKN_init
    call Enable_PcInt7

	ldi YL, LOW(pageBuff)
	ldi YH, HIGH(pageBuff)
	ldi temp0, PAGESIZE
	ldi temp1, $AA
	ldi temp2, $BB
fillPageLoop:
	st Y+, temp1
	st Y+, temp2
	dec temp0
	brne fillPageLoop

    ; Init local variables.
    clr var0
	clr var1
	clr var2
	clr var3
	clr wordCount
    ldi YL,LOW(packetBuff)
    ldi YH,HIGH(packetBuff)

    ; Move Interrupts in boot section...
	; ...and enable them
    call Move_interrupts_BootSec
    sei
    
	; MAIN LOOP START
main_loop:
	call TKN_popPacket
	and temp0, temp0
	brne packet_received
	rjmp main_loop
packet_received:

switch_command:
	; Switch command depending on the first char,
	; only if the second char is ':'
	ldd temp1, Y+1
	cpi temp1, ':'
	brne push_loop
	ld temp1, Y

	cpi temp1, 'S'
	breq send_signature

	cpi temp1, 'R'
	breq run_user

	cpi temp1, 'V'
	breq send_version

	cpi temp1, 'B'
	breq do_boot

	; Default
	; Uknown_command:
    ldi temp1, '?'
    std Y+0, temp1
	clr temp1
	std Y+1, temp1
	rjmp push_loop
	
	push_loop:
	call TKN_pushPacket
	and temp0, temp0
	brne push_loop
main_loop_continue:
	rjmp main_loop
	; MAIN LOOP END

send_signature:
	call read_Signature
	rjmp push_loop

send_version:
	ldi ZL, LOW(ver_str<<1)
	ldi ZH, HIGH(ver_str<<1)
	call fillPacketBuf
	rjmp push_loop

run_user:
	push temp0
	push YL
	push YH

	ldi ZL, LOW(run_str<<1)
	ldi ZH, HIGH(run_str<<1)
	call fillPacketBuf
	call TKN_PushPacket
	call 0x0000
	
	pop YH
	pop YL
	pop temp0
	ldi ZL, LOW(ret_str<<1)
	ldi ZH, HIGH(ret_str<<1)
	call fillPacketBuf
	rjmp push_loop

do_boot:
	clr wordCount
	ldd pageCount, Y+3
	ldi ZL, LOW(rdy_str<<1)
	ldi ZH, HIGH(rdy_str<<1)
	call fillPacketBuf
	call TKN_PushPacket
	ldi XL, LOW(pageBuff)
	ldi XH, HIGH(pageBuff)
hexPacket_loop:
	call TKN_popPacket
	and temp0, temp0
	brne hexPacket_received
	rjmp hexPacket_loop
hexPacket_received:
	ldi temp1, TKN_PACKET_SIZE
hexPacket_store:
	ld temp2, Y+
	st X+, temp2
	dec temp1
	brne hexPacket_store
	ldi YL,LOW(packetBuff)
    ldi YH,HIGH(packetBuff)
	ldi temp1, TKN_PACKET_SIZE/2
	add wordCount, temp1
	cpi wordCount, PAGESIZE
	brne hexPacket_loop
hexPage_received:
	ldi YL, LOW(pageBuff)
	ldi YH, HIGH(pageBuff)
	clr ZL
	mov ZH, pageCount
	
	cli
	call BL_Write_page
	sei

	ldi YL, LOW(packetBuff)
    ldi YH, HIGH(packetBuff)
	rjmp main_loop_continue


/*==============================================================
=== - Read signature
================================================================*/
read_signature:
	clr ZL
	clr ZH
	ldi temp1, (1<<SPMEN) | (1<<SIGRD)
	out SPMCSR, temp1
	lpm var1, Z
	inc ZL
	inc ZL
	ldi temp1, (1<<SPMEN) | (1<<SIGRD)
	out SPMCSR, temp1
	lpm var2, Z
	inc ZL
	inc ZL
	ldi temp1, (1<<SPMEN) | (1<<SIGRD)
	out SPMCSR, temp1
	lpm var3, Z
	mov ZL, YL
	mov ZH, YH
	clr temp1
	mov temp2, var1
	call Bin2ToHex4
	ldi temp1, '0'
	std Y+0, temp1
	ldi temp1, 'x'
	std Y+1, temp1
	clr temp1
	mov temp2, var2
	call Bin2ToHex4
	ldi temp1, '0'
	std Y+4, temp1
	ldi temp1, 'x'
	std Y+5, temp1
	clr temp1
	mov temp2, var3
	call Bin2ToHex4
	ldi temp1, '0'
	std Y+8, temp1
	ldi temp1, 'x'
	std Y+9, temp1

	ret

/*==============================================================
=== - FillPacketBuf ===
================================================================*/
fillPacketBuf:
	push temp0
	push temp1
	push YL
	push YH
	push ZL
	push ZH

    ldi temp1, TKN_PACKET_SIZE
fillPacketBufLoop:
    lpm temp0, Z+
    st Y+, temp0
    dec temp1;
    brne fillPacketBufLoop

	pop ZH
	pop ZL
	pop YH
	pop YL
	pop temp1
	pop temp0
	ret

/*==============================================================
=== - Utility
=== - ISRs
=== - etc
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
=== - Enable the PCINT7 (switch)
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
=== - Handle SWITCH 7 Pin Change Interrupt
===		var+
===		Transmit(var)
================================================================*/

PCI0_ISR:
	;Save SREG and main thread's temp registers
	in rsreg, SREG
	mov itemp0, temp0
	mov itemp1, temp1
	mov itemp2, temp2
	push YL
	push YH
	push ZL
	push ZH

	;Copy the data to the Transmission buffer
	ldi YH,HIGH(packetBuff)
	ldi YL,LOW(packetBuff)
	ldi ZL, LOW(int_str<<1)
    ldi ZH, HIGH(int_str<<1)
	call fillPacketBuf

	ldi temp0, 1 ; HardCode the receiver
	call TKN_pushPacket
	
	;Restore SREG and main thread's temp registers
	pop ZH
	pop ZL
	pop YH
	pop YL
	mov temp2, itemp2
	mov temp1, itemp1
	mov temp0, itemp0
	out SREG, rsreg

	reti
