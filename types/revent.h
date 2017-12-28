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

#ifndef TYPES_REVENT_H
#define TYPES_REVENT_H

#include <time.h>
#include <types/preset.h>

typedef enum {
	/** Start recording */
	re_start,
	/** Stop recording */
	re_stop
} revent_type_t;

/** Recording event
 */
typedef struct {
	/** Up pointer to revents */
	struct revents *revents;
	/** Link in revents_t.revents */
	link_t lrevents;
	/** Event type */
	revent_type_t etype;
	/** Preset where this event originates */
	preset_t *preset;
	/** Time of this event */
	time_t t;
	/** Nominal start time */
	time_t nst;
} revent_t;

typedef struct revents {
	list_t revents;
} revents_t;

#endif
