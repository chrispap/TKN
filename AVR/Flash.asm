/*==============================================================
===This routine writes one page of data from RAM to Flash
=== 
=== - the first data location in RAM is pointed to by the Y pointer
===   the first data location in Flash is pointed to by the Z-pointer
=== - error handling is not included
=== - the routine must be placed inside the Boot space
===   (at least the BL_Do_spm sub routine). Only code inside NRWW section can
===   be read during Self-Programming (Page Erase and Page Write).
=== 
=== - registers used: 
===       r0, r1, temp1 (r16), temp2 (r17), looplo (r24), loophi (r25), spmcrval (r20)
=== 
=== - storing and restoring of registers is not included in the routine
===   register usage can be optimized at the expense of code size
=== - It is assumed that either the interrupt table is moved to the Boot
===   loader section or that the interrupts are disabled.
================================================================*/

.def spmcrval = r23
.def looplo   = r24
.def loophi   = r25

.equ PAGESIZEB = PAGESIZE*2 ;PAGESIZEB is page size in BYTES, not words
;;;.org SMALLBOOTSTART

BL_Write_page:
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
	sbrs temp1, RWWSB ; If RWWSB is set, the RWW section is not ready yet
	ret
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
	// Should implement error handling
	ret
