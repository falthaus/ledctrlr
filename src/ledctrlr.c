////////////////////////////////////////////////////////////////////////////////
//
//	ledctrl.c
//
//
//	(C) 2025, Felix Althaus
//
//
////////////////////////////////////////////////////////////////////////////////
//					  __ __
//		      RESET -|  -  |- VCC
//	Jumper		PB3 -|     |- PB2	TXD (software tx-only UART)
//	Jumper		PB4 -|     |- PB1	Output (OCR1A)
//				GND -|_____|- PB0	RC Input (PCINT0)
//
//
////////////////////////////////////////////////////////////////////////////////


#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include <stdlib.h>
#include <stdbool.h>


////////////////////////////////////////////////////////////////////////////////




#define INPUT_LOW		(1100)
#define INPUT_MID		(1520)
#define INPUT_HI		(1940)
#define INPUT_TOL		(105)


#define VOUT_LOW_MV		(500)
#define VOUT_MID_MV		(1250)
#define VOUT_HI_MV		(2500)
#define VSUPPLY_MV		(3300)

#define VOUT_DEFAULT_MV	(0)


#define USE_64MHZ


////////////////////////////////////////////////////////////////////////////////

// I/O Configuration
//						// I/O		Weak pull-ups
#define RCIN 	(PB0)	// I		no
#define OUT 	(PB1)	// O		no
#define TXD 	(PB2)	// O		no
#define CFG0 	(PB3)	// I		yes
#define CFG1 	(PB4)	// I		yes


////////////////////////////////////////////////////////////////////////////////


void uart_init(void);
void uart_transmit(uint8_t);
void uart_print(char*);


////////////////////////////////////////////////////////////////////////////////


volatile uint8_t t1_lo;
volatile uint8_t t2_lo;

volatile uint8_t t1_hi;
volatile uint8_t t2_hi;

volatile uint8_t tcnt0_hi;

// PB3 and PB4 are configured with internal pull-up resistors and are
// connected to a jumper each:
// Jumper present: pin low
// Jumper absent:  pin high

enum mode_t { NORMAL = (1<<PIN3) | (1<<PIN4),	// no jumper
			  TEST1  = (1<<PIN3),				// jumper on PB4 only
			  TEST2  = (1<<PIN4),				// jumper on PB3 only
			  TEST3  = 0 };						// both jumpers


////////////////////////////////////////////////////////////////////////////////


