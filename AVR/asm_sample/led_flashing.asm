.include "m644PAdef.inc"

.equ TKN_PACKET_SIZE    = 0x10

.equ TKN_pushPacket     = 0x7062
.equ TKN_popPacket      = 0x7098
.equ fillPacketBuf      = 0x7434
.equ bin1ToHex2         = 0x734d
.equ setLeds            = 0x72c0

.def temp0              = r16
.def temp1              = r17
.def temp2              = r18
.def del_replays        = r21

.dseg
.org SRAM_START
packetBuff: .byte TKN_PACKET_SIZE

.cseg
;Vectors
.org 0x0000
    jmp main

str:    .db "Timer counted!  "

main:
    ldi YL, LOW(packetBuff)
    ldi YH, HIGH(packetBuff)
    ldi ZL, LOW(str << 1)
    ldi ZH, HIGH(str << 1)
    call fillPacketBuf

    ; Init vars
    ldi del_replays, $5
    
    ; Setup timer
    ldi temp0, 0x00
    out TCCR0A, temp0
    
    ; Start timer
    ldi temp0, 0x04
    out TCCR0B, temp0

loop:
    in temp0, PINB
    ldi temp1, $08
    eor temp0, temp1
    out PORTB, temp0
    
    mov temp1, del_replays
delay:
    in temp0, TIFR0
    sbrs temp0, TOV0
    rjmp delay
    sbr temp0, (1<<TOV0)
    out TIFR0, temp0
    dec temp1
    brne delay
    
    ; Read new del_replay
    call TKN_popPacket
    and temp0, temp0
    breq loop
    ldd del_replays, Y+0
    and del_replays, del_replays
    breq exit   
    rjmp loop
    
exit:
    ; Stop timer
    clr temp0
    out TCCR0B ,temp0
    out TCCR0A ,temp0
	;Clear Leds
	ser temp0
	out PORTB, temp0
    ret
