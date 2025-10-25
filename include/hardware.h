#ifndef _HARDWARE_H_
#define _HARDWARE_H_
////////////////////////////////////////////////////////////////////////////////
//					  __ __
//		      RESET -|  -  |- VCC
//	Jumper		PB3 -|     |- PB2	TXD (software tx-only UART)
//	Jumper		PB4 -|     |- PB1	Output (OCR1A)
//				GND -|_____|- PB0	RC Input (PCINT0)
//
//
////////////////////////////////////////////////////////////////////////////////

// I/O Configuration
//						// I/O		Weak pull-ups
#define RCIN 	(PB0)	// I		no
#define OUT 	(PB1)	// O		no
#define TXD 	(PB2)	// O		no
#define CFG0 	(PB3)	// I		yes
#define CFG1 	(PB4)	// I		yes

////////////////////////////////////////////////////////////////////////////////

// Hardware-dependent Constants

// Software UART baud rate
#define BAUDRATE        (19200)

// Set OSCCAL calibration value for 3.3V and ambient temperature
// (factory calibration is done for 3.0V and 25°C)
#define OSCCAL_VALUE    (0x4F)


////////////////////////////////////////////////////////////////////////////////
#endif