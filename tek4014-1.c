/*-
 * Copyright (c) 2008 Poul-Henning Kamp
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * $Id$
 */

#include "aduc.h"

#include "loran0.h"

/**********************************************************************
 * TEK4014 plotting functions
 */

#define SC(x) ser_char(x)
#define SS(x) ser_str(x)

void
tek4014_start(void)
{
	SC(0x9d); SS("15;green"); SC(0x9c);		/* TEK fg col	*/
	SC(0x9d); SS("16;black"); SC(0x9c);		/* TEK bg col	*/
	SC(0x9b); SS("?38h");				/* TEK mode	*/
	SC(0x0d);
	SC(27); SC(12);					/* TEK PAGE	*/

	/* Draw an outline */
	tek4014_pen();
	tek4014_coord(       0,        0);
	tek4014_coord(4095 - 0,        0);
	tek4014_coord(4095 - 0, 3071 - 0);
	tek4014_coord(       0, 3071 - 0);
	tek4014_coord(       0,        0);
}

void
tek4014_end(void)
{
	SC(31);						/* Text mode */
	SC(27); SC(3);					/* VT PAGE	*/
}

void
tek4014_coord(unsigned x, unsigned y)
{
	unsigned char lox, loy, hix, hiy, eb;
	static unsigned char lloy = -1, lhix = -1, lhiy = -1, leb = -1;

	if (y > 3071)
		y = 3071;
	if (x > 4095)
		x = 4095;
	hiy = (y >> 7) & 0x1f;
	loy = (y >> 2) & 0x1f;
	hix = (x >> 7) & 0x1f;
	lox = (x >> 2) & 0x1f;
	eb = (x & 3) | ((y & 3) << 2);

	if (hiy != lhiy)
		SC(hiy | 0x20);
	if (eb != leb)
		SC( eb | 0x60);
	if (eb != leb || loy != lloy || hix != lhix)
		SC(loy | 0x60);
	if (hix != lhix)
		SC(hix | 0x20);
	SC(lox | 0x40);
        lhiy = hiy;
        lhix = hix;
        lloy = loy;
        leb = eb;
}

void
tek4014_pen(void)
{
	SC(29);
}

void
tek4014_text(void)
{
	SC(31);
}

void
tek4014_curve(unsigned x, unsigned y, int *p, unsigned len, int scalex, int scaley)
{
	int i;

	tek4014_pen();
	tek4014_coord(0, y);
	tek4014_coord(50, y);
	tek4014_pen();
	for (i = 0; i < len; i++)
		tek4014_coord(i * scalex + x, y + p[i] / scaley);
}
