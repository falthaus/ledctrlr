# ledctrlr
ATtiny firmware for RC-controlled LED driver



## Overview

- PlatformIO project using `atmelavr` platform (not Arduino)
- Intended for [ATtiny45](https://www.microchip.com/en-us/product/attiny45)
- Measures RC pulse length with 1us resolution (16bit), e.g. from a RC receiver.<br/>
  Uses Timer/Counter0 (extended to 16bit with OVF interrupt), and PCINT0 interrupt.
- Generates 250 kHz PWM output intended for simple RC-filtered single-channel PWM DAC.<br/>
  Uses Timer/Counter1 with 64 MHZ PLL for smaller filters and 8bit resolution.
- Maps three different input pulse length ranges to three PWM output values, <br/>
  e.g. assuming a 3-point switch on transmitter side
- Creates debug output through software UART (as needed).



## Configuration

### Constants in `ledctrlr.c`

|Constant|Description|
|:---|:---|
|`INPUT_[LOW\|MID\|HI]_US`|Threshold for low, middle and high input range (e.g switch positions), in [us]|
|`INPUT_TOL_US`|Tolerance on input thresholds, in [us]|
|`VOUT_[LOW\|MID\|HI]_MV`|Output voltages, corresponding to input range (e.g. switch position), in [mV].<br/>Needs to be calibrated for the specific LED driver and hardware configuration. <br/>Assumes correct `VSUPPLY_MV`|
|`VOUT_DEFAULT_MV`|Default output voltage, at startup, in [mV].<br/>Needs to be calibrated for the specific LED driver and hardware configuration. <br/>Assumes correct `VSUPPLY_MV`|



### Constants in `hardware.h`

|Constant|Description|
|:---|:---|
|`OSCCAL_VALUE`|Internal RC oscillator calibration value to be written to OSCCAL register.<br/>The factory calibration is made at 3.0V and 25°C which may not be applicable, e.g. if a different supply voltage is used.|
|`BAUDRATE`|Baudrate of debug software-UART. Keep at 19200 baud: The time for debug output is limited and longer durations will interfere with the pulse measurements. Faster standardard baud rates will not be accurate enough.|
|`VSUPPLY_MV`|ATtiny Supply voltage, in [mV]|


## Building
<font style="background-color: #FFFF00">TODO</font>



## Programming

### Flash Memory
Avrdude is used through custom PlatformIO build flags, see `.platformio.ini`.
<font style="background-color: #FFFF00">TODO</font>


### Fuses

<font style="background-color: #FFFF00">TODO: CONFIRM values</font>

- System clock not divided (default is 1/8)
- Brown-out Detector (BOD) enabled, trigger level 2.5V .. 2.9V, aligned with 3.3V supply voltage
  <br/>(default is BOD off)

|Fuse Byte|avrdude<br/>memtype|Value|Remarks|
|:------|:------:|:------:|:------|
|Extended Fuse Byte|`efuse`|`0xFF`|default|
|High Fuse Byte|`hfuse`|`0xDD`|BOD trigger level typ. 2.7V|
|Low Fuse Byte|`lfuse`|`0xE2`|clock not divided|


Programming the low fuse byte:
`avrdude -c avrispmkII -p t45 -U lfuse:w:0xe2:m`

Programming the high fuse byte:
`avrdude -c avrispmkII -p t45 -U hfuse:w:0xdd:m`

