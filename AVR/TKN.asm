;Packet Byte offsets
.equ TKN_OFFS_HEADER    = 0
.equ TKN_OFFS_TYPE      = 1
.equ TKN_OFFS_RECEIVER  = 2
.equ TKN_OFFS_SENDER    = 3
.equ TKN_OFFS_TOKEN_EOF = 4
.equ TKN_OFFS_PACKET_ID = 4
.equ TKN_OFFS_CONTROL   = 5
.equ TKN_OFFS_ACK_EOF   = 5
.equ TKN_OFFS_DATA_START= 6
.equ TKN_OFFS_DATA_STOP = 21
.equ TKN_OFFS_DATA_EOF  = 22

.equ TKN_HEADER_BYTE    = $00
.equ TKN_EOF_BYTE       = $FF

.equ TKN_TYPE_DATA      = $AA
.equ TKN_TYPE_ACK       = $BB
.equ TKN_TYPE_TOKEN     = $CC

.equ TKN_MODE_DATA      = $01
.equ TKN_MODE_ACK       = $02
.equ TKN_MODE_TOKEN     = $03

;Packet Sizes
.equ TKN_PACKET_SIZE     = 16
.equ TKN_QUEUE_LEN       = 8
.equ TKN_QUEUE_SIZE      = TKN_PACKET_SIZE * TKN_QUEUE_LEN
.equ TKN_MAX_PACKET_SIZE = TKN_OFFS_DATA_EOF + 1

/*==============================================================
=== - Register usage
================================================================*/
;Node
.def TKN_ID					= r0
;Transmitter
.def TKN_TX_QUEUE_INDEX_IN  = r1
.def TKN_TX_QUEUE_INDEX_OUT = r2
.def TKN_TX_PENDING         = r3
.def TKN_TX_PACKET_ID       = r4
.def TKN_TX_STATUS          = r5
.def TKN_TX_BYTE            = r6
;Receiver
.def TKN_RX_QUEUE_INDEX_IN  = r7
.def TKN_RX_QUEUE_INDEX_OUT = r8
.def TKN_RX_PENDING         = r9
.def TKN_RX_BYTE            = r10
.def TKN_RX_STATUS          = r11

/*==============================================================
=== - RAM Data
===		The RX/TX Queues should be 0x80 aligned
================================================================*/
.dseg
.org SRAM_START

TKN_RX_QUEUE:
    .byte TKN_QUEUE_SIZE

TKN_TX_QUEUE:
    .byte TKN_QUEUE_SIZE

TKN_TX_QUEUE_ADDR:
    .byte TKN_QUEUE_LEN

TKN_RX_QUEUE_ADDR:
    .byte TKN_QUEUE_LEN

TKN_TX_BUFFER:
    .byte TKN_MAX_PACKET_SIZE

TKN_RX_BUFFER:
    .byte TKN_MAX_PACKET_SIZE

.cseg
TOKEN_PACKET:
    .db $00, TKN_TYPE_TOKEN, $00, $00, $FF


/*******************************************
 *** C L I E N T  R O U T I N E S        ***
 *******************************************/
/*==============================================================
=== Routine: TKN_init
===     Initialize the data structures used by the network
================================================================*/
TKN_init:
    clr TKN_TX_QUEUE_INDEX_IN
    clr TKN_TX_QUEUE_INDEX_OUT
    clr TKN_TX_PENDING
    clr TKN_TX_STATUS
    clr TKN_TX_BYTE
    clr TKN_TX_PACKET_ID

    clr TKN_RX_QUEUE_INDEX_IN
    clr TKN_RX_QUEUE_INDEX_OUT
    clr TKN_RX_PENDING
    clr TKN_RX_BYTE

    ldi temp1, 0
    ldi temp2, 0
    call EEPROM_read
    mov TKN_ID, temp0
    call USART_Init
    ret

