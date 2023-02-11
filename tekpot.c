/* $Id$ */
/*
 * Copyright (c) 2009 Dimitri Sokolyuk <demon@dim13.org>
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
#include <math.h>
#include <err.h>

typedef	struct point Point;
typedef	struct patch Patch;

struct	point {
	float	x, y, z;
} *point, *ppoint, max;

struct	patch {
	int	p[4][4];
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
		sc(27);
		ss("[?38h");
	} else {
		sc(27);
		sc(003);
	}
}

void
tekclear()
{
	tekenable(1);
	sc(27);
	sc(014);
	tekenable(0);
}

void
tekpen(int flag)
{
	if (flag) {
		sc(29);
	} else {
		sc(29);
		sc(007);
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
	int	i;
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

	for (i = 0; i < *patches; i++) {
		fscanf(fd, "%i, %i, %i, %i,", &a, &b, &c, &d);
		ppatch->p[0][0] = --a;
		ppatch->p[0][1] = --b;
		ppatch->p[0][2] = --c;
		ppatch->p[0][3] = --d;
		fscanf(fd, "%i, %i, %i, %i,", &a, &b, &c, &d);
		ppatch->p[1][0] = --a;
		ppatch->p[1][1] = --b;
		ppatch->p[1][2] = --c;
		ppatch->p[1][3] = --d;
		fscanf(fd, "%i, %i, %i, %i,", &a, &b, &c, &d);
		ppatch->p[2][0] = --a;
		ppatch->p[2][1] = --b;
		ppatch->p[2][2] = --c;
		ppatch->p[2][3] = --d;
		fscanf(fd, "%i, %i, %i, %i\n", &a, &b, &c, &d);
		ppatch->p[3][0] = --a;
		ppatch->p[3][1] = --b;
		ppatch->p[3][2] = --c;
		ppatch->p[3][3] = --d;
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

	for (i = 0; i < *verticles; i++) {
		fscanf(fd, "%f, %f, %f\n", &x, &y, &z);
		ppoint->x = x;
		if (fabs(x) > max.x)
			max.x = fabs(x);
		ppoint->y = y;
		if (fabs(y) > max.y)
			max.y = fabs(y);
		ppoint->z = z;
		if (fabs(z) > max.z)
			max.z = fabs(z);
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

#if 0
void
bernstein(int steps)
{
	int uinc = 1.0/steps;
	int u = uinc;

	for (i = 1; i < steps; i++, u += uinc) {
		u_sqr = u * u;			/* u^2 */
		tmp = 1.0 - u;			/* (1-u) */
		tmp_sqr = tmp * tmp;		/* (1-u)^2 */
		b[0][i] = tmp * tmp_sqr;	/* (1-u)^3 */
		b[1][i] = 3 * u * tmp_sqr;	/* 3u(1-u)^2 */
		b[2][i] = 3 * u_sqr * tmp;	/* 3u^2(1-u) */
		b[2][i] = u * u_sqr;		/* u^3 */
	}
}
#endif

void
bezier(Patch *pp, int step, int steps)
{
	Point	p[16];
	int	i, j, k;
	float	s = (float)step/(float)steps;

	for (i = 0; i < 4; i++) {
		for (j = 0; j < 4; j++) {
			k = pp->p[i][j];
			p[4*i+j].x = point[k].x;
			p[4*i+j].y = point[k].y;
			p[4*i+j].z = point[k].z;
		}
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
		tekpen(1);
		for (j = 0; j < npoints; j++)
			bezier(&patch[i], j, npoints);
		tekpen(0);
	}
	tekenable(0);

	return 0;
}
