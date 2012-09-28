/* Entry point */

.include "m644PAdef.inc"

/*==============================================================
=== Register definitions ===
=== Constant definitions ===
================================================================*/
.equ LEDS_OUT   = PORTB
.equ LEDS_IN    = PINB
.equ SWITCHES_IN= PINA

; Register usage - Inside ISRs
.def rsreg  = r16
.def itemp0 = r17
.def itemp1 = r18
.def itemp2 = r19

; Register usage - Inside main thread
.def temp0  = r20
.def temp1  = r21
.def temp2  = r22

.def mVar0 = r0 ;unused
.def mVar1 = r1 ;unused
.def mVar2 = r2 ;unused
.def mVar3 = r3 ;unused
.def mVar4 = r23 ;unused
.def pageCount = r24
.def wordCount = r25


/*==============================================================
=== inclusion of Source Files === 
=== Vectors.asm should be included first of ALL !!!
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
	; Init Stack Pointer
	ldi  r16,low(RAMEND)
	out  SPL,r16
	ldi  r16,high(RAMEND)
	out  SPH,r16

	; Configure PORTB as output  (LEDs are connected to this port)
	; All bits have logic zero output
	ldi  r16,0b11111111
	out  PORTB,r16
	ldi  r16,0b11111111	// All bits are set to OUTPUT
	out  DDRB,r16

	; Configure PORTA as input (Switches are connected to this port)
	ldi  r16,0b11111111
	out  PORTA,r16
	ldi  r16,0b00000000
	out  DDRA,r16

	; Procceed to the application
	jmp main
 
/*==============================================================
=== Main ===
================================================================*/
.dseg
pageBuff:	.byte (PAGESIZE*2)
dataPacket: .byte TKN_PACKET_SIZE

.cseg
msg_str:	.db "-AVR-TKNboot----"
err_str:	.db "-Error----------"
int_str:	.db "-PC Int 7-------"
ready_str:	.db "-MCU-READY------"
user_str:	.db "-Goto User------"

main:
    call ledInit
    call TKN_init
    call Enable_PcInt7
	//
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

	//
    ; Init local variables.
    clr mvar0
	clr mvar1
	clr mvar2
	clr mvar3
	clr wordCount
    ldi YL,LOW(dataPacket)
    ldi YH,HIGH(dataPacket)

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
	cpi temp1, 'B'
	breq do_boot
	cpi temp1, 'U'
	breq do_user

	; unknown_command:
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
	inc mvar0
	mov temp0, mvar0
	call setLeds1
	rjmp main_loop
	; MAIN LOOP END

send_signature:
	ldi ZL, LOW(0x0000)
	ldi ZH, HIGH(0x0000)
	ldi temp0, (1<<SPMEN) | (1<<SIGRD)
	out SPMCSR, temp0
	lpm mvar1, Z
	inc ZL
	inc ZL
	ldi temp0, (1<<SPMEN) | (1<<SIGRD)
	out SPMCSR, temp0
	lpm mvar2, Z
	inc ZL
	inc ZL
	ldi temp0, (1<<SPMEN) | (1<<SIGRD)
	out SPMCSR, temp0
	lpm mvar3, Z
	std Y+2, mvar1
	std Y+3, mvar2
	std Y+4, mvar3
	rjmp push_loop

do_user:
	;cli
	jmp 0x0000 // <-- Goto User Section !!!

    ;ldi ZL, LOW(user_str<<1)
    ;ldi ZH, HIGH(user_str<<1)
	;call fillPacketBuf
	;rjmp push_loop

do_boot:
	clr wordCount
	ldd pageCount, Y+3
	ldi ZL, LOW(ready_str<<1)
	ldi ZH, HIGH(ready_str<<1)
	call fillPacketBuf
	call TKN_PushPacket
	ldi ZL, LOW(pageBuff)
	ldi ZH, HIGH(pageBuff)
hexPacket_loop:
	call TKN_popPacket
	and temp0, temp0
	brne hexPacket_received
	rjmp hexPacket_loop
hexPacket_received:
	ldi temp1, TKN_PACKET_SIZE
hexPacket_store:
	ld temp2, Y+
	st Z+, temp2
	dec temp1
	brne hexPacket_store
	ldi YL,LOW(dataPacket)
    ldi YH,HIGH(dataPacket)
	ldi temp1, TKN_PACKET_SIZE/2
	add wordCount, temp1
	cpi wordCount, PAGESIZE
	brne hexPacket_loop
hexPage_received:
	ldi YL, LOW(pageBuff)
	ldi YH, HIGH(pageBuff)
	clr ZL
	mov ZH, pageCount
	call BL_Write_page
	ldi YL, LOW(dataPacket)
    ldi YH, HIGH(dataPacket)
	rjmp main_loop_continue


/*==============================================================
=== fillPacketBuf ===
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
	push ZL
	push ZH

	;Copy the data to the Transmission buffer
	ldi YH,HIGH(dataPacket)
	ldi YL,LOW(dataPacket)
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


;;; PROXEIRO ;;;

;detect_newLine:
	; Detect '\n' or '\0' in the last character of the packet
	;ldd temp1, Y + TKN_PACKET_SIZE - 1
	;and temp1, temp1 ;detected zero byte
	;breq hexLine_complete
	;cpi temp1, '\n'
	;breq hexLine_complete ;detected new line
    
;inc_Buffer:
    ; Increase the pointer to the hexline buffer
	;ldi temp1, TKN_PACKET_SIZE
	;add YL, temp1
	;brcc notOverflow
	;inc YH
	;notOverflow:
	;rjmp main_loop

;hexLine_complete:
	; Send the ready singaling string
	;ldi YL,LOW(dataPacket)
	;ldi YH,HIGH(dataPacket)
