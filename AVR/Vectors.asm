/*
===============================================================
= Interrupt Vectors ===========================================
=
= Unused interrupts -> user app.
===============================================================
*/

.cseg
.equ MY_BOOTSTART = LARGEBOOTSTART
.org MY_BOOTSTART

/* Ignore any unused interrupt */
/*
jmp Reset
reti nop			; External Interrupt Request 0
reti nop			; External Interrupt Request 1
reti nop			; External Interrupt Request 2
jmp PCI0_ISR				; Pin Change Interrupt Request 0
reti nop			; Pin Change Interrupt Request 1
reti nop			; Pin Change Interrupt Request 2
reti nop			; Pin Change Interrupt Request 3
reti nop			; Watchdog Time-out Interrupt
reti nop			; Timer/Counter2 Compare Match A
reti nop			; Timer/Counter2 Compare Match B
reti nop			; Timer/Counter2 Overflow
reti nop			; Timer/Counter1 Capture Event
reti nop			; Timer/Counter1 Compare Match A
reti nop			; Timer/Counter1 Compare Match B
reti nop			; Timer/Counter1 Overflow
reti nop			; Timer/Counter0 Compare Match A
reti nop			; Timer/Counter0 Compare Match B
reti nop			; Timer/Counter0 Overflow
reti nop			; SPI Serial Transfer Complete
jmp URXC0_ISR				; USART0, Rx Complete
jmp US0TKN_TX_ISR 			; USART0 Data register Empty
reti nop			; USART0, Tx Complete
reti nop			; Analog Comparator
reti nop			; ADC Conversion Complete
reti nop			; EEPROM Ready
reti nop			; 2-wire Serial Interface
reti nop			; Store Program Memory Read
reti nop			; USART1 RX complete
reti nop			; USART1 Data Register Empty
reti nop			; USART1 TX complete
*/

/* Redirect all unused interrupts to user section */
jmp Reset
jmp PC-MY_BOOTSTART			; External Interrupt Request 0
jmp PC-MY_BOOTSTART			; External Interrupt Request 1
jmp PC-MY_BOOTSTART			; External Interrupt Request 2
jmp PCI0_ISR				; Pin Change Interrupt Request 0
jmp PC-MY_BOOTSTART			; Pin Change Interrupt Request 1
jmp PC-MY_BOOTSTART			; Pin Change Interrupt Request 2
jmp PC-MY_BOOTSTART			; Pin Change Interrupt Request 3
jmp PC-MY_BOOTSTART			; Watchdog Time-out Interrupt
jmp PC-MY_BOOTSTART			; Timer/Counter2 Compare Match A
jmp PC-MY_BOOTSTART			; Timer/Counter2 Compare Match B
jmp PC-MY_BOOTSTART			; Timer/Counter2 Overflow
jmp PC-MY_BOOTSTART			; Timer/Counter1 Capture Event
jmp PC-MY_BOOTSTART			; Timer/Counter1 Compare Match A
jmp PC-MY_BOOTSTART			; Timer/Counter1 Compare Match B
jmp PC-MY_BOOTSTART			; Timer/Counter1 Overflow
jmp PC-MY_BOOTSTART			; Timer/Counter0 Compare Match A
jmp PC-MY_BOOTSTART			; Timer/Counter0 Compare Match B
jmp PC-MY_BOOTSTART			; Timer/Counter0 Overflow
jmp PC-MY_BOOTSTART			; SPI Serial Transfer Complete
jmp URXC0_ISR				; USART0, Rx Complete
jmp US0TKN_TX_ISR 			; USART0 Data register Empty
jmp PC-MY_BOOTSTART			; USART0, Tx Complete
jmp PC-MY_BOOTSTART			; Analog Comparator
jmp PC-MY_BOOTSTART			; ADC Conversion Complete
jmp PC-MY_BOOTSTART			; EEPROM Ready
jmp PC-MY_BOOTSTART			; 2-wire Serial Interface
jmp PC-MY_BOOTSTART			; Store Program Memory Read
jmp PC-MY_BOOTSTART			; USART1 RX complete
jmp PC-MY_BOOTSTART			; USART1 Data Register Empty
jmp PC-MY_BOOTSTART			; USART1 TX complete
