.include "m644PAdef.inc"

.cseg
.org 0x0000
jmp Reset
jmp ISR_01
jmp ISR_02
jmp ISR_03
jmp ISR_04
jmp ISR_05
jmp ISR_06
jmp ISR_07
jmp ISR_08
jmp ISR_09
jmp ISR_10
jmp ISR_11
jmp ISR_12
jmp ISR_13
jmp ISR_14
jmp ISR_15
jmp ISR_16
jmp ISR_17
jmp ISR_18
jmp ISR_19
jmp ISR_20
jmp ISR_21
jmp ISR_22
jmp ISR_23
jmp ISR_24
jmp ISR_25
jmp ISR_26
jmp ISR_27
jmp ISR_28
jmp ISR_29
jmp ISR_30

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
str:    .db "General Node.asm"
strbad01:    .db "BAD INTERRUPT:01"
strbad02:    .db "BAD INTERRUPT:02"
strbad03:    .db "BAD INTERRUPT:03"
strbad04:    .db "BAD INTERRUPT:04"
strbad05:    .db "BAD INTERRUPT:05"
strbad06:    .db "BAD INTERRUPT:06"
strbad07:    .db "BAD INTERRUPT:07"
strbad08:    .db "BAD INTERRUPT:08"
strbad09:    .db "BAD INTERRUPT:09"
strbad10:    .db "BAD INTERRUPT:10"
strbad11:    .db "BAD INTERRUPT:11"
strbad12:    .db "BAD INTERRUPT:12"
strbad13:    .db "BAD INTERRUPT:13"
strbad14:    .db "BAD INTERRUPT:14"
strbad15:    .db "BAD INTERRUPT:15"
strbad16:    .db "BAD INTERRUPT:16"
strbad17:    .db "BAD INTERRUPT:17"
strbad18:    .db "BAD INTERRUPT:18"
strbad19:    .db "BAD INTERRUPT:19"
strbad20:    .db "BAD INTERRUPT:20"
strbad21:    .db "BAD INTERRUPT:21"
strbad22:    .db "BAD INTERRUPT:22"
strbad23:    .db "BAD INTERRUPT:23"
strbad24:    .db "BAD INTERRUPT:24"
strbad25:    .db "BAD INTERRUPT:25"
strbad26:    .db "BAD INTERRUPT:26"
strbad27:    .db "BAD INTERRUPT:27"
strbad28:    .db "BAD INTERRUPT:28"
strbad29:    .db "BAD INTERRUPT:29"
strbad30:    .db "BAD INTERRUPT:30"

main:
    ldi YL, LOW(packetBuff)
    ldi YH, HIGH(packetBuff)
	;rjmp exit

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



/** bad interrupts ISRs **/

ISR_01:
    ldi ZL, LOW(strbad01 << 1)
    ldi ZH, HIGH(strbad01 << 1)
    call fillPacketBuf
	ldi temp0, 1
	call TKN_PushPacket
	reti

ISR_02:
    ldi ZL, LOW(strbad02 << 1)
    ldi ZH, HIGH(strbad02 << 1)
    call fillPacketBuf
	ldi temp0, 1
	call TKN_PushPacket
	reti

ISR_03:
    ldi ZL, LOW(strbad03 << 1)
    ldi ZH, HIGH(strbad03 << 1)
    call fillPacketBuf
	ldi temp0, 1
	call TKN_PushPacket
	reti

ISR_04:
    ldi ZL, LOW(strbad04 << 1)
    ldi ZH, HIGH(strbad04 << 1)
    call fillPacketBuf
	ldi temp0, 1
	call TKN_PushPacket
	reti

ISR_05:
    ldi ZL, LOW(strbad05 << 1)
    ldi ZH, HIGH(strbad05 << 1)
    call fillPacketBuf
	ldi temp0, 1
	call TKN_PushPacket
	reti

ISR_06:
    ldi ZL, LOW(strbad06 << 1)
    ldi ZH, HIGH(strbad06 << 1)
    call fillPacketBuf
	ldi temp0, 1
	call TKN_PushPacket
	reti

ISR_07:
    ldi ZL, LOW(strbad07 << 1)
    ldi ZH, HIGH(strbad07 << 1)
    call fillPacketBuf
	ldi temp0, 1
	call TKN_PushPacket
	reti

ISR_08:
    ldi ZL, LOW(strbad08 << 1)
    ldi ZH, HIGH(strbad08 << 1)
    call fillPacketBuf
	ldi temp0, 1
	call TKN_PushPacket
	reti

ISR_09:
    ldi ZL, LOW(strbad09 << 1)
    ldi ZH, HIGH(strbad09 << 1)
    call fillPacketBuf
	ldi temp0, 1
	call TKN_PushPacket
	reti