/*==============================================================
=== Routine: TKN_pushPacket
===     Push a packet into the Tx queue
===
=== @param: Y The Address of the packet 
===           to be pushed in the TX_QUEUE
===
=== @param: temp0 The Receiver of the packet
=== 
=== @return: In success -> temp0 = 0;
===          In failure -> temp0 remains as is.
================================================================*/
TKN_pushPacket:
    ;Store SREG for later restorage.
    in temp2, SREG
    cli
    
    ;If there is NOT an empty slot discard the new packet and return;
    mov temp1, TKN_TX_PENDING
    cpi temp1, TKN_QUEUE_LEN
    in temp1, SREG
    sbrc temp1, SREG_Z
    rjmp TKN_pushPacketFullQueue
        
    push XL
    push XH
    push YL
    push YH

    ldi XL, LOW(TKN_TX_QUEUE_ADDR)
    ldi XH, HIGH(TKN_TX_QUEUE_ADDR)
    add XL, TKN_TX_QUEUE_INDEX_IN

    ;Store the ADDRESS
    st X, temp0 
    
    ;Perform lg(PACKET_SIZE) = lg(16) = [4 Left Shifts]
    lsl TKN_TX_QUEUE_INDEX_IN
    lsl TKN_TX_QUEUE_INDEX_IN
    lsl TKN_TX_QUEUE_INDEX_IN
    lsl TKN_TX_QUEUE_INDEX_IN

    ;Copy the packet pointed by Y to the first available queue slot
    ldi XL, LOW(TKN_TX_QUEUE)
    ldi XH, HIGH(TKN_TX_QUEUE)
    add XL, TKN_TX_QUEUE_INDEX_IN

    ldi temp1, TKN_PACKET_SIZE
copyPacket:
    ld temp0, Y+
    st X+, temp0
    dec temp1
    brne copyPacket

    ;Update TKN_TX_PENDING/POINTER_IN
    inc TKN_TX_PENDING

    lsr TKN_TX_QUEUE_INDEX_IN
    lsr TKN_TX_QUEUE_INDEX_IN
    lsr TKN_TX_QUEUE_INDEX_IN
    lsr TKN_TX_QUEUE_INDEX_IN

    ;TKN_TX_QUEUE_INDEX_IN = 
    ;   (TKN_TX_QUEUE_INDEX_IN+PACKET_SIZE)%BUFFER_SIZE
    inc TKN_TX_QUEUE_INDEX_IN
    mov temp0, TKN_TX_QUEUE_INDEX_IN
    cbr temp0, TKN_QUEUE_LEN
    mov TKN_TX_QUEUE_INDEX_IN, temp0
    clr temp0

TKN_pushPacketRet:
    pop YH
    pop YL
    pop XH
    pop XL
    
TKN_pushPacketFullQueue:
    ;Restore Interrupt Flag in its previous state
    out SREG, temp2
    ret

/*==============================================================
=== Routine: token_popPacket
===     Pop a packet from the Rx queue
===
=== @param: Y The Address of the buffer where 
===           the packet will be copied
=== 
=== @return: If RX_QUEUE Empty -> temp0 = 0;
===          Else temp0 = The Sender of the packet.
================================================================*/
TKN_popPacket:
    ;Store SREG for later restorage.
    in temp2, SREG
    cli
    
    ;Check if there is pending data
    mov temp0, TKN_RX_PENDING
    and temp0, temp0
    breq TKN_popPacketEmptyQueue
    
    push XL
    push XH
    push YL
    push YH
    
    /* Copy the first packet to the buffer pointed by Y */
    lsl TKN_RX_QUEUE_INDEX_OUT
    lsl TKN_RX_QUEUE_INDEX_OUT
    lsl TKN_RX_QUEUE_INDEX_OUT
    lsl TKN_RX_QUEUE_INDEX_OUT
    ldi XL, LOW(TKN_RX_QUEUE)
    ldi XH, HIGH(TKN_RX_QUEUE)
    add XL, TKN_RX_QUEUE_INDEX_OUT
    lsr TKN_RX_QUEUE_INDEX_OUT
    lsr TKN_RX_QUEUE_INDEX_OUT
    lsr TKN_RX_QUEUE_INDEX_OUT
    lsr TKN_RX_QUEUE_INDEX_OUT
    ldi temp1, TKN_PACKET_SIZE
