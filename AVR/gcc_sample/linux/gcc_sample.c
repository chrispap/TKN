#include <avr/io.h>
#include <avr/interrupt.h>
#include <string.h>
#include "gcc_sample.h"

char TKN_Send(char *buf, char dest_id)
{
    asm ("push r16");
    asm ("push r17");
    asm ("push r18");
    asm ("push r28");
    asm ("push r29");

    asm ("mov r28, r24");
    asm ("mov r29, r25");
    asm ("mov r16, r22");
    ((void(*)()) TKN_PUSH_ADDR)();
    asm ("mov  r24, r16");

    asm ("pop r29");
    asm ("pop r28");
    asm ("pop r18");
    asm ("pop r17");
    asm ("pop r16");
}

char TKN_Receive(char *buf)
{
    asm ("push r16");
    asm ("push r17");
    asm ("push r18");
    asm ("push r28");
    asm ("push r29");

    asm ("mov r28, r24");
    asm ("mov r29, r25");
    ((void(*)()) TKN_POP_ADDR)();
    asm ("mov  r24, r16");

    asm ("pop r29");
    asm ("pop r28");
    asm ("pop r18");
    asm ("pop r17");
    asm ("pop r16");
}

ISR(BADISR_vect)
{
    TKN_Send("BAD_INTERRUPT!!!", 1);
}


#define TICKS_PER_PHASE 32
volatile uint8_t ticks;
uint8_t ON=0x00, OFF=0xFF;
char *msg = "From Gcc app.";
char packet_buf[16];

ISR(TIMER0_OVF_vect)
{
   ++ticks;
}

int main(void)
{
    /* Init timer */
    TCCR0A = 0;
    TCCR0B = 5<<CS00;
    TIMSK0 = (1<<TOIE0);
    TCNT0  = 0;
    ticks  = 0;

    PORTB = 0xFF;
    sei();

    uint8_t phase, counter=0;
    do
    {
        phase = ticks % TICKS_PER_PHASE;

        if (phase==0) {
            PORTB=ON;
            packet_buf[0] = ++counter;
            TKN_Send(packet_buf, 1);
            TKN_Send(packet_buf, 3);
            TKN_Send(packet_buf, 4);
        }
        else if (phase<4) {
            PORTB = phase%2? OFF: ON;
        }

        uint8_t t = ticks; while (t==ticks); // Wait this phase to end.
    } while (!TKN_Receive( packet_buf));

    /* Stop timer */
    TCCR0A = 0;
    TCCR0B = 0;

    /*Return to TKN_BOOT */
    ((void(*)()) TKN_RETURN_ADDR)();
}
