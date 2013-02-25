/*******************************************
 * Code responsible to handle the USART...
 *	1. Initialization
 *	2. Byte transmission
 *	3. Byte Reception
 *******************************************/

/*==============================================================
=== USART INIT =================================================
================================================================*/
USART_Init:
	; Set baud rate (2400bps,  8MHz -> UBRR=207)
	;               (4800bps,  8MHz -> UBRR=103)
	;               (9600bps,  8MHz -> UBRR= 51)
	;               (38400bps, 8MHz -> UBRR= 12)
	;               (57600bps, 8MHz -> UBRR=  8)
	;               (115200bps,8MHz -> UBRR=  3)
	ldi temp0, 0
	sts UBRR0H, temp0
	ldi temp0, 8
	sts UBRR0L, temp0

	; Enable receiver, transmitter, Rx Interrupt, //TxDataRegReady Interrupt.
	ldi temp0, (1<<RXEN0)|(1<<TXEN0)|(1<<RXCIE0)//|(1<<UDRIE0)
	sts UCSR0B,temp0

	; Set frame format: 8data, 1stop bit
	ldi temp0, (3<<UCSZ00)
	sts UCSR0C,temp0

	ret

/*==============================================================
=== USART TRANSMIT =============================================
================================================================*/
USART_Transmit:
	; Wait for empty transmit buffer
	lds  temp1, UCSR0A
	sbrs temp1, UDRE0
	rjmp USART_Transmit

	; Put temp register into the send buffer.
	sts  UDR0, temp0

	ret

/*==============================================================
=== USART RECEIVE ==============================================
================================================================*/
USART_Receive:
	; Wait for data to be received
	lds  temp0, UCSR0A
	sbrs temp0, RXC0
	rjmp USART_Receive

	; Get and return received data from buffer
	lds temp0, UDR0

	ret

/*******************************************
 * ISRs for the USART
 *******************************************/

/*==============================================================
=== USART0 Rx Complete ISR =====================================
================================================================*/
URXC0_ISR:
	push temp0
	push temp1
	push temp2
	push XL
	push XH
	push YL
	push YH
	push ZL
	push ZH
	push rsreg
	in rsreg, SREG
	
	call USART_Receive
	call TKN_Receiver

	out SREG, rsreg
	pop rsreg
	pop ZH
	pop ZL
	pop YH
	pop YL
	pop XH
	pop XL
	pop temp2
	pop temp1
	pop temp0

	reti

/*==============================================================
=== USART0 Data Register Ready ISR==============================
================================================================*/
US0TKN_TX_ISR:
	push temp0
	push temp1
	push temp2
	push XL
	push XH
	push YL
	push YH
	push ZL
	push ZH
	push rsreg
	in rsreg, SREG
	
	call TKN_Transmitter

	out SREG, rsreg
	pop rsreg
	pop ZH
	pop ZL
	pop YH
	pop YL
	pop XH
	pop XL
	pop temp2
	pop temp1
	pop temp0

	reti
    
