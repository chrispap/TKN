.def rsreg = r16	;Register usage - Inside ISRs
.def itemp0= r17	;
.def itemp1= r18	;
.def itemp2= r19	;

.def temp0 = r20	;Register usage - Inside main thread
.def temp1 = r21	;
.def temp2 = r22	;

.def mVar  = r23	;Some general purpose main thread variable.

.dseg
dataPacket:
	.byte TKN_PACKET_SIZE

.cseg

tempBuff:
;.db "DATADATADATADATA" // 16 bytes!
;.db "----------------" // 16 bytes!
 .db "Chris Papapaulou"
 
/*==============================================================
=== Main
================================================================*/
main:
	call ledInit
	call TKN_init
	call Enable_PcInt7
	ldi mVar, $01

	;Fill the data packet.
	ldi ZL, LOW(tempBuff<<1)
	ldi ZH, HIGH(tempBuff<<1)

	ldi YH,HIGH(dataPacket)
	ldi YL,LOW(dataPacket)

	ldi temp1, TKN_PACKET_SIZE
fillPacketLoop:
	lpm temp0, Z+
	cbr temp0, $80
	nop
	st Y+, temp0
	dec temp1;
	brne fillPacketLoop

	;Move Interrupts in boot section
	;Enable Interrupts and enter the main loop.
	call Move_interrupts_BootSec
	sei
sleep_loop:
	sleep
	rjmp sleep_loop




