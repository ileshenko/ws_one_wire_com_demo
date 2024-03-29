#include "common.h"
#include <msp430g2553.h>

void default_state(void)
{
	P1DIR = 0xff;
	P2DIR = 0xff;
}

void clock_init(void)
{
	/* Set DCO to 1 MHz */
	DCOCTL = CALDCO_1MHZ;				//calibrate DCO
	BCSCTL1 = CALBC1_1MHZ;				//calibrate basic clock

	/* MCLK - 1MHz (MCLK = DCO/1)*/
	/* SMCLK - 1MHz (SMCLK = DCO/1)*/
	BCSCTL2 = SELM_0 | DIVM_0 | /*SELS |*/ DIVS_0; // .
}

void leds_init(void)
{
	P1DIR |= LED1 | LED2;
	P1REN &= ~(LED1 | LED2);
	P1OUT |= LED1;
	P1OUT &= ~LED2;
}

void uart_init(void)
{
	UCA0CTL1 |= UCSWRST; //toggle SW Reset on

	/* Port 1.2 (Rx) may be overridden by ADC if we need output only */
	P1SEL |= BIT1 | BIT2; //select RXD and TXD for UART
	P1SEL2 |= BIT1 | BIT2; //select RXD and TXD for UART

	/* UCSYNC = 0 (ASYNC),
	 * UCMODE = 0 (UART),
	 * UCSPB = 0 (one stop),
	 * UC7BIT = 0 (8 bit),
	 * UCMSB = 0 (LSB first),
	 * UCPEN = 0 (No Parity) */
	UCA0CTL0 = 0;

	/* input - SMCLK (1MHz) */
	UCA0CTL1 |= UCSSEL_3; /* SMCLK (0x3)*/

#if USB_BAUDRATE == 300
	/* Baud Rate Control */
	UCA0BR0 = 208;
	UCA0BR1 = 0;
	/* Modulation Control */
	UCA0MCTL = UCBRF_5 | UCBRS_0 | UCOS16;


#elif USB_BAUDRATE == 9600
	/* Baud Rate Control */
	UCA0BR0 = 6;
	UCA0BR1 = 0;
	/* Modulation Control */
	UCA0MCTL = UCBRF_8 | UCBRS_0 | UCOS16;
#else
#error Unknown Boudrate
#endif

	UCA0CTL1 &= ~UCSWRST; //toggle SW Reset off
}
