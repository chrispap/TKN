/*
=====================
= Interrupt Vectors =
=====================
*/

.cseg

;org 0x0000
.org LARGEBOOTSTART

	jmp Reset
	reti			; External Interrupt Request 0
	nop
	reti			; External Interrupt Request 1
	nop
	reti			; External Interrupt Request 2
	nop
	jmp PCI0_ISR	; Pin Change Interrupt Request 0
	
	reti			; Pin Change Interrupt Request 1
	nop
	reti			; Pin Change Interrupt Request 2
	nop
	reti			; Pin Change Interrupt Request 3
	nop
	reti			; Watchdog Time-out Interrupt
	nop
	reti			; Timer/Counter2 Compare Match A
	nop
	reti			; Timer/Counter2 Compare Match B
	nop
	reti			; Timer/Counter2 Overflow
	nop
	reti			; Timer/Counter1 Capture Event
	nop
	reti			; Timer/Counter1 Compare Match A
	nop
	reti			; Timer/Counter1 Compare Match B
	nop
	reti			; Timer/Counter1 Overflow
	nop
	reti			; Timer/Counter0 Compare Match A
	nop
	reti			; Timer/Counter0 Compare Match B
	nop
	reti			; Timer/Counter0 Overflow
	nop
	reti			; SPI Serial Transfer Complete
	nop
	jmp URXC0_ISR	; USART0, Rx Complete
	
	jmp US0TKN_TX_ISR ; USART0 Data register Empty

	reti			; USART0, Tx Complete
	nop
	reti			; Analog Comparator
	nop
	reti			; ADC Conversion Complete
	nop
	reti			; EEPROM Ready
	nop
	reti			; 2-wire Serial Interface
	nop
	reti			; Store Program Memory Read
	nop
	reti			; USART1 RX complete
	nop
	reti			; USART1 Data Register Empty
	nop
	reti			; USART1 TX complete
	nop
	
