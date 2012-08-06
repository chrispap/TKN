/*==============================================================
=== LED Routines: ===
===		setLedsX:
=== 
================================================================*/
.dseg
ledValues: 
	.byte 2

.cseg
ledInit:
	push YL
	push YH
	push temp0

	clr temp0

	ldi YL, LOW(ledValues)
	ldi YH, HIGH(ledValues)
	
	st Y+, temp0
	st Y+, temp0
	
	call setLeds

	pop temp0
	pop YH
	pop YL
	ret

setLeds:
	com temp0
	out LEDS_OUT, temp0
	com temp0 ;Restore register
	ret

setLeds0:
	push YL
	push YH
	push temp1

	ldi YL, LOW(ledValues)
	ldi YH, HIGH(ledValues)

	st Y+, temp0
	ld temp1, Y
	
	or temp1, temp0
	com temp1
	out LEDS_OUT, temp1
	
	pop temp1
	pop YH
	pop YL
	ret

setLeds1:
	push YL
	push YH
	push temp1

	ldi YL, LOW(ledValues)
	ldi YH, HIGH(ledValues)

	std Y+1, temp0
	ld temp1, Y

	or temp1, temp0
	com temp1
	out LEDS_OUT, temp1
	
	pop temp1
	pop YH
	pop YL
	ret

/*==============================================================
=== Make an effect with the LEDs ===============================
=== Dependencies: LEDS_OUT points to the led port
================================================================*/
blinkLeds:
	;save state
	push temp0
	push temp1

	in temp1, LEDS_IN
	ser temp0
blink:
	call setLeds
	call delay
	lsr  temp0
	sbrc temp0, 0
	rjmp blink
	
	;restore state
	out LEDS_OUT, temp1
	pop temp1
	pop temp0
	ret

/*==============================================================
=== A small software delay =====================================
================================================================*/
delay:
	;save state
	push temp0
	push temp1

	clr  temp0
dl1:clr  temp1
dl2:dec  temp1
	brne dl2
	dec  temp0
	brne dl1

	;restore state
	pop temp1
	pop temp0
	ret

/*==============================================================
=== EEPROM Read ================================================
=== Read 1 byte from EEPROM to temp0 register.
=== It is assumed that interrupts are controlled so that 
=== no interrupts will occur during execution of this routine.
================================================================*/
EEPROM_read:
	push temp1
	push temp2

	; Wait for completion of previous write
	sbic EECR,EEPE
	rjmp EEPROM_read
	; Set up address (temp2:temp1) in address register
	out EEARL, temp1
	out EEARH, temp2
	; Start eeprom read by writing EERE
	sbi EECR, EERE
	; Read data from Data Register
	in temp0, EEDR

	pop temp2
	pop temp1
	ret




