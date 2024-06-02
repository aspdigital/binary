/*
 * stable_timer.c
 *
 *  Created on: Jun 2, 2024
 *      Author: andy
 *
 * Counts the time we must keep buttons pressed before we declare validity.
 */

#include "stable_timer.h"

/*
 * Keep track of the number of times t3 has overflowed. 20 times is 200 ms.
 */
static volatile uint8_t t3_of_cnt;

/*
 * Indicate the hold time interval has ended.
 */
static volatile bool waited_stable_time = false;


/*
 * How many ticks of our 10 ms timer do we wait for stability?
 * Possibly override by compiler settings.
 */
#ifndef STABLE_TIME
#define STABLE_TIME 250
#endif

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
 * Return true when we've waited the defined stable time.
 * the flag is cleared on this read.
 */
bool STABLE_TIMER_is_stable(void)
{
	bool yorn;

	yorn = waited_stable_time;
	waited_stable_time = false;
	return yorn;
}

/*
 * start or reset the stable time timer.
 */
void STABLE_TIMER_start(void)
{
	TMR3CN0 &= ~TMR3CN0_TR3__RUN;
	TMR3 = 0x0000;
	t3_of_cnt = 0;
	EIE1 |= EIE1_ET3__ENABLED;
	TMR3CN0 |= TMR3CN0_TR3__RUN;
}