TKN_Pop_fetchPacketLoop:
    ld temp0, X+
    st Y+, temp0
    dec temp1
    brne TKN_Pop_fetchPacketLoop
    
    /* temp0 = packet_Sender */
    ldi XL, LOW(TKN_RX_QUEUE_ADDR)
    ldi XH, HIGH(TKN_RX_QUEUE_ADDR)
    add XL, TKN_RX_QUEUE_INDEX_OUT
    ld temp0, x
    
    ;TKN_RX_QUEUE_INDEX_OUT = 
    ;   (TKN_RX_QUEUE_INDEX_OUT+PACKET_SIZE)%BUFFER_SIZE
    inc TKN_RX_QUEUE_INDEX_OUT
    mov temp1, TKN_RX_QUEUE_INDEX_OUT
    cbr temp1, TKN_QUEUE_LEN
    mov TKN_RX_QUEUE_INDEX_OUT, temp1

    dec TKN_RX_PENDING

	mov temp1, temp0
	mov temp0, TKN_RX_PENDING
	call setLeds
	mov temp0, temp1

TKN_popPacketRet:
    pop YH
    pop YL
    pop XH
    pop XL

TKN_popPacketEmptyQueue:
    out SREG, temp2
    ret


/*******************************************
 *** T R A N S M I T T E R               ***
 *******************************************/
/*==============================================================
=== Routine: TKN_byteTransmit
===
===     If there are pending packets in the transmission queue
===     send the next one.
================================================================*/
TKN_Transmitter:
    sbrs TKN_TX_STATUS, 6 ; return if inactive
    ret
    sbrc TKN_TX_STATUS, 5 ; return if waiting for ACK
    ret

    mov temp1, TKN_TX_STATUS
    cbr temp1, 0xFC ; Keep only the MODE bits

    ldi ZH,HIGH(TKN_Transmitter_Modes)
    ldi ZL,LOW(TKN_Transmitter_Modes)
    add ZL, temp1
    brcc TKN_Transmitter_Switch_NoOverflow
    inc ZH
TKN_Transmitter_Switch_NoOverflow:
    ijmp
    
TKN_Transmitter_Modes:
    rjmp TKN_Transmitter_Activate   ;MODE 0
    rjmp TKN_Transmitter_DATA       ;MODE 1
    rjmp TKN_Transmitter_ACK        ;MODE 2
    rjmp TKN_Transmitter_TOKEN      ;MODE 3

TKN_Transmitter_Activate:
    and TKN_TX_PENDING, TKN_TX_PENDING
    brne TKN_Transmitter_FetchPacket

    ;Set TOKEN_MODE
    mov temp0, TKN_TX_STATUS
    sbr temp0, TKN_MODE_TOKEN 
    cbr temp0, (~$FC & ~TKN_MODE_TOKEN)
    mov TKN_TX_STATUS, temp0
    call setLeds0
    ret

TKN_Transmitter_FetchPacket:
    ldi XL, LOW(TKN_TX_BUFFER)
    ldi XH, HIGH(TKN_TX_BUFFER)
    
    ; BYTE#0 header
    ldi temp0, TKN_HEADER_BYTE
    st X+, temp0
    ; BYTE#1 type
    ldi temp0, TKN_TYPE_DATA
    st X+, temp0
    ; BYTE#2 receiver
    ldi YL, LOW(TKN_TX_QUEUE_ADDR)
    ldi YH, HIGH(TKN_TX_QUEUE_ADDR)
    add YL, TKN_TX_QUEUE_INDEX_OUT
    ld temp0, Y
    st X+, temp0
    ; BYTE# 3 sender
    st X+, TKN_ID
    ; BYTE#4 packet_id
    st X+, TKN_TX_PACKET_ID
    inc TKN_TX_PACKET_ID
    ; BYTE#5 check-byte
    clr temp0
    st X+, temp0
    ; BYTE#6..21
    lsl TKN_TX_QUEUE_INDEX_OUT
    lsl TKN_TX_QUEUE_INDEX_OUT
    lsl TKN_TX_QUEUE_INDEX_OUT
    lsl TKN_TX_QUEUE_INDEX_OUT
    ldi YL, LOW(TKN_TX_QUEUE)
    ldi YH, HIGH(TKN_TX_QUEUE)
    add YL, TKN_TX_QUEUE_INDEX_OUT
    lsr TKN_TX_QUEUE_INDEX_OUT
    lsr TKN_TX_QUEUE_INDEX_OUT
    lsr TKN_TX_QUEUE_INDEX_OUT
    lsr TKN_TX_QUEUE_INDEX_OUT
    ldi temp1, TKN_PACKET_SIZE
