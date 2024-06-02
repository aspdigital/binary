/*
 * config.c
 *
 *  Created on: May 25, 2024
 *      Author: andy
 *
 * Configure the board peripherals.
 */

#include <SI_EFM8UB2_Register_Enums.h>

void Config(void)
{
	uint8_t save;
	/*
	 - Disable Watchdog Timer
	 - System clock divided by 12
	 - PCA continues to function normally while the system controller is in
	       Idle Mode
	 - Disable the CF interrupt
	 - Disable Watchdog Timer
	 - Watchdog Timer Enable unlocked
	*/
	SFRPAGE = 0x00;
	PCA0MD &= ~PCA0MD_WDTE__BMASK;
	PCA0MD = PCA0MD_CPS__SYSCLK_DIV_12 | PCA0MD_CIDL__NORMAL | PCA0MD_ECF__OVF_INT_DISABLED
		 | PCA0MD_WDTE__DISABLED | PCA0MD_WDLCK__UNLOCKED;

	// disable the PCA timer. We are not using the PCA.
	PCA0CN0_CR = PCA0CN0_CR__STOP;

	/*
	 - Each byte of a firmware flash write is written individually
	 - Enable the prefetch engine
	 - set flash read timing for 48 MHz clocking and enable the flash read one-shot
	 - (which is recommended for reduced power consumption.
	 */
	PFE0CN = PFE0CN_FLBWE__BLOCK_WRITE_DISABLED | PFE0CN_PFEN__ENABLED;
	FLSCL = FLSCL_FOSE__ENABLED | FLSCL_FLRT__SYSCLK_BELOW_48_MHZ;

	/*
	 - Clock.
	 -- Enable HF oscillator and set SYSCLK to it (divide by 1)
	 - USB clock source is HFOSC (48 MHz) for full speed USB
	 - SYSCLK is 48 MHz too.
	 */
	HFO0CN = HFO0CN_IFCN__SYSCLK_DIV_1 | HFO0CN_IOSCEN__ENABLED;
	CLKSEL = CLKSEL_CLKSL__HFOSC | CLKSEL_USBCLK__HFOSC
			| CLKSEL_OUTCLK__SYSCLK_SYNC_IO;

	/*
	 * Set up voltage reference.
	 - Disable the internal Temperature Sensor
	 - Disable the internal reference buffer
	 - The on-chip voltage reference buffer gain is 2
	 - The REFSL bit selects the voltage reference source
	 - Use VDD as the voltage reference
	*/
	REF0CN = REF0CN_TEMPE__DISABLED | REF0CN_REFBE__DISABLED | REF0CN_REFBGS__GAIN_2
		 | REF0CN_REGOVR__REFSL | REF0CN_REFSL__VDD;

	/*
	 * Set up I/O ports. The eval board use is noted.
	 *
	 * Port 0:
	 *
	 * P0.0: VCOM EN  (output push-pull), must drive high. (skipped)
	 * P0.1:          (input open-drain) (DISP_CS)
	 * P0.2: PB0      (input open-drain) SKIP
	 * P0.3: PB1      (input open-drain) SKIP
	 * P0.4: UART0 TX (output push-pull)
	 * P0.5: UART0 TX (input open-drain)
	 * P0.6:          (input open-drain) (EXP8)
	 * P0.7:          (input open-drain) (EXP6)
	 */
	P0MDOUT = P0MDOUT_B0__PUSH_PULL | P0MDOUT_B1__OPEN_DRAIN
				| P0MDOUT_B2__OPEN_DRAIN | P0MDOUT_B3__OPEN_DRAIN
				| P0MDOUT_B4__PUSH_PULL | P0MDOUT_B5__OPEN_DRAIN
				| P0MDOUT_B6__OPEN_DRAIN | P0MDOUT_B7__OPEN_DRAIN;

	P0MDIN = P0MDIN_B0__DIGITAL | P0MDIN_B1__DIGITAL | P0MDIN_B2__DIGITAL
			| P0MDIN_B3__DIGITAL | P0MDIN_B4__DIGITAL | P0MDIN_B5__DIGITAL
			| P0MDIN_B6__DIGITAL | P0MDIN_B7__DIGITAL;

	P0SKIP = P0SKIP_B0__SKIPPED | P0SKIP_B1__NOT_SKIPPED
			| P0SKIP_B2__SKIPPED | P0SKIP_B3__SKIPPED
			| P0SKIP_B4__NOT_SKIPPED | P0SKIP_B5__NOT_SKIPPED
			| P0SKIP_B6__SKIPPED | P0SKIP_B7__NOT_SKIPPED;

	P0_B0 = 1; // so our UART is routed to the Segger UART.

	/*
	 * Port 1:
	 *
	 * P1.0:             (input open-drain) (EXP4)
	 * P1.1:             (input open-drain) (EXP10)
	 * P1.2:             (input open-drain) (SENSOR_I2C_SDA/EXP16)
	 * P1.3:             (input open-drain) (EXP15)
	 * P1.4: DISP_ENABLE (output push-pull) (SKIP) must drive low
	 * P1.5:             (input open-drain)
	 * P1.6: LEDG        (output push-pull) SKIP
	 * P1.7: LEDB        (output push-pull) SKIP
	 */

	P1MDOUT = P1MDOUT_B0__OPEN_DRAIN | P1MDOUT_B1__OPEN_DRAIN
				| P1MDOUT_B2__OPEN_DRAIN | P1MDOUT_B3__OPEN_DRAIN
				| P1MDOUT_B4__PUSH_PULL | P1MDOUT_B5__OPEN_DRAIN
				| P1MDOUT_B6__PUSH_PULL | P1MDOUT_B7__PUSH_PULL;

	P1MDIN = P1MDIN_B0__DIGITAL | P1MDIN_B1__DIGITAL | P1MDIN_B2__DIGITAL
			| P1MDIN_B3__DIGITAL | P1MDIN_B4__DIGITAL | P1MDIN_B5__DIGITAL
			| P1MDIN_B6__DIGITAL | P1MDIN_B7__DIGITAL;

	P1SKIP = P1SKIP_B0__NOT_SKIPPED | P1SKIP_B1__NOT_SKIPPED
			| P1SKIP_B2__NOT_SKIPPED | P1SKIP_B3__NOT_SKIPPED
			| P1SKIP_B4__SKIPPED | P1SKIP_B5__NOT_SKIPPED
			| P1SKIP_B6__SKIPPED | P1SKIP_B7__SKIPPED;

	P1_B4 = 0; // so board controller/Segger drives display, leaving those pins free for us

	/*
	 * Port 2:
	 *
	 * P2.0: LEDR        (output push-pull) (SKIP)
	 * P2.1:             (input open-drain) (EXP12, UART1_TX)
	 * P2.2:             (input open-drain) (EXP14, UART1_RX)
	 * P2.3:             (input open-drain)
	 * P2.4:             (input open-drain)
	 * P2.5:             (input open-drain) (JOYSTICK) (SKIP)
	 * P2.6:             (input open-drain) (EXP11)
	 * P2.7:             (input open-drain) (EXP13)
	 */

	P2MDOUT = P2MDOUT_B0__PUSH_PULL | P2MDOUT_B1__OPEN_DRAIN
				| P2MDOUT_B2__OPEN_DRAIN | P2MDOUT_B3__OPEN_DRAIN
				| P2MDOUT_B4__OPEN_DRAIN | P2MDOUT_B5__OPEN_DRAIN
				| P2MDOUT_B6__OPEN_DRAIN | P2MDOUT_B7__OPEN_DRAIN;

	P2MDIN = P2MDIN_B0__DIGITAL | P2MDIN_B1__DIGITAL | P2MDIN_B2__DIGITAL
			| P2MDIN_B3__DIGITAL | P2MDIN_B4__DIGITAL | P2MDIN_B5__DIGITAL
			| P2MDIN_B6__DIGITAL | P2MDIN_B7__DIGITAL;

	P2SKIP = P2SKIP_B0__SKIPPED | P2SKIP_B1__NOT_SKIPPED
			| P2SKIP_B2__NOT_SKIPPED | P2SKIP_B3__NOT_SKIPPED
			| P2SKIP_B4__NOT_SKIPPED | P2SKIP_B5__SKIPPED
			| P2SKIP_B6__NOT_SKIPPED | P2SKIP_B7__NOT_SKIPPED;

	/*
	 * Port 3:
	 *
	 * P3.0:             (input open-drain) (EXP3)
	 * P3.1:             (input open-drain) (EXP5)
	 * P3.2:             (input open-drain) (EXP7)
	 * P3.3:             (input open-drain) (EXP9)
	 * P3.4:             (input open-drain)
	 * P3.5:             (input open-drain)
	 * P3.6: ADC0 In     (analog, skip in crossbar)
	 * P3.7:             (input open-drain)
	 */

	P3MDOUT = P3MDOUT_B0__OPEN_DRAIN | P3MDOUT_B1__OPEN_DRAIN
				| P3MDOUT_B2__OPEN_DRAIN | P3MDOUT_B3__OPEN_DRAIN
				| P3MDOUT_B4__OPEN_DRAIN | P3MDOUT_B5__OPEN_DRAIN
				| P3MDOUT_B6__OPEN_DRAIN | P3MDOUT_B7__OPEN_DRAIN;

	P3MDIN = P3MDIN_B0__DIGITAL | P3MDIN_B1__DIGITAL | P3MDIN_B2__DIGITAL
			| P3MDIN_B3__DIGITAL | P3MDIN_B4__DIGITAL | P3MDIN_B5__DIGITAL
			| P3MDIN_B6__ANALOG | P3MDIN_B7__DIGITAL;

	P3SKIP = P3SKIP_B0__NOT_SKIPPED | P3SKIP_B1__NOT_SKIPPED
				| P3SKIP_B2__NOT_SKIPPED | P3SKIP_B3__NOT_SKIPPED
				| P3SKIP_B4__NOT_SKIPPED | P3SKIP_B5__NOT_SKIPPED
				| P3SKIP_B6__SKIPPED | P3SKIP_B7__NOT_SKIPPED;

	/*
	 * Port 4:
	 *
	 * P4.0:             (input open-drain)
	 * P4.1:             (input open-drain)
	 * P4.2:             (input open-drain)
	 * P4.3:             (input open-drain)
	 * P4.4:             (input open-drain)
	 * P4.5:             (input open-drain)
	 * P4.6:             (input open-drain)
	 * P4.7:             (input open-drain)
	 */

	P4MDOUT = P4MDOUT_B0__OPEN_DRAIN | P4MDOUT_B1__OPEN_DRAIN
				| P4MDOUT_B2__OPEN_DRAIN | P4MDOUT_B3__OPEN_DRAIN
				| P4MDOUT_B4__OPEN_DRAIN | P4MDOUT_B5__OPEN_DRAIN
				| P4MDOUT_B6__OPEN_DRAIN | P4MDOUT_B7__OPEN_DRAIN;

	P4MDIN = P4MDIN_B0__DIGITAL | P4MDIN_B1__DIGITAL | P4MDIN_B2__DIGITAL
			| P4MDIN_B3__DIGITAL | P4MDIN_B4__DIGITAL | P4MDIN_B5__DIGITAL
			| P4MDIN_B6__DIGITAL | P4MDIN_B7__DIGITAL;
	/*
	 * Configure crossbar.
	 - Weak Pullups enabled

	 - Crossbar enabled
	 - CEX0, CEX1 routed to Port pin
	 - ECI unavailable at Port pin
	 - T0 unavailable at Port pin
	 - T1 unavailable at Port pin
	 ***********************************************************************/
	XBR1 = XBR1_WEAKPUD__PULL_UPS_ENABLED | XBR1_XBARE__ENABLED
			| XBR1_PCA0ME__CEX0_CEX1 | XBR1_ECIE__DISABLED | XBR1_T0E__DISABLED
			| XBR1_T1E__DISABLED;

	/*
	 - UART0 TX, RX routed to Port pins P0.4 and P0.5
	 - SPI I/O unavailable at Port pins
	 - SMBus 0 I/O unavailable at Port pins
	 - CP0 unavailable at Port pin
	 - Asynchronous CP0 unavailable at Port pin
	 - CP1 unavailable at Port pin
	 - Asynchronous CP1 unavailable at Port pin
	 - SYSCLK unavailable at Port pin
	*/
	XBR0 = XBR0_URT0E__ENABLED | XBR0_SPI0E__DISABLED | XBR0_SMB0E__DISABLED
			| XBR0_CP0E__DISABLED | XBR0_CP0AE__DISABLED | XBR0_CP1E__DISABLED
			| XBR0_CP1AE__DISABLED | XBR0_SYSCKE__DISABLED;

	/*
	 * Configure timers for UART.
	 - System clock divided by 4
	 - Counter/Timer 0 uses the clock defined by the prescale field, SCA
	 - Timer 2 high byte uses the clock defined by T2XCLK in TMR2CN0
	 - Timer 2 low byte uses the clock defined by T2XCLK in TMR2CN0
	 - Timer 3 high byte uses the clock defined by T3XCLK in TMR3CN0
	 - Timer 3 low byte uses the clock defined by T3XCLK in TMR3CN0
	 - Timer 1 uses the system clock.
	*/
	CKCON0 = CKCON0_SCA__SYSCLK_DIV_4 | CKCON0_T0M__PRESCALE
			| CKCON0_T2MH__EXTERNAL_CLOCK | CKCON0_T2ML__EXTERNAL_CLOCK
			| CKCON0_T3MH__EXTERNAL_CLOCK | CKCON0_T3ML__EXTERNAL_CLOCK
			| CKCON0_T1M__SYSCLK;
	/*
	 - Mode 0, 13-bit Counter/Timer
	 - Mode 2, 8-bit Counter/Timer with Auto-Reload
	 - Timer Mode
	 - Timer 0 enabled when TR0 = 1 irrespective of INT0 logic level
	 - Timer Mode
	 - Timer 1 enabled when TR1 = 1 irrespective of INT1 logic level
	 */
	TMOD = TMOD_T0M__MODE0 | TMOD_T1M__MODE2 | TMOD_CT0__TIMER
			| TMOD_GATE0__DISABLED | TMOD_CT1__TIMER | TMOD_GATE1__DISABLED;

	/* Set baud rate with timer 1 */
	save = TCON;
	TCON &= ~TCON_TR0__BMASK & ~TCON_TR1__BMASK;
	TH1 = (0x30 << TH1_TH1__SHIFT);
	TCON  |= (save & TCON_TR0__BMASK) | (save & TCON_TR1__BMASK);

	/*
	 - Start Timer 1 running so the UART works.
	 */

	//TCON |= TCON_TR1__RUN;

	/* Enable UART0 reception. */
	SCON0 |= SCON0_REN__RECEIVE_ENABLED;

	/*
	 * Set SAR clock frequency to as close to data sheet max (8.33 MHz) as possible.
	 *
	 * Fclksar = Fsysclk/(ADCSC + 1)
	 *    8E6   = 48E6/(5 + 1)
	 *
	 *  Sample data word should be right justifed because we will average.
	 */
	ADC0CF = ADC0CF_ADLJST__RIGHT_JUSTIFIED | (ADC0CF_ADSC__SHIFT << 5 );

	/*
	 * ADC input select. We've chosen P3.6, which is according to the data sheet
	 * ADC channel #29.
	 */
	AMX0N = AMX0N_AMX0N__GND;
	AMX0P = AMX0N_AMX0N__ADC0N29;

	/*
	 * Set conversion started by Timer 2 overflow.
	 * Convert immediately (converter is always in track mode).
	 * Enable the converter in the main loop as needed.
	 */
	ADC0CN0 = ADC0CN0_ADCM__TIMER2;

	/*
	 * A/D Conversion is started by Timer 2 overflow, at a rate of 100kHz.
	 * Timer 2 is clocked by SYSCLK / 12, which is 4 MHz.
	 * We use the lower half Timer 2 in 8-bit auto reload mode.
	 * To get the 50 kHz rate we reload at:
	 *
	 *    Ftimer_low = Finclock / (256 - TMR2RLL)
	 *
	 *  or (256 - TMR2RLL) = (4E6 / 100E3)
	 *      256 - TMR2RLL = 40 => TMR2RLL = 216
	 *
	 *  In split mode, TR2 is always enabled, so this starts running immediately.
	 *
	 * NB: we will average 512 samples to get the final result.
	 * At 100 kHz, it takes 5.12 ms to get a sample.
	 */
	TMR2RLL = 216;
	TMR2CN0 = TMR2CN0_T2XCLK__SYSCLK_DIV_12 | TMR2CN0_T2SPLIT__8_BIT_RELOAD;

	/*
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
	 * Set the compare interval to 200 ms as our first attempt.
	 * Configure as a 16-bit timer with auto reload (no option to not auto).
	 *
	 * Overflow rate is Ftimer3 = Finclock / (65536 - TMR3RLH:TMR3RLL)
	 *
	 * The timer is clocked by SYSCLK / 2 = 4 MHz, and with the reload value
	 * set to 0, we get a rollover rate of 61.035 Hz or 16.384 ms. This is not
	 * long enough, so we configure our timer for multiple rollovers and count
	 * the rollovers and when we get enough we do our tests.
	 *
	 * So choose a 10 ms reload time, which is 100 Hz, and this results in reload
	 * values:
	 *
	 * 100 Hz = 12 MHz / (65536 - reload)
	 *
	 * 65536 - reload = 12 MHz / 100 Hz
	 * -reload = 120000 - 65536
	 * -reload = 54464 = 0xD4C0
	 *
	 * We need to count 20 rollovers at 10 Hz to get the 250 ms interval.
	 *
	 * The default control register setting TMR3CN0 of 0x00 is correct for this
	 * use. We will enable the timer to run and enable its high byte overflow
	 * interrupt on the first detected button press.
	 */
	TMR3RL = 0xD4C0;

	/*
	 - Enable each interrupt according to its individual mask setting
	 - Disable external interrupt 0
	 - Disable external interrupt 1
	 - Disable all SPI0 interrupts
	 - Disable all Timer 0 interrupt
	 - Disable all Timer 1 interrupt
	 - Disable Timer 2 interrupt
	 - Enable UART0 interrupt
	*/
	/* Enable ADC EOC interrupt, disable others */
	EIE1 =  EIE1_EADC0__ENABLED | EIE1_EWADC0__DISABLED | EIE1_ECP0__DISABLED
			| EIE1_ECP1__DISABLED | EIE1_EPCA0__DISABLED | EIE1_ESMB0__DISABLED
			| EIE1_ET3__DISABLED | EIE1_EUSB0__DISABLED;

	IP = IP_PS0;

	IE = IE_EA__ENABLED | IE_EX0__DISABLED | IE_EX1__DISABLED
			| IE_ESPI0__DISABLED | IE_ET0__DISABLED | IE_ET1__DISABLED
			| IE_ET2__DISABLED | IE_ES0__ENABLED;

}
