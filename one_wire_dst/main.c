/*
 * main.c
 */
#include <msp430g2553.h>
#include "../common/common.h"

unsigned long jiffies;

void timer_init(void)
{
	/* Timer A */
	/* Source SMCLK (1MHz) TASSEL_2
	 * divider 8 (125 kHz) ID_3
	 * Mode UP/Down MC_3
	 * Interrupt enable TAIE
	 */
    TA0CTL = TASSEL_2 | ID_3 | MC_3 | TAIE;
    TA0CCR0 = 0x3D09; /* 125 kHz / 2 / 4 (1/4 second)*/

    jiffies = 0;
}

static void show_error(void)
{
	P1OUT ^= LED1;
	if (P1OUT & LED1)
		P1OUT &= ~LED2;
	else
		P1OUT |= LED2;
}

unsigned char stat;
unsigned char timeout;
int i;

static void show_data(void)
{
	if (stat & 1)
		P1OUT |= LED1;
	else
		P1OUT &= ~LED1;

	if (stat & 2)
		P1OUT |= LED2;
	else
		P1OUT &= ~LED2;
}

void main(void)
{
	WDTCTL = WDTPW + WDTHOLD;                 // Stop WDT

	i = -1;
	default_state();
	clock_init();
	timer_init();
	uart_init();
	leds_init();

	IE2 |= UCA0RXIE;

	for (;;)
	{
		if (timeout >= 10)
			show_error();
		else
		{
			timeout++;
			show_data();
		}

		_BIS_SR(LPM0_bits + GIE);
	}
}

#pragma vector=TIMER0_A1_VECTOR
__interrupt void main_timer(void)
{
	switch(TA0IV)
	{
	case 10: /* TAIFG, timer overflow */
		break;
	case 2: /* TACCR1 CCIFG, compare 1 */
	case 4: /* TACCR2 CCIFG, compare 2 */
	default: /* impossible! */
		for (;;);
	}

	jiffies++;

	_BIC_SR_IRQ(LPM0_bits);
}


#pragma vector = USCIAB0RX_VECTOR
__interrupt void uscib0rx_isr (void)
{
	unsigned char byte;

	byte = UCA0RXBUF;
	if (i == -1)
	{
		if (byte == MAGIC_BEGIN)
			i++;
	}
	else if (i == DATA_LEN)
	{
		if (byte == MAGIC_END)
		{
			timeout = 0;
			i = -1;
		}
	}
	else
	{
		i++;
		stat = byte;
	}
}

#pragma vector = PORT1_VECTOR
__interrupt void port1_Stub (void)
{
	for (;;);
}

#pragma vector = USCIAB0TX_VECTOR
__interrupt void USCIAB0TX_Stub (void)
{

}
