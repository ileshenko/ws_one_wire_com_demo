/*
 * periodic send one byte data over UART:
 * period - once a second
 * DATA:
 *  0x0 <1 sec> [value] <1 sec>
 * main.c
 */
#include <msp430g2553.h>
#include <string.h>
#include "../common/common.h"

char *to_print;
unsigned int i, uart_len;

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
    TA0CCR0 = 0xF424; /* 125 kHz / 2 */

    jiffies = 0;
}

void buttons_init(void)
{
	P1SEL &= ~BTN;									// switch to GPIO mode
	P1SEL2 &= ~BTN;									// switch to GPIO mode

	P1DIR &= ~BTN;									// Set as Input
	P1REN |= BTN;									// Poll Up/Down Resistor enable
	P1OUT |= BTN;									// Poll Up
	P1IE |= BTN;									// Interrupt Enabled
	P1IES |= BTN;									// Hi/Lo edge
	P1IFG &= ~BTN;									// IFG cleared
}


void uart_data(char *data, int len)
{
	to_print = data;
	i = 0;
	uart_len = len;
	IE2 |= UCA0TXIE;
}

void uart_text(char *line)
{
	int len = strlen(line);
	uart_data(line, len);
}

//static char hello[] = "Hello World!\n\r";
static char data[3] = {MAGIC_BEGIN, 0, MAGIC_END};
static char stat;

void main(void)
{
	WDTCTL = WDTPW + WDTHOLD;                 // Stop WDT

	default_state();
	clock_init();
	timer_init();
	uart_init();
	leds_init();
	buttons_init();

	for (;;)
	{
//		uart_text(hello);
		data[1] = stat;
		uart_data(data, 3);

		if (stat & 1)
			P1OUT |= LED1;
		else
			P1OUT &= ~LED1;

		if (stat & 2)
			P1OUT |= LED2;
		else
			P1OUT &= ~LED2;

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

#pragma vector = PORT1_VECTOR
__interrupt void port1_isr (void)
{
	if (P1IFG & BTN)
	{
		stat++;
		P1IFG &= ~BTN;
	}
	else
		for (;;);
}

#pragma vector = USCIAB0TX_VECTOR
__interrupt void USCIAB0TX_ISR (void)
{
	UCA0TXBUF = to_print[i++];				// TX next character

	if (i == uart_len)
		IE2 &= ~UCA0TXIE;                       // Disable USCI_A0 TX interrupt
}

#pragma vector = USCIAB0RX_VECTOR
__interrupt void uscib0rx_Stub (void)
{
	for (;;);
}

