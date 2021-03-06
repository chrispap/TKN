.include "m644PAdef.inc"

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
.def duty_cycle         = r21

.dseg
.org SRAM_START
packetBuff: .byte TKN_PACKET_SIZE

.cseg
;Vectors
.org 0x0000
    jmp main

main:
    ldi YL, LOW(packetBuff)
    ldi YH, HIGH(packetBuff)

    ; Init vars
    ldi duty_cycle, $FF
    
    ; Setup timer
    ldi temp0, 0xC3
    out TCCR0A, temp0
    out OCR0A, duty_cycle
    
    ; Start timer
    ldi temp0, 0x01
    out TCCR0B, temp0

loop:
    ; Read new del_replay
    call TKN_popPacket
    and temp0, temp0
    breq loop
    ldd duty_cycle, Y+0
    and duty_cycle, duty_cycle
    breq exit
    out OCR0A, duty_cycle
    rjmp loop
    
exit:
    ; Stop timer
    clr temp0
    out TCCR0B ,temp0
    out TCCR0A ,temp0
    ret
