.include "m644PAdef.inc"

.cseg
.org 0x0000

jmp Reset
jmp BAD_ISR
jmp BAD_ISR
jmp BAD_ISR
jmp BAD_ISR
jmp BAD_ISR
jmp ISR_PCINT2
jmp BAD_ISR
jmp BAD_ISR
jmp BAD_ISR
jmp BAD_ISR
jmp BAD_ISR
jmp BAD_ISR
jmp BAD_ISR
jmp BAD_ISR
jmp BAD_ISR
jmp BAD_ISR
jmp BAD_ISR
jmp BAD_ISR
jmp BAD_ISR
jmp BAD_ISR
jmp BAD_ISR
jmp BAD_ISR
jmp BAD_ISR
jmp BAD_ISR
jmp BAD_ISR
jmp BAD_ISR
jmp BAD_ISR
jmp BAD_ISR
jmp BAD_ISR
jmp BAD_ISR

.equ TKN_PACKET_SIZE    = 0x10

.equ TKN_pushPacket     = 0x7062
.equ TKN_popPacket      = 0x7098
.equ fillPacketBuf      = 0x7432
.equ Bin2ToHex4         = 0x7344
.equ Bin1ToHex2         = 0x734b
.equ Bin1ToHex1         = 0x734f
.equ setLeds            = 0x72bc

.def temp0              = r16
.def temp1              = r17
.def temp2              = r18
.def counter			= r0
.def counter_last		= r1

Reset:
    jmp main

.dseg
.org SRAM_START
packetBuff: .byte TKN_PACKET_SIZE

.cseg
main:
    ; Configure PORTC as input
    ser temp0
    out PORTC, temp0
	eor temp0, temp0
    out DDRC, temp0
	lds temp0, PCICR
	sbr temp0, (1<<PCIE2)
	sts PCICR, temp0
	lds temp0, PCMSK2
	sbr temp0, (1<<PCINT16)
	sts PCMSK2, temp0

	; Load packet address
    ldi YL, LOW(packetBuff)
    ldi YH, HIGH(packetBuff)

	clr counter
	clr counter_last

	; Flush Rx Queue
flush_rx:
	call TKN_popPacket
	and  temp0, temp0
	breq flush_rx

	; Enter main loop
loop:
    call TKN_popPacket
    and  temp0, temp0
    breq exit 
	cp  counter, counter_last
	breq loop
	ldi temp0, 1
    call TKN_pushPacket
	mov counter_last, counter
    rjmp loop

exit:
    ret

ISR_PCINT2:
	;inc counter
	;inc counter_last
	reti

; Bad Interrupt ISRs
BAD_ISR:
    reti