int main(void)
{

	uint16_t t1;
	uint16_t t2;
	int16_t rcin;

	char buf[16];
	char output_state;

	enum mode_t mode;

// FIXME: check OSCCAL value, place define in .platformio file
	OSCCAL = 0x4E;	    // manually calibrated by hand for 3.3V and ambient
						// (factory calibration done for 3.0V and 25°C)

// FIXME: configure unused as inputs and with pull-up
	PORTB = (1<<TXD)|(1<<CFG0)|(1<<CFG1);	// TXD idle level is logic high, all others low
											// CFG0 and CFG1 with internal pull-ups

												// DDR: 0=input, 1=output
	DDRB = ~((1<<RCIN)|(1<<CFG0)|(1<<CFG1));	// RCIN, CFG0 CFG1 are inputs, all others output


	uart_init();


	PCMSK |= (1<<PCINT0);
	GIMSK |= (1<<PCIE);

	// TODO: go with 8MHz timer clock
	TCCR0B = (1<<CS01);	// 2022-02-09: with (1<<CS00) the UART gets messed up (to many ISRs...)
	TIMSK  = (1<<TOIE0);


	// TCCR0A = (1<<COM0B1) | (1<<WGM01) | (1<<WGM00);
	// TCCR0B = (1<<CS00);
	// OCR0B = 64;



    #ifdef USE_8MHZ

        // f_pwm = 40 kHz
        // Resolution: 200 steps
        // T_pwm = 25 us
        // 40 cycles / 1ms
        // 400 cycles per 10 ms
        //
        OCR1C = 200-1;
        TCCR1 = (1<<PWM1A) | (1<<COM1A1) | (1<<CS10);
        OCR1A = duty_cycle;
        OCR1A = 100;

    #endif


    #ifdef USE_64MHZ

        //PLLCSR |= (1<<LSM);       // Low-speed mode
        PLLCSR |= (1<<PLLE);        // enable PLL
        _delay_us(100);             // wait for PLL steady-state
                                    // (PLOCK should be ignored during PLL lock-in)
        while( !(PLLCSR & (1<<PLOCK)) );  // wait for PLL to lock
        PLLCSR|= (1<<PCKE);         // enable asynchronous clock

        TCCR1 = (1<<PWM1A) | (1<<COM1A1) | (1<<CS10);   // PCK/1
        OCR1C = 0xFF;               // 64 MHz / 1 / 256 = 250 kHz @ 8bit resolution
        OCR1A = OCR1A = (255L*VOUT_DEFAULT_MV + (VSUPPLY_MV/2))/VSUPPLY_MV;

    #endif


	// check jumper configuration, but only at startup
	mode = PINB & ((1<<PIN3) | (1<<PIN4));

	GPIOR0 = 0x00;
	sei();



	while(true)
	{

		if(GPIOR0 & 0x01)
		{
			cli();

			t1 = 256*t1_hi + t1_lo;
			t2 = 256*t2_hi + t2_lo;

			rcin = t2 - t1;


			if((rcin > INPUT_LOW-INPUT_TOL) && (rcin < INPUT_LOW+INPUT_TOL))
			{
				OCR1A = (255L*VOUT_LOW_MV + (VSUPPLY_MV/2))/VSUPPLY_MV;
				output_state = '0';
			}
			else if((rcin > INPUT_MID-INPUT_TOL) && (rcin < INPUT_MID+INPUT_TOL))
			{
				OCR1A = (255L*VOUT_MID_MV + (VSUPPLY_MV/2))/VSUPPLY_MV;
				output_state = '1';
			}
			else if((rcin > INPUT_HI-INPUT_TOL) && (rcin < INPUT_HI+INPUT_TOL))
			{
				OCR1A = (255L*VOUT_HI_MV + (VSUPPLY_MV/2))/VSUPPLY_MV;
				output_state = '2';

			}
			else
			{
				output_state = '-';
			}


			// Time for UART transmissions is limited
			// RCIN period is about 14ms (e.g. for Futaba RX)
			// Time from falling egde to next rising is thus ca. 12ms
			// At 19200 baud, 1 character (10 bits) is 521 us
			// => there is time for max. 23 characters

			uart_transmit(mode + '0');
			uart_transmit('\t');
			uart_print(utoa(rcin, buf, 10));
			uart_transmit('\t');
			uart_transmit(output_state);
			uart_print("\r\n");

			// Re-enable interrupts after UART transmissions are done
			GPIOR0 &= ~(0x01);
			sei();

		} // end if

	} // end while

} // end main


////////////////////////////////////////////////////////////////////////////////


void uart_init(void)
{
	;
}


////////////////////////////////////////////////////////////////////////////////


#define BIT_DELAY	(1e6/BAUD)

void uart_transmit(uint8_t c)
{
	uint8_t bp;

	// idle (high)

	// START bit (low)
	PORTB &= ~(1<<TXD);
	_delay_us(BIT_DELAY);

	for(bp=1; bp; bp=bp<<1)				// LSB first
	{
		if(c & bp)
			PORTB |= (1<<TXD);
		else
			PORTB &= ~(1<<TXD);
		_delay_us(BIT_DELAY);			// TODO: account for loop overhead!!!
	}

	// STOP bit (high)
	PORTB |= (1<<TXD);
	_delay_us(BIT_DELAY);

	// idle (high)
}


////////////////////////////////////////////////////////////////////////////////


void uart_print(char* s)
{
	while(*s)
	{
		uart_transmit(*s);
		s++;
	}
}


////////////////////////////////////////////////////////////////////////////////


ISR(PCINT0_vect)
{
	uint8_t tcnt0_lo_tmp;
	uint8_t tcnt0_hi_tmp;

	// TODO: see https://www.nongnu.org/avr-libc/examples/asmdemo/isrs.S
	//
	// from Microchip ATtinyX5 datasheet:
	// "When using the SEI instruction to enable interrupts, the instruction following SEI
	// will be executed before any pending interrupts..."

	sei();
	tcnt0_lo_tmp = TCNT0;
	cli();
	tcnt0_hi_tmp = tcnt0_hi;


	if(PINB & (1<<RCIN))
	{
		// Rising Edge (2nd edge)

		t2_lo = tcnt0_lo_tmp;
		t2_hi = tcnt0_hi_tmp;

		GPIOR0 |= (1<<0);	// signal to main loop that a full pulse has been acquired
	}
	else
	{
		// Falling Edge (1st edge)

		t1_lo = tcnt0_lo_tmp;
		t1_hi = tcnt0_hi_tmp;

		GPIOR0 &= ~(1<<0);	// pulse acquisition ongoing
	}

}


////////////////////////////////////////////////////////////////////////////////


ISR(TIM0_OVF_vect)
{
	tcnt0_hi++;
}


////////////////////////////////////////////////////////////////////////////////
