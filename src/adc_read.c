/*
 * adc_read.c
 *
 *  Created on: Jun 2, 2024
 *      Author: andy
 *
 * Read the ADC and average the readings.
 *
 * Set a flag when 512 readings have been averaged.
 */
#include "adc_read.h"

static volatile uint16_t adcval;			// averaged ADC value
static volatile bool got_new_adc_val;		// true when that value is updated

/*
 * ADC interrupt service routine.
 *
 * Average 512 samples into the final result. The accumulator is 19 bits.
 */
SI_INTERRUPT_USING(ADC_ISR, ADC0EOC_IRQn, 2)
{
	static uint32_t adcacc = 0;
	static uint16_t avgcnt = 0;

	/* Clear the interrupt flag, or else we go nowhere. */
	ADC0CN0_ADINT = 0;

	/* update the accumulator for the average.
	 * When we have enough, update the average and tell the world it's ready. */
	adcacc += ADC0;

	if( avgcnt == 512 )
	{
		avgcnt = 0;
		adcval = (uint16_t) (adcacc >> 9);
		adcacc = 0;
		got_new_adc_val = true;
	} else {
		++avgcnt;
	}
}

/*
 * Return the averaged ADC value.
 */
uint16_t ADC_get_value(void)
{
	return adcval;
}

/*
 * return true when the average is complete.
 * Calling this clears the flag.
 */
bool ADC_is_new_avg(void)
{
	bool yorn;

	yorn = got_new_adc_val;
	got_new_adc_val = false;
	return (yorn);
}
