# binary
Read an ADC that is driven by an R2R DAC and print an equivalent ASCII value and the ADC value.

# Requirements
This is built with the [Silicon Labs Simplicity Studio 5](https://www.silabs.com/developers/simplicity-studio) tools, with the Keil C51 tools installed.

Make sure to install the 8051 SDK v4.3.1, as those files are not copied to the project.

# hardware
The ADC input is on pin P3.6. An R2R DAC output must be connected to this pin. The ADC is configured to use the VDD supply as the reference, so be sure that the ladder output is scaled between 0 V and VDD (3.3 V).

# Theory of operation
Eight pushbutton switches are presented. They represent a byte in binary format. The original idea was that the eight buttons were monitored by a microcontroller port and would then be recognized as an ASCII character and set to the computer as a keystroke.

It was then decided that an R2R ladder could be used to generate a unique voltage as a function of the buttons pressed. An ADC in the micro can read the voltage and turn it into the keystroke.

The Joystick example provided by SiLabs for the EFM8UB2 micro does most of this. There is one complication: the joystick allows for only one state at a time (depending on where it is pushed) but we want to read the states of up to eight switches at a time. It is difficult to simultaneously press the desired buttons, and there is the additional question of "when is the button press considered valid?" This design addresses these issues.

## Reading the ADC.
The EFM8 has a 10-bit ADC that supports a maximum output rate of 500 ksamples/sec. The conversion can be triggered by any of several sources. An interrupt can be configured to indicate the end of conversion.

EFM8UB Timer 2 is used to pace the convert-starts. It is configured to trigger a new conversions at a rate of 100 ksamples/sec.

It is often useful to average the ADC readings to beat down noise. In the conversion-done ISR we implement a simple boxcar averager. 512 samples are accumulated. This takes 5.12 ms. When we have all of the samples, the average is obtained by simple shift-right to get a 10-bit result. A flag is set indicating that a new averaged sample is available. The main program loop polls, looking for that flag.

## Handling button presses
It can be difficult to press simultanously the eight buttons. Plus the buttons, being mechanical, have the usual "bounce." We need to address when the button press is considered "stable" before we accept the value read by the ADC. We also need to make sure that all switches are released before we start looking for the next set of presses.
EFM8UB2 Timer 3 is used to debounce the button presses and determine that the result is stable. The timer is configured for 16-bit auto-reload (there is no option to not do auto-reload) and it is clocked by SYSCLK/12 (the only option other than external), and the longest count this timer can do is about 16 ms.
Some experimenting showed that the debounce and hold for stability time must be much longer than 16 ms: about 2 seconds (given the hardware we have) works well. We count 2 seconds by configuring Timer 3 to roll over every 10 ms, and in the ISR for Timer 3 rollover we count the number of times that occurred. After 200 rollovers we have waited for 2 seconds.
A state machine is used to manage the button presses. At idle, the ADC reads near 0 with no buttons pressed. The machine looks for an (averaged) ADC value that is not near zero, and when it sees this, it starts Timer 3.
During the interval after the initial button press, we check each new ADC reading. If it is not the same as the previous, it means the button presses have changed, so we restart the 2 second timer and keep going. If it is the same, we check to see if the 2 second timer has expired. If it has, then we have our button press.
After validating the button press, we wait for the ADC value to return to zero before looking for the next presses.
When we have our valid button press, we report that over UART0 to a serial terminal.
