/* Nes/Snes/Genesis/SMS/Atari to USB
 * Copyright (C) 2006-2007 Raphaël Assénat
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
#include "ten.h"

#define REPORT_SIZE		3
#define GAMEPAD_BYTES	2

/* for chaning IO easily */



/*********** prototypes *************/
static void tenInit(void);
static void tenUpdate(void);
static char tenChanged(void);
static void tenBuildReport(unsigned char *reportBuffer);



// report matching the most recent bytes from the controller
static unsigned char last_read_controller_bytes[REPORT_SIZE];
// the most recently reported bytes
static unsigned char last_reported_controller_bytes[REPORT_SIZE];

static void readController(unsigned char bits[2])
{
	bits[0] = PINC;
	bits[1] = PINB;
}

static void tenInit(void)
{
	unsigned char sreg;
	sreg = SREG;
	cli();


	/* 
	 * --- 10 button on multiuse2 pinout ---
	 * PC5: Up
	 * PC4: Down
	 * PC3: Left
	 * PC2: Right
	 *
	 * PC1: Button 0
	 * PC0: Button 1
	 * PB5: Button 2
	 * PB4: Button 3
	 * PB3: Button 4
	 * PB2: Button 5 (JP2)
	 */

	DDRB = 0; // all inputs
	PORTB |= 0xff; // all pullups enabled

	// all of portC input with pullups
	DDRC &= ~0x3F;
	PORTC |= 0x3F;

	tenUpdate();

	SREG = sreg;
}



static void tenUpdate(void)
{
	unsigned char data[2];
	int x=128,y=128;
	
	readController(data);

	/* Buttons are active low. Invert values. */
	data[0] = data[0] ^ 0xff;
	data[1] = data[1] ^ 0xff;

	if (data[0] & 0x20) { y = 0; } // up
	if (data[0] & 0x10) { y = 255; } //down
	if (data[0] & 0x08) { x = 0; }  // left
	if (data[0] & 0x04) { x = 255; } // right

	last_read_controller_bytes[0]=x;
	last_read_controller_bytes[1]=y;
 	last_read_controller_bytes[2]=0;

	if (data[0] & 0x02) // btn 0
		last_read_controller_bytes[2] |= 0x01;
	if (data[0] & 0x01) // btn 1
		last_read_controller_bytes[2] |= 0x02;
	if (data[1] & 0x20) // btn 2
		last_read_controller_bytes[2] |= 0x04;
	if (data[1] & 0x10) // btn 3
		last_read_controller_bytes[2] |= 0x08;
	if (data[1] & 0x08) // btn 4
		last_read_controller_bytes[2] |= 0x10;
	if (data[1] & 0x04) // btn 5
		last_read_controller_bytes[2] |= 0x20;
	if (data[1] & 0x02) // btn 6
		last_read_controller_bytes[2] |= 0x40;
	if (data[1] & 0x01) // btn 7
		last_read_controller_bytes[2] |= 0x80;


}	

static char tenChanged(void)
{
	static int first = 1;
	if (first) { first = 0;  return 1; }
	
	return memcmp(last_read_controller_bytes, 
					last_reported_controller_bytes, REPORT_SIZE);
}

static void tenBuildReport(unsigned char *reportBuffer)
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

Gamepad tenGamepad = {
	report_size: 		REPORT_SIZE,
	reportDescriptorSize:	sizeof(snes_usbHidReportDescriptor),
	init: 			tenInit,
	update: 		tenUpdate,
	changed:		tenChanged,
	buildReport:		tenBuildReport
};

Gamepad *tenGetGamepad(void)
{
	tenGamepad.reportDescriptor = (void*)snes_usbHidReportDescriptor;

	return &tenGamepad;
}

