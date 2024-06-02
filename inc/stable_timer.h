/*
 * stable_timer.h
 *
 *  Created on: Jun 2, 2024
 *      Author: andy
 */

#ifndef INC_STABLE_TIMER_H_
#define INC_STABLE_TIMER_H_

#include <stdbool.h>
#include <SI_EFM8UB2_Register_Enums.h>

/*
 * Return true when we've waited the defined stable time.
 * the flag is cleared on this read.
 */
bool STABLE_TIMER_is_stable(void);

/*
 * start or reset the stable time timer.
 */
void STABLE_TIMER_start(void);

#endif /* INC_STABLE_TIMER_H_ */