TKN_Push_fetchPacketLoop:
    ld temp0, Y+
    st X+, temp0
    dec temp1
    brne TKN_Push_fetchPacketLoop
    ; BYTE#22 EOF
    ldi temp0, TKN_EOF_BYTE
    st X+, temp0

    ;Set (DATA MODE)
    mov temp0, TKN_TX_STATUS
    sbr temp0, TKN_MODE_DATA
    cbr temp0, (~$FC & ~TKN_MODE_DATA)
    mov TKN_TX_STATUS, temp0
    call setLeds0
    ret

TKN_Transmitter_TOKEN:
    ldi ZL, LOW(TOKEN_PACKET<<1)
    ldi ZH, HIGH(TOKEN_PACKET<<1)
    clr temp0
    add ZL, TKN_TX_BYTE
    adc ZH, temp0
    
    lpm temp0, Z
    call USART_Transmit

    inc TKN_TX_BYTE
    mov temp2, TKN_TX_BYTE
    cpi temp2, TKN_OFFS_TOKEN_EOF + 1
    breq TKN_Transmitter_TOKEN_Passed
    ret

TKN_Transmitter_TOKEN_Passed:
    ; MODE=idle / TxActive=0 / Token=0
    clr TKN_TX_BYTE
    clr TKN_TX_STATUS
    mov temp0, TKN_TX_STATUS
    call setLeds0

    ;Disable DATA REGISTER READY INTERRUPT on the USART
    lds temp0, UCSR0B
    cbr temp0, (1<<UDRIE0)
    sts UCSR0B, temp0
    ret

TKN_Transmitter_DATA:
    ldi temp2, TKN_OFFS_DATA_EOF + 1
    cp temp2, TKN_TX_BYTE
    breq TKN_Transmitter_DATA_PacketSent

    ldi XL, LOW(TKN_TX_BUFFER)
    ldi XH, HIGH(TKN_TX_BUFFER)
    add XL, TKN_TX_BYTE
    ld temp0, X
    call USART_Transmit
    inc TKN_TX_BYTE
    ret

TKN_Transmitter_DATA_PacketSent:
    sbrs TKN_TX_STATUS, 4
    rjmp TKN_Transmitter_DATA_PacketSent_Normal

TKN_Transmitter_DATA_PacketSent_Forward:
    ; MODE=idle / TxActive=0 / Token=0
    clr TKN_TX_BYTE
    clr TKN_TX_STATUS
    mov temp0, TKN_TX_STATUS
    call setLeds0
    ret
    
TKN_Transmitter_DATA_PacketSent_Normal:
    ;Clear TX_ACTIVE / Set waiting for ACK
    mov temp0, TKN_TX_STATUS
    cbr temp0, (1<<6)
    sbr temp0, (1<<5)
    mov TKN_TX_STATUS, temp0
    call setLeds0
    ret

