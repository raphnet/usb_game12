/* Twelve input to USB (4 directions + 8 buttons) to USB
 * Copyright (C) 2008 Raphaël Assénat
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * The author may be contacted at raph@raphnet.net
 */
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include <string.h>
#include "gamepad.h"
#include "analog.h"

#define REPORT_SIZE		3
#define GAMEPAD_BYTES	2

/* for chaning IO easily */



/*********** prototypes *************/
static void analogInit(void);
static void analogUpdate(void);
static char analogChanged(void);
static void analogBuildReport(unsigned char *reportBuffer);



// report matching the most recent bytes from the controller
static unsigned char last_read_controller_bytes[REPORT_SIZE];
// the most recently reported bytes
static unsigned char last_reported_controller_bytes[REPORT_SIZE];

unsigned short adc_sample(char id, int n_samples)
{
	unsigned short cur_val = 0;
	unsigned long total = 0;
	int i;

	if (id<0 || id > 5)
		return 0xffff;

	/* set MUX3:0  (bits 3:0). No mask needed because of range
	 * check above. */
	ADMUX = (1<<REFS0) | id;

	for (i=0; i<n_samples; i++) {
		ADCSRA |= (1<<ADSC);    /* start conversion */
		while (!(ADCSRA & (1<<ADIF)))
			{ /* do nothing... */ };
		
		cur_val = ADCL;
		cur_val |= (ADCH << 8);
		total += cur_val << 6; // convert to 16 bit

		_delay_ms(1);
	}

	if (n_samples == 1) {
		return cur_val;
	}
	return (total / n_samples) & 0xffff;
}

static void readController(unsigned char bits[4])
{
	bits[0] = PINC;
	bits[1] = PINB;
	bits[2] = adc_sample(1, 5) >> 8;
	bits[3] = adc_sample(0, 5) >> 8;
}

static void analogInit(void)
{
	unsigned char sreg;
	sreg = SREG;
	cli();


	/* 
	 * --- 10 button on multiuse2 pinout ---
	 * PC5: Button 1
	 * PC4: Button 2
	 * PC3: Button 3
	 * PC2: Button 4
	 *
	 * PC1: Analog 1
	 * PC0: Analog 2
	 *
	 * PB5: Button 5
	 * PB4: Button 6
	 * PB3: Button 7
	 * PB2: Button 8 (JP2)
	 */

	DDRB = 0; // all inputs
	PORTB |= 0xff; // all pullups enabled

	// all of portC input, pullups on buttons only
	DDRC &= ~0x3f;
	PORTC |= 0x3c;
	PORTC &= ~0x03; // no pull-up on analog inputs

	/* Use AVCC (usb 5 volts) and select ADC0. */
	ADMUX = (1<<REFS0);

	/* Enable ADC and setup prescaler to /128 (gives 93khz) */
	ADCSRA = (1<<ADEN) | (1<<ADPS2) | (1<<ADPS1) | (1<<ADPS0);

	analogUpdate();

	SREG = sreg;
}



static void analogUpdate(void)
{
	unsigned char data[4];

	readController(data);

	/* Buttons are active low. Invert values. */
	data[0] = data[0] ^ 0xff;
	data[1] = data[1] ^ 0xff;

	last_read_controller_bytes[0]=data[2];
	last_read_controller_bytes[1]=data[3]^0xff;

 	last_read_controller_bytes[2]=0;

	if (data[0] & 0x20) // btn 0
		last_read_controller_bytes[2] |= 0x01;
	if (data[0] & 0x10) // btn 1
		last_read_controller_bytes[2] |= 0x02;
	if (data[0] & 0x08) // btn 2
		last_read_controller_bytes[2] |= 0x04;
	if (data[0] & 0x04) // btn 3
		last_read_controller_bytes[2] |= 0x08;

	if (data[1] & 0x20) // btn 4
		last_read_controller_bytes[2] |= 0x10;
	if (data[1] & 0x10) // btn 5
		last_read_controller_bytes[2] |= 0x20;
	if (data[1] & 0x08) // btn 6
		last_read_controller_bytes[2] |= 0x40;
	if (data[1] & 0x04) // btn 7
		last_read_controller_bytes[2] |= 0x80;


}	

static char analogChanged(void)
{
	static int first = 1;
	if (first) { first = 0;  return 1; }
	
	return memcmp(last_read_controller_bytes, 
					last_reported_controller_bytes, REPORT_SIZE);
}

static void analogBuildReport(unsigned char *reportBuffer)
{
	if (reportBuffer != NULL)
	{
		memcpy(reportBuffer, last_read_controller_bytes, REPORT_SIZE);
	}
	memcpy(last_reported_controller_bytes, 
			last_read_controller_bytes, 
			REPORT_SIZE);	
}

#include "snes_descriptor.c"

Gamepad analogGamepad = {
	report_size: 		REPORT_SIZE,
	reportDescriptorSize:	sizeof(snes_usbHidReportDescriptor),
	init: 			analogInit,
	update: 		analogUpdate,
	changed:		analogChanged,
	buildReport:		analogBuildReport
};

Gamepad *analogGetGamepad(void)
{
	analogGamepad.reportDescriptor = (void*)snes_usbHidReportDescriptor;

	return &analogGamepad;
}

