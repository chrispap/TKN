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

Reset:
    jmp main

.dseg
.org SRAM_START
packetBuff: .byte TKN_PACKET_SIZE

.cseg
str:		.db "General Node.asm"
str_bad:    .db "BAD INTERRUPT!!!"

main:
    ldi YL, LOW(packetBuff)
    ldi YH, HIGH(packetBuff)
	
loop:	
	call TKN_popPacket
    and temp0, temp0
    breq loop
    ldd temp1, Y+0
    and temp1, temp1
    breq exit
    rjmp loop
	 
exit:
    ret

/* Bad Interrupt ISRs */
BAD_ISR:
    ldi ZL, LOW(str_bad << 1)
    ldi ZH, HIGH(str_bad << 1)
    call fillPacketBuf
	ldi temp0, 1
	call TKN_PushPacket
	reti