TKN_AckArrived:
    ;TKN_TX_QUEUE_INDEX_OUT = 
    ;   (TKN_TX_QUEUE_INDEX_OUT+PACKET_SIZE)%BUFFER_SIZE
    inc TKN_TX_QUEUE_INDEX_OUT
    mov temp0, TKN_TX_QUEUE_INDEX_OUT
    cbr temp0, TKN_QUEUE_LEN
    mov TKN_TX_QUEUE_INDEX_OUT, temp0

    clr TKN_TX_BYTE
    dec TKN_TX_PENDING

    ;Set TOKEN_MODE / TX_ACTIVE , Clear waiting for ACK
    mov temp0, TKN_TX_STATUS
    sbr temp0, TKN_MODE_TOKEN | (1<<6) 
    cbr temp0, (~$FC & ~TKN_MODE_TOKEN) | (1<<5)
    mov TKN_TX_STATUS, temp0
    call setLeds0
    ret

TKN_Transmitter_ACK:
    ldi temp2, TKN_OFFS_ACK_EOF + 1
    cp temp2, TKN_TX_BYTE
    breq TKN_Transmitter_ACK_PacketSent

    ldi XL, LOW(TKN_TX_BUFFER)
    ldi XH, HIGH(TKN_TX_BUFFER)
    add XL, TKN_TX_BYTE
    ld temp0, X
    call USART_Transmit
    inc TKN_TX_BYTE
    ret

TKN_Transmitter_ACK_PacketSent:
    ; MODE=idle / TxActive=0 / Token=0
    clr TKN_TX_BYTE
    clr TKN_TX_STATUS
    mov temp0, TKN_TX_STATUS
    call setLeds0
    ret


/*******************************************
 *** R E C E I V E R                     ***
 *******************************************/
 /*==============================================================
=== Routine: TKN_tokenArrived
===     1. Set the bit that indicates that we own the token.
===     2. Enable the interrupt that will wake up the transmitter
===        as long as this call returns.
================================================================*/
TKN_TokenArrived:
    ;if ( TokenOwner ) return
    sbrc TKN_TX_STATUS, 7
    ret

    ;Set TOKEN bit | TX_ACTIVE bit
    mov temp0, TKN_TX_STATUS
    sbr temp0, (1<<7) | (1<<6)
    mov TKN_TX_STATUS, temp0
    call setLeds0
    
    ;Enable DATA REGISTER READY INTERRUPT on the USART
    lds temp0, UCSR0B
    sbr temp0, (1<<UDRIE0)
    sts UCSR0B, temp0
    ret

/*==============================================================
=== Routine: TKN_Receiver
===		Receive a byte from temp0 register and handle it 
===		based on the state in which we are.
================================================================*/
TKN_Receiver:
    mov temp1, TKN_RX_STATUS
    cbr temp1, 0xFC ; Keep only the MODE bits

    ldi ZH,HIGH(TKN_Receiver_Modes)
    ldi ZL,LOW(TKN_Receiver_Modes)
    add ZL, temp1
    brcc TKN_Receiver_Switch_NoOverflow
    inc ZH
TKN_Receiver_Switch_NoOverflow:    
    ijmp
    
TKN_Receiver_Modes:
    rjmp TKN_Receiver_DetectPacket  ;MODE 0
    rjmp TKN_Receiver_DATA          ;MODE 1
    rjmp TKN_Receiver_ACK           ;MODE 2
    rjmp TKN_Receiver_TOKEN         ;MODE 3
    
TKN_Receiver_DetectPacket:
    sbrc TKN_RX_BYTE, 0
    rjmp TKN_Receiver_TypeByte

TKN_Receiver_HeaderByte:
    ldi temp1, TKN_HEADER_BYTE
    cpse temp0, temp1
    rjmp TKN_Receiver_HeaderByte_Incorrect

TKN_Receiver_HeaderByte_Correct:
    ldi XL, LOW(TKN_RX_BUFFER)
    ldi XH, HIGH(TKN_RX_BUFFER)
    st X, temp0
    inc TKN_RX_BYTE
    ret

TKN_Receiver_HeaderByte_Incorrect:
    ret

