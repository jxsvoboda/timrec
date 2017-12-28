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

#ifndef TYPES_PRESET_H
#define TYPES_PRESET_H

#include <adt/list.h>
#include <types/source.h>
#include <types/startspec.h>

/** Recording preset
 *
 * Single recording preset, describes a (posibly regularly occuring)
 * broadcast of interest.
 */
typedef struct preset {
	link_t lsched;
	/** Start time specification */
	startspec_t start;
	/** Nominal length in seconds */
	unsigned long duration_secs;
	/** Seconds to start in advance */
	unsigned long pre_buf_secs;
	/** Seconds to extend beyond nominal end */
	unsigned long post_buf_secs;
	/** Recording source */
	source_t *source;
	/** Recording name */
	char *recname;
} preset_t;

#endif
