/*
 * Copyright 2017 Jiri Svoboda
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#ifndef TYPES_STARTSPEC_H
#define TYPES_STARTSPEC_H

#include <stdbool.h>

typedef enum {
	/** Specific date (d.m.y) */
	ds_date = 1,
	/** Every week on the specified day-of-week */
	ds_dow = 2
} day_spec_kind_t;

typedef struct {
	int d;
	int m;
	int y;
} ds_date_t;

typedef struct {
	int dow;
} ds_dow_t;

typedef struct {
	int h;
	int m;
} tod_t;

/** Start time specification */
typedef struct {
	day_spec_kind_t dsk;
	union {
		ds_date_t date;
		ds_dow_t dow;
	} ds;
	tod_t tod;
	bool tod_valid;
} startspec_t;

#endif
