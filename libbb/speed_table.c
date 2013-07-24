/* vi: set sw=4 ts=4: */
/*
 * compact speed_t <-> speed functions for busybox
 *
 * Copyright (C) 2003  Manuel Novoa III  <mjn3@codepoet.org>
 *
 * Licensed under GPLv2 or later, see file LICENSE in this source tree.
 */

#include "libbb.h"

struct speed_map {
	unsigned short speed;
	unsigned short value;
};

static const struct speed_map speeds[] = {
#define SPEED_MAP(n) { B ##n, (int)(n < 0x8000u ? n : 0x8000u | (n >> 8)) }

	SPEED_MAP(0),
	SPEED_MAP(50),
	SPEED_MAP(75),
	SPEED_MAP(110),
	SPEED_MAP(134),
	SPEED_MAP(150),

	SPEED_MAP(200),
	SPEED_MAP(300),
	SPEED_MAP(600),
	SPEED_MAP(1200),
	SPEED_MAP(1800),
	SPEED_MAP(2400),
	SPEED_MAP(4800),
	SPEED_MAP(9600),

#ifdef B19200
	SPEED_MAP(19200),
#elif defined(EXTA)
	{ EXTA, 19200 },
#endif
#ifdef B38400
	SPEED_MAP(38400),
#elif defined(EXTB)
	{ EXTB, 0x8000u | (38400 >> 8) },
#endif
#ifdef B57600
	SPEED_MAP(57600),
#endif
#ifdef B115200
	SPEED_MAP(115200),
#endif
#ifdef B230400
	SPEED_MAP(230400),
#endif
#ifdef B460800
	SPEED_MAP(460800),
#endif
#ifdef B921600
	SPEED_MAP(921600),
#endif

#undef SPEED_MAP
};

enum { NUM_SPEEDS = ARRAY_SIZE(speeds) };

unsigned FAST_FUNC tty_baud_to_value(speed_t speed)
{
	int i = 0;

	do {
		if (speed == speeds[i].speed) {
			if (speeds[i].value & 0x8000U) {
				return ((unsigned long) (speeds[i].value) & ~0x8000U) << 8;
			}
			return speeds[i].value;
		}
	} while (++i < NUM_SPEEDS);

	return 0;
}

speed_t FAST_FUNC tty_value_to_baud(unsigned int value)
{
	int i = 0;

	do {
		if (value == tty_baud_to_value(speeds[i].speed)) {
			return speeds[i].speed;
		}
	} while (++i < NUM_SPEEDS);

	return (speed_t) - 1;
}

#if 0
/* testing code */
#include <stdio.h>

int main(void)
{
	unsigned long v;
	speed_t s;

	for (v = 0 ; v < 1000000; v++) {
		s = tty_value_to_baud(v);
		if (s == (speed_t) -1) {
			continue;
		}
		printf("v = %lu -- s = %0lo\n", v, (unsigned long) s);
	}

	printf("-------------------------------\n");

	for (s = 0 ; s < 010017+1; s++) {
		v = tty_baud_to_value(s);
		if (!v) {
			continue;
		}
		printf("v = %lu -- s = %0lo\n", v, (unsigned long) s);
	}

	return 0;
}
#endif