TKN_Receiver_TypeByte:
    cpi temp0, TKN_TYPE_DATA
    breq TKN_Receiver_TypeByte_DATA
    cpi temp0, TKN_TYPE_ACK
    breq TKN_Receiver_TypeByte_ACK
    cpi temp0, TKN_TYPE_TOKEN
    breq TKN_Receiver_TypeByte_TOKEN

    ;OK, the byte was not actually a valid type but 
    ;it may be the header.
    ldi temp1, TKN_HEADER_BYTE
    cpse temp0, temp1
    dec TKN_RX_BYTE
    ret

TKN_Receiver_TypeByte_DATA:
    ;Set DATA_MODE
    mov temp1, TKN_RX_STATUS
    sbr temp1, TKN_MODE_DATA
    cbr temp1, (~$FC & ~TKN_MODE_DATA)
    mov TKN_RX_STATUS, temp1
    rjmp TKN_Receiver_TypeByte_Valid

TKN_Receiver_TypeByte_ACK:
    ;Set  ACK_MODE
    mov temp1, TKN_RX_STATUS
    sbr temp1, TKN_MODE_ACK 
    cbr temp1, (~$FC & ~TKN_MODE_ACK)
    mov TKN_RX_STATUS, temp1
    rjmp TKN_Receiver_TypeByte_Valid

TKN_Receiver_TypeByte_TOKEN:
    ;Set TOKEN_MODE
    mov temp1, TKN_RX_STATUS
    sbr temp1, TKN_MODE_TOKEN 
    cbr temp1, (~$FC & ~TKN_MODE_TOKEN)
    mov TKN_RX_STATUS, temp1

TKN_Receiver_TypeByte_Valid:
    ldi YL, LOW(TKN_RX_BUFFER)
    ldi YH, HIGH(TKN_RX_BUFFER)
    std Y+1, temp0
    inc TKN_RX_BYTE
    ret

/** END OF DETECT_PACKET **/

TKN_Receiver_DATA:
    ;Store The DATA PACKET
    ldi YL, LOW(TKN_RX_BUFFER)
    ldi YH, HIGH(TKN_RX_BUFFER)
    add YL, TKN_RX_BYTE
    st Y, temp0
    inc TKN_RX_BYTE
    ldi temp0, TKN_OFFS_DATA_EOF + 1
    cpse temp0, TKN_RX_BYTE
    ret

