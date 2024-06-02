/*
 * adc_read.h
 *
 *  Created on: Jun 2, 2024
 *      Author: andy
 */

#ifndef ADC_READ_H_
#define ADC_READ_H_

#include <stdbool.h>
#include <SI_EFM8UB2_Register_Enums.h>

/*
 * Return the averaged ADC value.
 */
uint16_t ADC_get_value(void);

/*
 * return true when the average is complete.
 * Calling this clears the flag.
 */
bool ADC_is_new_avg(void);

#endif /* ADC_READ_H_ */
