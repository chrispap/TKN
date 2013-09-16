.include "m644PAdef.inc"

.equ TKN_PACKET_SIZE    = 0x10

.equ TKN_pushPacket     = 0x7062
.equ TKN_popPacket      = 0x7098
.equ fillPacketBuf      = 0x7436
.equ Bin2ToHex4         = 0x7348
.equ Bin1ToHex2         = 0x734f
.equ Bin1ToHex1         = 0x7353
.equ setLeds            = 0x72c4

.def temp0              = r16
.def temp1              = r17
.def temp2              = r18
.def duty_cycle         = r21

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
    ldi duty_cycle, $5
    
    ; Setup timer
    ldi temp0, 0xC3
    out TCCR0A, temp0
    ldi temp0, 50
    out OCR0A, duty_cycle
    
    ; Start timer
    ldi temp0, 0x04
    out TCCR0B, temp0

loop:

dimmUp:
    in temp0, TIFR0
    sbrs temp0, TOV0
    rjmp dimmUp
    sbr temp0, (1<<TOV0)
    out TIFR0, temp0
    inc duty_cycle
    cpi duty_cycle, $00
    breq ReachedUp
    out OCR0A, duty_cycle
    rjmp dimmUp
ReachedUp:
    ser duty_cycle
dimmDown:
    in temp0, TIFR0
    sbrs temp0, TOV0
    rjmp dimmDown
    sbr temp0, (1<<TOV0)
    out TIFR0, temp0
    dec duty_cycle
    and duty_cycle, duty_cycle
    out OCR0A, duty_cycle
    brne dimmDown
    
    ; Read new del_replay
    call TKN_popPacket
    and temp0, temp0
    breq loop
    ldd temp0, Y+0
    and temp0, temp0
    breq exit   
    
    rjmp loop
    
exit:
    ; Stop timer
    clr temp0
    out TCCR0B ,temp0
    out TCCR0A ,temp0
    ret
