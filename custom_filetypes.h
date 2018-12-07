#ifndef CUSTOM_FILETYPES_H

#define CUSTOM_FILETYPES_H

#include <stdint.h>

#define byte uint8_t

struct note {
	int beat;	
	byte pitch;
	byte velocity;
	int length;
	bool is_playing;
	bool is_recorded;
	unsigned long millis_started;
};

#endif