ISR_10:
    ldi ZL, LOW(strbad10 << 1)
    ldi ZH, HIGH(strbad10 << 1)
    call fillPacketBuf
	ldi temp0, 1
	call TKN_PushPacket
	reti
   
    ISR_11:
    ldi ZL, LOW(strbad11 << 1)
    ldi ZH, HIGH(strbad11 << 1)
    call fillPacketBuf
	ldi temp0, 1
	call TKN_PushPacket
	reti

ISR_12:
    ldi ZL, LOW(strbad12 << 1)
    ldi ZH, HIGH(strbad12 << 1)
    call fillPacketBuf
	ldi temp0, 1
	call TKN_PushPacket
	reti

ISR_13:
    ldi ZL, LOW(strbad13 << 1)
    ldi ZH, HIGH(strbad13 << 1)
    call fillPacketBuf
	ldi temp0, 1
	call TKN_PushPacket
	reti

ISR_14:
    ldi ZL, LOW(strbad14 << 1)
    ldi ZH, HIGH(strbad14 << 1)
    call fillPacketBuf
	ldi temp0, 1
	call TKN_PushPacket
	reti

ISR_15:
    ldi ZL, LOW(strbad15 << 1)
    ldi ZH, HIGH(strbad15 << 1)
    call fillPacketBuf
	ldi temp0, 1
	call TKN_PushPacket
	reti

ISR_16:
    ldi ZL, LOW(strbad16 << 1)
    ldi ZH, HIGH(strbad16 << 1)
    call fillPacketBuf
	ldi temp0, 1
	call TKN_PushPacket
	reti

ISR_17:
    ldi ZL, LOW(strbad17 << 1)
    ldi ZH, HIGH(strbad17 << 1)
    call fillPacketBuf
	ldi temp0, 1
	call TKN_PushPacket
	reti

ISR_18:
    ldi ZL, LOW(strbad18 << 1)
    ldi ZH, HIGH(strbad18 << 1)
    call fillPacketBuf
	ldi temp0, 1
	call TKN_PushPacket
	reti

ISR_19:
    ldi ZL, LOW(strbad19 << 1)
    ldi ZH, HIGH(strbad19 << 1)
    call fillPacketBuf
	ldi temp0, 1
	call TKN_PushPacket
	reti

ISR_20:
    ldi ZL, LOW(strbad20 << 1)
    ldi ZH, HIGH(strbad20 << 1)
    call fillPacketBuf
	ldi temp0, 1
	call TKN_PushPacket
	reti

    
    ISR_21:
    ldi ZL, LOW(strbad21 << 1)
    ldi ZH, HIGH(strbad21 << 1)
    call fillPacketBuf
	ldi temp0, 1
	call TKN_PushPacket
	reti

ISR_22:
    ldi ZL, LOW(strbad22 << 1)
    ldi ZH, HIGH(strbad22 << 1)
    call fillPacketBuf
	ldi temp0, 1
	call TKN_PushPacket
	reti

ISR_23:
    ldi ZL, LOW(strbad23 << 1)
    ldi ZH, HIGH(strbad23 << 1)
    call fillPacketBuf
	ldi temp0, 1
	call TKN_PushPacket
	reti

ISR_24:
    ldi ZL, LOW(strbad24 << 1)
    ldi ZH, HIGH(strbad24 << 1)
    call fillPacketBuf
	ldi temp0, 1
	call TKN_PushPacket
	reti

ISR_25:
    ldi ZL, LOW(strbad25 << 1)
    ldi ZH, HIGH(strbad25 << 1)
    call fillPacketBuf
	ldi temp0, 1
	call TKN_PushPacket
	reti

ISR_26:
    ldi ZL, LOW(strbad26 << 1)
    ldi ZH, HIGH(strbad26 << 1)
    call fillPacketBuf
	ldi temp0, 1
	call TKN_PushPacket
	reti

ISR_27:
    ldi ZL, LOW(strbad27 << 1)
    ldi ZH, HIGH(strbad27 << 1)
    call fillPacketBuf
	ldi temp0, 1
	call TKN_PushPacket
	reti

ISR_28:
    ldi ZL, LOW(strbad28 << 1)
    ldi ZH, HIGH(strbad28 << 1)
    call fillPacketBuf
	ldi temp0, 1
	call TKN_PushPacket
	reti

ISR_29:
    ldi ZL, LOW(strbad29 << 1)
    ldi ZH, HIGH(strbad29 << 1)
    call fillPacketBuf
	ldi temp0, 1
	call TKN_PushPacket
	reti

ISR_30:
    ldi ZL, LOW(strbad30 << 1)
    ldi ZH, HIGH(strbad30 << 1)
    call fillPacketBuf
	ldi temp0, 1
	call TKN_PushPacket
	reti

