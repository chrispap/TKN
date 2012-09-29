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


/*==============================================================
=== - Bin2ToHex4
===		Converts a 16-bit-binary to uppercase Hex-ASCII
=== 
=== - In: 16-bit-binary in temp1:temp2, Z points to first
===		position of the four-character Hex-ASCII
===
===	- Out: Z points to the first digit of the four-character
===		Hex-ASCII, ASCII digits A..F in capital letters
=== 
=== - Used registers: temp1:temp2 (unchanged), temp0 (unchanged)
================================================================*/
Bin2ToHex4:
	push temp0
	mov temp0, temp1 ; load MSB
	rcall Bin1ToHex2 ; convert byte
	mov temp0,temp2
	rcall Bin1ToHex2
	sbiw ZL,4 ; Set Z to start
	pop temp0
	ret
;
; Bin1ToHex2 converts an 8-bit-binary to uppercase hex
; Called by: Bin2ToHex4
;
Bin1ToHex2:
	push temp0 ; Save byte
	swap temp0 ; upper to lower nibble
	rcall Bin1ToHex1
	pop temp0 ; Restore byte
Bin1ToHex1:
	andi temp0,$0F ; mask upper nibble
	subi temp0,-'0' ; add 0 to convert to ASCII
	cpi temp0,'9'+1 ; A..F?
	brcs Bin1ToHex1a
	subi temp0,-7 ; add 7 for A..F
Bin1ToHex1a:
	st z+,temp0 ; store in target
	ret ; and return