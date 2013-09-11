.include "m644PAdef.inc"

.cseg
.org 0x0000

jmp Reset
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
jmp BAD_ISR
jmp BAD_ISR
jmp BAD_ISR
jmp BAD_ISR
jmp BAD_ISR
jmp BAD_ISR

.equ TKN_PACKET_SIZE    = 0x10
.equ TKN_pushPacket     = 0x7062
.equ TKN_popPacket      = 0x7098
.equ fillPacketBuf      = 0x7434
.equ bin1ToHex2         = 0x734d
.equ setLeds            = 0x72c0
.def temp0              = r16
.def temp1              = r17
.def temp2              = r18

Reset:
    jmp main

.dseg
.org SRAM_START
packetBuff: .byte TKN_PACKET_SIZE

.cseg
str:		.db "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
str_bad:    .db "BAD INTERRUPT!!!"

main:
    ldi YL, LOW(packetBuff)
    ldi YH, HIGH(packetBuff)
	ldi ZL, LOW(str<<1)
	ldi ZH, HIGH(str<<1)
	call fillPacketBuf
	
set_adc:
	ldi temp1,  (1 << REFS0) | (1<<ADLAR) | (2<<MUX0)
	lds temp0, ADMUX
	or temp0, temp1
	sts ADMUX, temp0 
	
	ldi temp1, (1 << ADATE) | (7<<ADPS0);
	lds temp0, ADCSRA
	or temp0, temp1
	sts ADCSRA, temp0

	ldi temp0, 0 // Free running mode
	sts ADCSRB, temp0
	
	ldi temp1, (1 << ADEN) | (1 << ADSC)
	lds temp0, ADCSRA
	or temp0, temp1
	sts ADCSRA, temp0 

loop:	
	call TKN_popPacket
    and temp0, temp0
	brne exit

send_val:
	lds temp1, ADCH
	st Y, temp1
	ldi temp0, 1
	call TKN_pushPacket

    rjmp loop
	 
exit:
turn_off_adc:
	clr temp0
	sts ADCSRA, temp0 

    ret

/* Bad Interrupt ISRs */
BAD_ISR:
    ldi ZL, LOW(str_bad << 1)
    ldi ZH, HIGH(str_bad << 1)
    call fillPacketBuf
	ldi temp0, 1
	call TKN_PushPacket
	reti