TKN_Receiver_DATA_Received:
    ;For now only the mode bits are held in the RX_STATUS, 
    ;so its safe to clear it.
    clr TKN_RX_BYTE    
    clr TKN_RX_STATUS

    ldi YL, LOW(TKN_RX_BUFFER)
    ldd temp0, Y + TKN_OFFS_SENDER
    ;Compare the packet's sender with myself...
    cp temp0, TKN_ID 
    ;The packet was sent by ME... 
    ;So the actual receiver did NOT receive it.
    ;Discard the packet :(
    breq TKN_Receiver_DATA_FromMe
    ldd temp0, Y + TKN_OFFS_RECEIVER
    cp temp0, TKN_ID 
    ;The packet is for me!
    breq TKN_Receiver_DATA_ForMe

    ; The packet is not either FROM me either FOR me.
    ; So I have to forward it.
TKN_Receiver_DATA_Forward:
    ldi YL, LOW(TKN_RX_BUFFER)
    ldi YH, HIGH(TKN_RX_BUFFER)

    ldi XL, LOW(TKN_TX_BUFFER)
    ldi XH, HIGH(TKN_TX_BUFFER)

    ldi temp1, TKN_OFFS_DATA_EOF + 1
    
TKN_Receiver_DATA_Forward_cpy:
    ld temp0, Y+
    st X+, temp0
    dec temp1
    brne TKN_Receiver_DATA_Forward_cpy 

    clr TKN_TX_BYTE
    ;Set (DATA MODE / TX_Forward / TX_ACTIVE)
    mov temp0, TKN_TX_STATUS         ;This should become a routine call.
    sbr temp0, (1<<4)|(1<<6)         ;Set TX_ACTIVE, TX_Forward
    sbr temp0, TKN_MODE_DATA         ;Set the '1' bits of MODE
    cbr temp0, (~$FC & ~TKN_MODE_DATA);Set the '0' bits of MODE
    mov TKN_TX_STATUS, temp0
    call setLeds0

    ;Enable DATA REGISTER READY INTERRUPT on the USART
    lds temp0, UCSR0B
    sbr temp0, (1<<UDRIE0)
    sts UCSR0B, temp0
    ret

TKN_Receiver_DATA_FromMe:
    call TKN_AckArrived
    ret

TKN_Receiver_DATA_ForMe:
    ;Store tha packet in the RX_QUEUE
    ;If there is NOT an empty slot discard this packet.
    mov temp1, TKN_RX_PENDING
    cpi temp1, TKN_QUEUE_LEN
    in temp1, SREG
    sbrc temp1, SREG_Z
    rjmp TKN_Receiver_DATA_SendACK

    ldi YL, LOW(TKN_RX_BUFFER)
    ldi YH, HIGH(TKN_RX_BUFFER)

    ldi XL, LOW(TKN_RX_QUEUE_ADDR)
    ldi XH, HIGH(TKN_RX_QUEUE_ADDR)
    add XL, TKN_RX_QUEUE_INDEX_IN

    ;Store the Sender of the packet
    ldd temp0, Y + TKN_OFFS_SENDER
    st X, temp0 

    ;Perform lg(PACKET_SIZE) = lg(16) = [4 Left Shifts]
    lsl TKN_RX_QUEUE_INDEX_IN
    lsl TKN_RX_QUEUE_INDEX_IN
    lsl TKN_RX_QUEUE_INDEX_IN
    lsl TKN_RX_QUEUE_INDEX_IN

    ;Store the packet inside RX_BUFFER 
	;to the first available RX Queue slot
    ldi XL, LOW(TKN_RX_QUEUE)
    ldi XH, HIGH(TKN_RX_QUEUE)
    add XL, TKN_RX_QUEUE_INDEX_IN

    ldi temp1, TKN_OFFS_DATA_START
    add YL, temp1
    ldi temp1, TKN_PACKET_SIZE
    
TKN_StorePacket:
    ld temp0, Y+
    st X+, temp0
    dec temp1
    brne TKN_StorePacket

    ;Update TKN_TX_PENDING/POINTER_IN
    inc TKN_RX_PENDING

	mov temp0, TKN_RX_PENDING
	call setLeds

    lsr TKN_RX_QUEUE_INDEX_IN
    lsr TKN_RX_QUEUE_INDEX_IN
    lsr TKN_RX_QUEUE_INDEX_IN
    lsr TKN_RX_QUEUE_INDEX_IN

    ;TKN_RX_QUEUE_INDEX_IN = 
    ;   (TKN_RX_QUEUE_INDEX_IN+PACKET_SIZE)%BUFFER_SIZE
    inc TKN_RX_QUEUE_INDEX_IN
    mov temp0, TKN_RX_QUEUE_INDEX_IN
    cbr temp0, TKN_QUEUE_LEN
    mov TKN_RX_QUEUE_INDEX_IN, temp0

TKN_Receiver_DATA_SendACK:
    ;Create the ACK Packet
    ldi YL, LOW(TKN_RX_BUFFER)
    ldi YH, HIGH(TKN_RX_BUFFER)

    ldi XL, LOW(TKN_TX_BUFFER)
    ldi XH, HIGH(TKN_TX_BUFFER)

    ; BYTE#0 header
    ldi temp0, TKN_HEADER_BYTE
    st X+, temp0
    ; BYTE#1 type
    ldi temp0, TKN_TYPE_ACK
    st X+, temp0
    ; BYTE#2 receiver
    ldd temp0, Y + TKN_OFFS_SENDER
    st X+, temp0
    ; BYTE# 3 sender
    st X+, TKN_ID
    ; BYTE#4 packet_id
    ldd temp0, Y + TKN_OFFS_PACKET_ID
    st X+, temp0
    ; BYTE#5 EOF
    ldi temp0, TKN_EOF_BYTE
    st X+, temp0

    ;Here send tha ACK Packet
    clr TKN_TX_BYTE
    ;Set (ACK MODE / TX_ACTIVE)
    mov temp0, TKN_TX_STATUS    ;This should become a routine call.
    sbr temp0, (1<<6)           ;Set TX_ACTIVE
    sbr temp0, TKN_MODE_ACK     ;Set the '1' bits of MODE
    cbr temp0, (~$FC & ~TKN_MODE_ACK)   ;Set the '0' bits of MODE
    mov TKN_TX_STATUS, temp0
    call setLeds0

    ;Enable DATA REGISTER READY INTERRUPT on the USART
    lds temp0, UCSR0B
    sbr temp0, (1<<UDRIE0)
    sts UCSR0B, temp0
    ret

TKN_Receiver_TOKEN:
    ;No need to actually *store* the token
    inc TKN_RX_BYTE
    ldi temp0, TKN_OFFS_TOKEN_EOF + 1
    cpse temp0, TKN_RX_BYTE
    ret
TKN_Receiver_TOKEN_Received:
    ;For now only the mode bits are held in the RX_STATUS, 
    ;so its safe to clear it.
    clr TKN_RX_BYTE    
    clr TKN_RX_STATUS 
    call TKN_TokenArrived
    ret

TKN_Receiver_ACK:
    ;Store The ACK PACKET
    ldi YL, LOW(TKN_RX_BUFFER)
    ldi YH, HIGH(TKN_RX_BUFFER)
    add YL, TKN_RX_BYTE
    st Y, temp0
    inc TKN_RX_BYTE
    ldi temp0, TKN_OFFS_ACK_EOF + 1
    cpse temp0, TKN_RX_BYTE
    ret
TKN_Receiver_ACK_Received:
    clr TKN_RX_BYTE    
    clr TKN_RX_STATUS
    ldi YL, LOW(TKN_RX_BUFFER)
    ldd temp0, Y + TKN_OFFS_SENDER
    ;Compare the packet's sender with myself.
    cp temp0, TKN_ID 
    ;The packet was sent by ME.
    ;So the actual receiver did NOT receive it.
    ;Discard the packet :(
    breq TKN_Receiver_ACK_FromMe
    ldd temp0, Y + TKN_OFFS_RECEIVER
    cp temp0, TKN_ID 
    ;The packet is for me!
    breq TKN_Receiver_ACK_ForMe

TKN_Receiver_ACK_Forward:
    ; The ACK is not either FROM me either FOR me.
    ; So I have to forward it.
    ldi YL, LOW(TKN_RX_BUFFER)
    ldi YH, HIGH(TKN_RX_BUFFER)

    ldi XL, LOW(TKN_TX_BUFFER)
    ldi XH, HIGH(TKN_TX_BUFFER)

    ldi temp1, TKN_OFFS_ACK_EOF + 1

TKN_Receiver_ACK_Forward_cpy:
    ld temp0, Y+
    st X+, temp0
    dec temp1
    brne TKN_Receiver_ACK_Forward_cpy

    clr TKN_TX_BYTE
    ;Set (ACK MODE / TX_Forward / TX_ACTIVE)
    mov temp0, TKN_TX_STATUS    ;This should become a routine call.
    sbr temp0, (1<<4)|(1<<6)    ;Set TX_ACTIVE, TX_Forward
    sbr temp0, TKN_MODE_ACK     ;Set the '1' bits of MODE
    cbr temp0, (~$FC & ~TKN_MODE_ACK)   ;Set the '0' bits of MODE
    mov TKN_TX_STATUS, temp0
    call setLeds0

    ;Enable DATA REGISTER READY INTERRUPT on the USART
    lds temp0, UCSR0B
    sbr temp0, (1<<UDRIE0)
    sts UCSR0B, temp0
    ret
TKN_Receiver_ACK_FromMe:
    ret
TKN_Receiver_ACK_ForMe:
    call TKN_AckArrived
    ret
