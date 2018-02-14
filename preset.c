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

#include <assert.h>
#include <errno.h>
#include <preset.h>
#include <startspec.h>
#include <stdlib.h>
#include <time.h>

int preset_create(preset_t **rpreset)
{
	preset_t *preset;

	preset = calloc(1, sizeof(preset_t));
	if (preset == NULL)
		return ENOMEM;

	preset->pre_buf_secs = 10 * 60;
	preset->post_buf_secs = 15 * 60;

	*rpreset = preset;
	return 0;
}

void preset_destroy(preset_t *preset)
{
	free(preset);
}

/** Start time with preset instantitated on specified date. */
static time_t preset_start_with_date(preset_t *preset, ds_date_t *date)
{
	return startspec_date_tod_ts(date, &preset->start.tod) -
	    preset->pre_buf_secs;
}

static time_t preset_start(preset_t *preset)
{
	assert(preset->start.dsk == ds_date);

	return preset_start_with_date(preset, &preset->start.ds.date);
}

/** Nominal start time with preset instantitated on specified date. */
static time_t preset_nom_start_with_date(preset_t *preset, ds_date_t *date)
{
	return startspec_date_tod_ts(date, &preset->start.tod);
}

static time_t preset_nom_start(preset_t *preset)
{
	assert(preset->start.dsk == ds_date);

	return preset_nom_start_with_date(preset, &preset->start.ds.date);
}

/** Stop time with preset instantitated on specified date. */
static time_t preset_stop_with_date(preset_t *preset, ds_date_t *date)
{
	return startspec_date_tod_ts(date, &preset->start.tod) +
	    preset->duration_secs + preset->post_buf_secs;
}

static time_t preset_stop(preset_t *preset)
{
	assert(preset->start.dsk == ds_date);

	return preset_stop_with_date(preset, &preset->start.ds.date);
}

int preset_get_next_event(preset_t *preset, time_t t, revent_t *revent)
{
	time_t tstart, tstop, nstart;
	ds_date_t date;

	switch (preset->start.dsk) {
	case ds_date:
		tstart = preset_start(preset);
		tstop = preset_stop(preset);
		nstart = preset_nom_start(preset);

		if (tstart >= t) {
			/* Start */
			revent->etype = re_start;
			revent->preset = preset;
			revent->t = tstart;
			revent->nst = nstart;
			revent->revents = NULL;
			link_initialize(&revent->lrevents);
		} else if (tstop >= t) {
			/* Stop */
			revent->etype = re_stop;
			revent->preset = preset;
			revent->t = tstop;
			revent->nst = nstart;
			revent->revents = NULL;
			link_initialize(&revent->lrevents);
		} else {
			return ENOENT;
		}
		break;
	case ds_dow:
		/* Get date from t */
		startspec_ts_date_tod(t, &date, NULL);

		while (preset_stop_with_date(preset, &date) >= t) {
			/* Previous day */
			startspec_date_decr(&date);
		}

		while (true) {
			/* Is it the correct DOW? */
			if (startspec_date_get_dow(&date) ==
			    preset->start.ds.dow.dow) {

				tstart = preset_start_with_date(preset, &date);
				tstop = preset_stop_with_date(preset, &date);
				nstart = preset_nom_start_with_date(preset,
				    &date);

				if (tstart >= t) {
					/* Start */
					revent->etype = re_start;
					revent->preset = preset;
					revent->t = tstart;
					revent->nst = nstart;
					revent->revents = NULL;
					link_initialize(&revent->lrevents);
					return 0;
				} else if (tstop >= t) {
		    			/* Stop */
					revent->etype = re_stop;
					revent->preset = preset;
					revent->t = tstop;
					revent->nst = nstart;
					revent->revents = NULL;
					link_initialize(&revent->lrevents);
					return 0;
				}
			}

			/* Next day */
			startspec_date_incr(&date);
		}

		return ENOENT;
	}

	return 0;
}

/** Append start events for all active sessions.
 *
 * Append start events for all sessions active at @a t generated from
 * @a preset to @a revents.
 */
int preset_append_cur_start_events(preset_t *preset, time_t t,
    revents_t *revents)
{
	time_t tstart, tstop, nstart;
	revent_t *revent;
	ds_date_t date;

	switch (preset->start.dsk) {
	case ds_date:
		tstart = preset_start(preset);
		tstop = preset_stop(preset);
		nstart = preset_nom_start(preset);

		if (tstart < t && tstop >= t) {
			revent = calloc(1, sizeof(revent_t));
			if (revent == NULL)
				return ENOMEM;

			revent->etype = re_start;
			revent->preset = preset;
			revent->t = tstart;
			revent->nst = nstart;
			revent->revents = revents;
			link_initialize(&revent->lrevents);
			list_append(&revent->lrevents, &revents->revents);
		}
		break;
	case ds_dow:
		/* Get date from t */
		startspec_ts_date_tod(t, &date, NULL);

		while (preset_stop_with_date(preset, &date) >= t) {
			/* Previous day */
			startspec_date_decr(&date);
		}

		while (preset_start_with_date(preset, &date) <= t) {
			/* Is it the correct DOW? */
			if (startspec_date_get_dow(&date) ==
			    preset->start.ds.dow.dow) {

				tstart = preset_start_with_date(preset, &date);
				tstop = preset_stop_with_date(preset, &date);
				nstart = preset_nom_start_with_date(preset,
				    &date);

				if (tstart <= t && tstop >= t) {
					revent = calloc(1, sizeof(revent_t));
					if (revent == NULL)
						return ENOMEM;

					revent->etype = re_start;
					revent->preset = preset;
					revent->t = tstart;
					revent->nst = nstart;
					revent->revents = revents;
					link_initialize(&revent->lrevents);
					list_append(&revent->lrevents, &revents->revents);

				}
			}

			/* Next day */
			startspec_date_incr(&date);
		}
		break;
	}

	return 0;
}
