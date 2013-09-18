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
.equ fillPacketBuf      = 0x7432
.equ Bin2ToHex4         = 0x7344
.equ Bin1ToHex2         = 0x734b
.equ Bin1ToHex1         = 0x734f
.equ setLeds            = 0x72bc

.def temp0              = r16
.def temp1              = r17
.def temp2              = r18
.def last_val           = r21

Reset:
    jmp main

.dseg
.org SRAM_START
inPacketBuff: .byte TKN_PACKET_SIZE
outPacketBuff: .byte TKN_PACKET_SIZE

.cseg
str:		.db "________________"
str_bad:    .db "BAD INTERRUPT!!!"

main:
    ldi YL, LOW(outPacketBuff)
    ldi YH, HIGH(outPacketBuff)
	ldi temp1, TKN_PACKET_SIZE
	
	ldi temp0, 'A'
	st Y+, temp0
	dec temp1
	
	ldi temp0, ':'
	st Y+, temp0
	dec temp1
	
	ldi temp0, '0'
	st Y+, temp0
	dec temp1
	
	ldi temp0, 'x'
	st Y+, temp0
	dec temp1
	
	clr temp0
init_buf:
	st Y+, temp0
	dec temp1
	and temp1, temp1
	brne init_buf
	
set_adc:
	ldi temp1,  (1 << REFS0) | (1<<ADLAR) | (2<<MUX0)
	lds temp0, ADMUX
	or temp0, temp1
	sts ADMUX, temp0 
	
	ldi temp1, (1 << ADATE) | (7<<ADPS0)
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
    ldi YL, LOW(inPacketBuff)
    ldi YH, HIGH(inPacketBuff)
loop1:
	call TKN_popPacket
    and temp0, temp0
	brne exit

send_val:
	lds temp2, ADCH
	cp temp2, last_val
	breq loop1
	mov last_val, temp2
    ldi YL, LOW(outPacketBuff)
    ldi YH, HIGH(outPacketBuff)
	mov ZL, YL 
	mov ZH, YH
	inc ZL
	inc ZL
	inc ZL
	inc ZL
	clr temp1
	call Bin2ToHex4
	ldi temp0, 1
sendLoop:
	call TKN_pushPacket
	and temp0, temp0
	brne sendLoop

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

