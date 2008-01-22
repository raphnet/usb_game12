#ifndef _devdesc_h__
#define _devdesc_h__

#include <avr/pgmspace.h>

extern const char usbDescrDevice[] PROGMEM;
int getUsbDescrDevice_size(void);

#endif // _devdesc_h__

