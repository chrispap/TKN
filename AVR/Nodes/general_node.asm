.include "m644PAdef.inc"

.cseg
.org 0x0000
    jmp Reset
.org INT0addr   ; External Interrupt Request 0
    reti
.org INT1addr   ; External Interrupt Request 1
    reti
.org INT2addr   ; External Interrupt Request 2
    reti
.org PCI0addr   ; Pin Change Interrupt Request 0
    reti
.org PCI1addr   ; Pin Change Interrupt Request 1
    reti
.org PCI2addr   ; Pin Change Interrupt Request 2
    reti
.org PCI3addr   ; Pin Change Interrupt Request 3
    reti
.org WDTaddr    ; Watchdog Time-out Interrupt
    reti
.org OC2Aaddr   ; Timer/Counter2 Compare Match A
    reti
.org OC2Baddr   ; Timer/Counter2 Compare Match B
    reti
.org OVF2addr   ; Timer/Counter2 Overflow
    reti
.org ICP1addr   ; Timer/Counter1 Capture Event
    reti
.org OC1Aaddr   ; Timer/Counter1 Compare Match A
    reti
.org OC1Baddr   ; Timer/Counter1 Compare Match B
    reti
.org OVF1addr   ; Timer/Counter1 Overflow
    reti
.org OC0Aaddr   ; Timer/Counter0 Compare Match A
    reti
.org OC0Baddr   ; Timer/Counter0 Compare Match B
    reti
.org OVF0addr   ; Timer/Counter0 Overflow
    reti
.org SPIaddr    ; SPI Serial Transfer Complete
    reti
.org URXC0addr  ; USART0, Rx Complete
    reti
.org UDRE0addr  ; USART0 Data register Empty
    reti
.org UTXC0addr  ; USART0, Tx Complete
    reti
.org ACIaddr    ; Analog Comparator
    reti
.org ADCCaddr   ; ADC Conversion Complete
    reti
.org ERDYaddr   ; EEPROM Ready
    reti
.org TWIaddr    ; 2-wire Serial Interface
    reti
.org SPMRaddr   ; Store Program Memory Read
    reti
.org URXC1addr  ; USART1 RX complete
    reti
.org UDRE1addr  ; USART1 Data Register Empty
    reti
.org UTXC1addr  ; USART1 TX complete
    reti

.equ TKN_PACKET_SIZE    = 0x10
.equ TKN_pushPacket     = 0x7053
.equ TKN_popPacket      = 0x707e
.equ fillPacketBuf      = 0x73f0
.equ bin1ToHex2         = 0x7309
.equ setLeds            = 0x727a

.def temp0              = r16
.def temp1              = r17
.def temp2              = r18

Reset:
    jmp main

.dseg
.org $400
packetBuff: .byte TKN_PACKET_SIZE

.cseg
str:    .db "Node with vector"

main:
    ldi YL, LOW(packetBuff)
    ldi YH, HIGH(packetBuff)

    ldi ZL, LOW(str<<1)
    ldi ZH, HIGH(str<<1)

    call fillPacketBuf

    ldi temp0, 1
    call TKN_pushPacket
    ret
