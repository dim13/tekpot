/* $Id$ */
/*
 * Copyright (c) 2009 Dimitri Sokolyuk <sokolyuk@gmail.com>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef	struct point Point;
typedef	struct patch Patch;

struct	point {
	float	x, y, z;
} *point, *ppoint, max;

struct	patch {
	int	p[16];
} *patch, *ppatch;

int	tekheight = 3072;
int	tekwidth = 4096;
int	npoints = 100;

void
sc(char c)
{
	putchar(c);
}

void
ss(char *s)
{
	while (*s)
		putchar(*s++);
}

void
tekenable(int flag)
{
	if (flag) {
		sc(27); ss("[?38h");
	} else {
		sc(27); sc(003);
	}
}

void
tekclear()
{
	tekenable(1);
	sc(27); sc(014);
	tekenable(0);
}

void
tekpen(int flag)
{
	if (flag) {
		sc(29); sc(007);
	} else {
		sc(29);
	}
}
	
void
tekcoord(unsigned int x, unsigned int y)
{
	unsigned	char lox, loy, hix, hiy, eb;
	static	unsigned char lloy = -1, lhix = -1, lhiy = -1, leb = -1;

	if (y >= tekheight)
		y = tekheight - 1;
	if (x >= tekwidth)
		x = tekwidth - 1;
	hiy = (y >> 7) & 0x1f;
	loy = (y >> 2) & 0x1f;
	hix = (x >> 7) & 0x1f;
	lox = (x >> 2) & 0x1f;
	eb = (x & 3) | ((y & 3) << 2);

	if (hiy != lhiy)
		sc(hiy | 0x20);
	if (eb != leb)
		sc(eb | 0x60);
	if (eb != leb || loy != lloy || hix != lhix)
		sc(loy | 0x60);
	if (hix != lhix)
		sc(hix | 0x20);
	sc(lox | 0x40);
	lhiy = hiy;
	lhix = hix;
	lloy = loy;
	leb = eb;
}

void
loadpatch(char *filename, int *patches, int *verticles)
{
	int	ii, jj;
	float	x, y, z;
	int	a, b, c, d;
	FILE	*fd;

	fd = fopen(filename, "r");
	if (!fd)
		err(1, "can't open %s", filename);

	fscanf(fd, "%i\n", patches);

	patch = calloc(*patches, sizeof(Patch));
	if (!patch)
		err(1, "can't allocate memory");
	ppatch = patch;

	for (ii = 0; ii < *patches; ii++) {
		jj = 0;
		a = 0;
		b = 0;
		c = 0;
		d = 0;
		fscanf(fd, "%i, %i, %i, %i,", &a, &b, &c, &d);
		ppatch->p[jj++] = --a;
		ppatch->p[jj++] = --b;
		ppatch->p[jj++] = --c;
		ppatch->p[jj++] = --d;
		fscanf(fd, "%i, %i, %i, %i,", &a, &b, &c, &d);
		ppatch->p[jj++] = --a;
		ppatch->p[jj++] = --b;
		ppatch->p[jj++] = --c;
		ppatch->p[jj++] = --d;
		fscanf(fd, "%i, %i, %i, %i,", &a, &b, &c, &d);
		ppatch->p[jj++] = --a;
		ppatch->p[jj++] = --b;
		ppatch->p[jj++] = --c;
		ppatch->p[jj++] = --d;
		fscanf(fd, "%i, %i, %i, %i\n", &a, &b, &c, &d);
		ppatch->p[jj++] = --a;
		ppatch->p[jj++] = --b;
		ppatch->p[jj++] = --c;
		ppatch->p[jj++] = --d;
		++ppatch;
	}

	fscanf(fd, "%i\n", verticles);

	point = calloc(*verticles, sizeof(Point));
	if (!point)
		err(1, "can't allocate memory");
	ppoint = point;

	max.x = 0;
	max.y = 0;
	max.z = 0;

	for (ii = 0; ii < *verticles; ii++) {
		fscanf(fd, "%f, %f, %f\n", &x, &y, &z);
		ppoint->x = x;
		if (abs(x) > max.x)
			max.x = abs(x);
		ppoint->y = y;
		if (abs(y) > max.y)
			max.y = abs(y);
		ppoint->z = z;
		if (abs(z) > max.z)
			max.z = abs(z);
		++ppoint;
	}

	fclose(fd);
}

int
rnd(float f)
{
	if (f > 0)
		f += 0.5;
	else
		f -= 0.5;

	return (int)f;
}

void
rotx(Point *p)
{
	p->y = 0.5 * p->y + 0.866 * p->z;
	p->z = -0.866 * p->y + 0.5 * p->z;
}

void
project(Point *p)
{
	float	d = 100 * max.z;
	float	zoom = 1000;

	rotx(p);

	p->x *= d/(2*d - p->z);
	p->y *= d/(2*d - p->z);

	p->x *= zoom;
	p->y *= zoom;

	p->x += tekwidth/2;
	p->y += tekheight/3;

	tekcoord(rnd(p->x), rnd(p->y));
}

void
vec(Point *a, Point *b, float lambda)
{
	a->x += lambda*(b->x - a->x);
	a->y += lambda*(b->y - a->y);
	a->z += lambda*(b->z - a->z);
}

void
bezier(Patch *pp, int step, int steps)
{
	Point	p[16];
	int	i, j, k;
	float	s = (float)step/(float)steps;

	for (i = 0; i < 16; i++) {
		k = pp->p[i];
		p[i].x = point[k].x;
		p[i].y = point[k].y;
		p[i].z = point[k].z;
	}

	for (i = 15; i > 0; i--)
		for (j = 0; j < i; j++)
			vec(&p[j], &p[j + 1], s);	

	project(p);
}

void
usage()
{
	extern	char *__progname;

	fprintf(stderr, "usage: %s datafile\n", __progname);

	exit(1);
}

int
main(int argc, char **argv)
{
	int	patches, verticles;
	int	i, j;

	if (argc != 2)
		usage();

	loadpatch(*++argv, &patches, &verticles);

	tekclear();
	tekenable(1);
	for (i = 0; i < patches; i++) {
		tekpen(0);
		for (j = 0; j < npoints; j++)
			bezier(&patch[i], j, npoints);
		tekpen(1);
	}
	tekenable(0);

	return 0;
}
