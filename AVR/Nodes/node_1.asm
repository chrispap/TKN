.include "m644PAdef.inc"

.cseg

;Vectors
.org 0x0000
	jmp Reset

	.def temp0 = r16
	.def temp1 = r17
	.def temp2 = r18

	.equ TKN_PACKET_SIZE	= 16
	.equ TKN_pushPacket		= 0x7053
	.equ TKN_popPacket		= 0x707e
	.equ fillPacketBuf		= 0x73f0

;Reset
Reset:

    jmp main

;Main
.dseg
.org $400
packetBuff:	.byte TKN_PACKET_SIZE

.cseg
;  :    .db "----------------"
str:	.db "Hex for node 1  "

main:

    ldi YL, LOW(packetBuff)
	ldi YH, HIGH(packetBuff)

	ldi ZL, LOW(str<<1)
	ldi ZH, HIGH(str<<1)

	call fillPacketBuf

	ldi temp0, 1
	call TKN_pushPacket

	ret

