.include "m644PAdef.inc"

/*===============
= Int Vectors 
=================*/
.cseg
.org 0x0000
	jmp Reset
.org INT0addr	; External Interrupt Request 0
	reti
.org INT1addr	; External Interrupt Request 1
	reti
.org INT2addr	; External Interrupt Request 2
	reti
.org PCI0addr	; Pin Change Interrupt Request 0
	reti
.org PCI1addr	; Pin Change Interrupt Request 1
	reti
.org PCI2addr	; Pin Change Interrupt Request 2
	reti
.org PCI3addr	; Pin Change Interrupt Request 3
	reti
.org WDTaddr	; Watchdog Time-out Interrupt
	reti
.org OC2Aaddr	; Timer/Counter2 Compare Match A
	reti
.org OC2Baddr	; Timer/Counter2 Compare Match B
	reti
.org OVF2addr	; Timer/Counter2 Overflow
	reti
.org ICP1addr	; Timer/Counter1 Capture Event
	reti
.org OC1Aaddr	; Timer/Counter1 Compare Match A
	reti
.org OC1Baddr	; Timer/Counter1 Compare Match B
	reti
.org OVF1addr	; Timer/Counter1 Overflow
	reti
.org OC0Aaddr	; Timer/Counter0 Compare Match A
	reti
.org OC0Baddr	; Timer/Counter0 Compare Match B
	reti
.org OVF0addr	; Timer/Counter0 Overflow
	reti
.org SPIaddr	; SPI Serial Transfer Complete
	reti
.org URXC0addr	; USART0, Rx Complete
	reti
.org UDRE0addr	; USART0 Data register Empty
	reti
.org UTXC0addr	; USART0, Tx Complete
	reti
.org ACIaddr	; Analog Comparator
	reti
.org ADCCaddr	; ADC Conversion Complete
	reti
.org ERDYaddr	; EEPROM Ready
	reti
.org TWIaddr	; 2-wire Serial Interface
	reti
.org SPMRaddr	; Store Program Memory Read
	reti
.org URXC1addr	; USART1 RX complete
	reti
.org UDRE1addr	; USART1 Data Register Empty
	reti
.org UTXC1addr	; USART1 TX complete
	reti

/*===============
= Reset 
=================*/
Reset:
	// Init Stack Pointer
	ldi r16,low(RAMEND)
	out SPL,r16
	ldi r16,high(RAMEND)
	out SPH,r16

	; Configure PORTB as output  (LEDs are connected to this port)
	; All bits have logic zero output
	ldi  r16,0b11111111
	out  PORTB,r16
	ldi  r16,0b11111111	// All bits are set to OUTPUT
	out  DDRB,r16

	; Configure PORTA as input (Switches are connected to this port)
	ldi  r16,0b11111111
	out  PORTA,r16
	ldi  r16,0b00000000
	out  DDRA,r16

	.equ LEDS_OUT   = PORTB
	.equ LEDS_IN    = PINB
	.equ SWITCHES_IN= PINA

/*===============
= Buffers in SRAM 
=================*/

.dseg
.org SRAM_START
dataLabel:
	.byte 16	// Example: How to reserve 16 bytes to internal RAM
	
/*===============
= Main
=================*/
.cseg

main:
	in r3, SWITCHES_IN
	out LEDS_OUT, r3
	jmp main




