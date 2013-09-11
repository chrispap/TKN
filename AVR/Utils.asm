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
	ret
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
	ret
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
	; Wait for completion of previous write
	sbic EECR,EEPE
	rjmp EEPROM_read
    ; Store regs used
    push temp1
    push temp2
	; Set up address (temp2:temp1) in address register
	out EEARL, temp1
	out EEARH, temp2
	; Start eeprom read by writing EERE
	sbi EECR, EERE
	; Read data from Data Register
	in temp0, EEDR
    ; Restore regs used
	pop temp2
	pop temp1
	ret

/*==============================================================
=== - This routine writes one page of data from RAM to Flash
=== 
=== @param Y: First data location in RAM
=== @param Z: First data location in Flash
===
=== - Registers stored before used:
===		r0, r1, r20, r24, r25
===
=== - Registers destroyed:
===		temp1, temp2
=== 
=== - It is assumed that either the interrupt table is moved to
===   the Boot section or that the interrupts are disabled.
================================================================*/
.def spmcrval = r23
.def looplo   = r24
.def loophi   = r25

.equ PAGESIZEB = PAGESIZE*2 ;PAGESIZEB is page size in BYTES, not words

BL_Write_page:
	push r0
	push r1
	push looplo
	push loophi
	push spmcrval

	; Page Erase
	ldi spmcrval, (1<<PGERS) | (1<<SPMEN)
	call BL_Do_spm
	; re-enable the RWW section
	ldi spmcrval, (1<<RWWSRE) | (1<<SPMEN)
	call BL_Do_spm
	; transfer data from RAM to Flash page buffer
	ldi looplo, low(PAGESIZEB) ;init loop variable
	ldi loophi, high(PAGESIZEB) ;not required for PAGESIZEB<=256

	BL_Wrloop:
		ld r0, Y+
		ld r1, Y+
		ldi spmcrval, (1<<SPMEN)
		call BL_Do_spm
		adiw ZH:ZL, 2
		sbiw loophi:looplo, 2 ;use subi for PAGESIZEB<=256
	brne BL_Wrloop

	; execute Page Write
	subi ZL, low(PAGESIZEB) ;restore pointer
	sbci ZH, high(PAGESIZEB) ;not required for PAGESIZEB<=256
	ldi spmcrval, (1<<PGWRT) | (1<<SPMEN)
	call BL_Do_spm
	; re-enable the RWW section
	ldi spmcrval, (1<<RWWSRE) | (1<<SPMEN)
	call BL_Do_spm
	; read back and check, optional
	ldi looplo, low(PAGESIZEB) ;init loop variable
	ldi loophi, high(PAGESIZEB) ;not required for PAGESIZEB<=256
	subi YL, low(PAGESIZEB) ;restore pointer
	sbci YH, high(PAGESIZEB)

	BL_Rdloop:
		lpm r0, Z+
		ld r1, Y+
		cpse r0, r1
		jmp BL_Error
		sbiw loophi:looplo, 1 ;use subi for PAGESIZEB<=256
	brne BL_Rdloop

BL_Return:
	; BL_Return to RWW section
	; verify that RWW section is safe to read
	in temp1, SPMCSR
	sbrc temp1, RWWSB ; If RWWSB is set, the RWW section is not ready yet
	rjmp BL_reenable
	pop spmcrval
	pop loophi
	pop looplo
	pop r1
	pop r0

	;call blinkLeds
	ret

BL_reenable:
	; re-enable the RWW section
	ldi spmcrval, (1<<RWWSRE) | (1<<SPMEN)
	call BL_Do_spm
	rjmp BL_Return

BL_Do_spm:
	; check for previous SPM complete
	BL_Wait_spm:
		in temp1, SPMCSR
		sbrc temp1, SPMEN
	rjmp BL_Wait_spm

	; input: spmcrval determines SPM action
	; disable interrupts if enabled, store status
	in temp2, SREG
	cli

	; check that no EEPROM write access is present
	BL_Wait_ee:
		sbic EECR, EEPE
	rjmp BL_Wait_ee

	; SPM timed sequence
	out SPMCSR, spmcrval
	spm
	; restore SREG (to enable interrupts if originally enabled)
	out SREG, temp2
	ret

BL_Error:
	; Should implement error handling some day...
	ret

/*==============================================================
=== - Bin2ToHex4
===		Converts a 16-bit-binary to uppercase Hex-ASCII
=== 
=== - In: 16-bit-binary in temp1:temp2, Z points to first
===		position of the four-character Hex-ASCII
===
===	- Out: Z points after the last digit of the four-character
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
	;sbiw ZL,4 ; Set Z to start
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
