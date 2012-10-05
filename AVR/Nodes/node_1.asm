.include "m644PAdef.inc"

.def temp0 = r16
.def temp1 = r17
.def temp2 = r18
.def counter = r20
.def speed = r21

.equ TKN_PACKET_SIZE	= 16
.equ TKN_pushPacket		= 0x7053
.equ TKN_popPacket		= 0x707e
.equ fillPacketBuf		= 0x73f0
.equ bin1ToHex2			= 0x72c8
.equ setLeds			= 0x727a

.cseg
;Vectors
.org 0x0000
    jmp main

.dseg
.org $400
packetBuff:	.byte TKN_PACKET_SIZE

.cseg
str:	.db "Timer counted!  "

main:
    ldi YL, LOW(packetBuff)
    ldi YH, HIGH(packetBuff)
    ldi ZL, LOW(str << 1)
    ldi ZH, HIGH(str << 1)
    call fillPacketBuf
    ldi counter, $10
	ldi speed, 1
    
	; Start timer
    clr temp0
    out TCNT0, temp0
    clr temp0
    out TCCR0A, temp0
    ldi temp0, 0x01
    out TCCR0B, temp0
    
loop:
	; Toggle LED
	mov temp0, counter
	ror temp0
	ror temp0
	call setLeds
	; Read new speed
	call TKN_popPacket
	and temp0, temp0
	breq speed_the_same
	ldd speed, Y+0
	and speed, speed
	breq return
speed_the_same:
	mov temp1, speed
	clr temp0
    out TCNT0, temp0
lp:
    in temp0, TCNT0
    cpi temp0, $FF
    brne lp
    dec temp1
    brne lp
    dec counter
	rjmp loop
return:
	; Stop timer
    clr temp0
    out TCCR0A ,temp0
    out TCCR0B ,temp0
    ret
