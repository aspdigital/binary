/*
 * binary,
 *
 * a design to read eight pushbutton switches and interpret the presses as a
 * byte-wide binary number.
 */

#include <SI_EFM8UB2_Register_Enums.h>                  // SFR declarations
#include "config.h"
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "uart.h"
#include "adc_read.h"

//-----------------------------------------------------------------------------
// SiLabs_Startup() Routine
// ----------------------------------------------------------------------------
// This function is called immediately after reset, before the initialization
// code is run in SILABS_STARTUP.A51 (which runs before main() ). This is a
// useful place to disable the watchdog timer, which is enable by default
// and may trigger before main() in some instances.
//-----------------------------------------------------------------------------
void SiLabs_Startup(void) {
	// $[SiLabs Startup]
	// [SiLabs Startup]$
}

/*
 * Keep track of the number of times t3 has overflowed. 20 times is 200 ms.
 */
static volatile uint8_t t3_of_cnt;

/*
 * How many ticks of our 10 ms timer do we wait for stability?
 * Possibly override by compiler settings.
 */
#ifndef STABLE_TIME
#define STABLE_TIME 250
#endif

/*
 * Indicate the hold time interval has ended.
 */
static volatile bool waited_stable_time;

/*
 * Any ADC reading above this value means we've pressed something.
 */
#define PRESSED_VALUE 3

/*
 * Build in some tolerance for ADC readings, as our collection of resistors
 * and wiring may be noisy. A capacitor filter and op-amp buffer may mitigate
 * this noise. Tolerance is specified in ADU.
 */
#define TOLERANCE 5

/*
 * State machine state values.
 */
typedef enum _state_t
{
	S_IDLE,					// wait for any button presses
	S_GOT_A_PRESS,			// buttons were pressed, wait for stabilization
	S_WAIT_FOR_RELEASE		// we captured the pressed state, now wait for release
} state_t;

/*
 * LEDs for status.
 * Blue means "we detected press but it's not stable."
 * Green means "got valid press."
 * Red means "released, waiting for return to zero."
 */
sbit LED_R = P2^0;
sbit LED_G = P1^6;
sbit LED_B = P1^7;

#define LED_OFF 1
#define LED_ON  0

/*
 * ISR for Timer 3.
 *
 * Timer 3 sets the interval for testing "stable" button presses. That is:
 *
 * "Idle" means no presses, and the ADC reads a low (if not zero) value.
 * Anything else means a press.
 * When a press is detected, this timer is started and the ADC value (the
 * button press state) is saved.. When the timer expires, we see if the most
 * recent press matches the saved, and if so we declare the pressed value is
 * correct. If new reading does not match previous, we don't have our value
 * yet, so save the current value and try again.
 *
 * The button presses have to be stable for STABLE_TIME, which has a maximum
 * of 255 or 2550 ms.
 *
 * Timer 3 is clocked by SYSCLK / 12 or 4 MHz so its maximum count time is
 * 16.384 ms. We need to count multiple overflows of this timer to get to our
 * defined stable time.
 */
SI_INTERRUPT_USING(T3ISR, TIMER3_IRQn, 3)
{
	/* first we must clear the interrupt condition. */
	TMR3CN0 &= ~TMR3CN0_TF3H__SET;

	/* if we've interrupted STABLE_TIME times, we have waited our debounce time
	 * stop the timer, tell the world, prep for next time */
	if (t3_of_cnt == STABLE_TIME)
	{
		waited_stable_time = true;
		TMR3CN0 &= ~TMR3CN0_TR3__RUN;
		EIE1 &= ~EIE1_ET3__ENABLED;
		t3_of_cnt = 0;
	} else {
		++t3_of_cnt;
	}
}

/*
 * Main!
 */
void main(void)
{
	unsigned char rxstr[FDEPTH];		/* message buffer */
	uint16_t prev_adc_val;				/* last ADC reading, for comparison */
	uint16_t this_adc_val;				/* capture the most recent value, as it is volatile */
	state_t state;						/* state machine state register */
	unsigned char ascii_char;			/* ASCII character representing the button press */

	/* Configure our peripherals. */
	Config();

	/* Useful for debug */
	memset(rxstr, 0x00, FDEPTH);

	prev_adc_val = 0;
	state = S_IDLE;
	t3_of_cnt = 0;
	waited_stable_time = false;
	LED_R = LED_OFF;
	LED_G = LED_OFF;
	LED_B = LED_OFF;

	/* prepare serial terminal */
	UartInitFifos();

	/* Starts the timer that runs the UART. */
	TCON |= TCON_TR1__RUN;

	sprintf(rxstr, "Binary to ASCII test!\n\r");
	UartWriteStringToTxFifo(rxstr);

	/* Enable conversion. */
	ADC0CN0 |= ADC0CN0_ADEN__ENABLED ;

	/*
	 * Loop forever.
	 */
	while (1) {

		/* Don't do anything until we have a new ADC value. */

		if( ADC_is_new_avg() )
		{
			this_adc_val = ADC_get_value();

			switch (state) {
			case S_GOT_A_PRESS :
				/*
				 * We detected a button press.
				 * If the buttons change, then start again.
				 * If they are the same within the specified TOLERANCE,
				 * see if we've waited the full debounce time.
				 */
				if( (prev_adc_val > (this_adc_val + TOLERANCE)) ||
					(prev_adc_val < (this_adc_val - TOLERANCE)) )
				{
					prev_adc_val = this_adc_val;

					/* restart timer */
					TMR3CN0 &= ~TMR3CN0_TR3__RUN;
					TMR3 = 0x0000;
					t3_of_cnt = 0;
					TMR3CN0 |= TMR3CN0_TR3__RUN;

				} else {

					if( waited_stable_time )
					{
						/* A match! */
						LED_B = LED_OFF;
						LED_G = LED_ON;
						ascii_char = (unsigned char) (this_adc_val >> 2);
						sprintf(rxstr, "%u = %c\n\r", this_adc_val, ascii_char);
						UartWriteStringToTxFifo(rxstr);
						/*
						 * Now wait for all buttons to be released before we
						 * look for the next press.
						 * The ISR has stopped the timer.
						 */
						state = S_WAIT_FOR_RELEASE;
					}
				}
				break;

			case S_WAIT_FOR_RELEASE :
				/*
				 * A button press combination was captured.
				 * Now we are waiting for all to be released.
				 */
				if( this_adc_val <= PRESSED_VALUE )
				{
					LED_G = LED_OFF;
					state = S_IDLE;
				}
				break;

			default : /* includes S_IDLE */
				/*
				 * Check for new button press.
				 * If it was pressed, we'll save the current ADC value for the
				 * comparison. We can save it before as it doesn't really add
				 * a time penalty for doing so.
				 */
				prev_adc_val = this_adc_val;

				if( prev_adc_val > PRESSED_VALUE )
				{
					/*
					 * Yep, a press, prepare for comparison after waiting the
					 * stabilization time.
					 */
					state = S_GOT_A_PRESS;
					LED_B = LED_ON;
					/* start the hold timer */
					waited_stable_time = false;
					t3_of_cnt = 0;
					EIE1 |= EIE1_ET3__ENABLED;
					TMR3CN0 |= TMR3CN0_TR3__RUN;
				}
				break;

			} // switch
		} // got a new ADC reading
	} // forever
}
