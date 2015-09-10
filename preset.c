#include <assert.h>
#include <errno.h>
#include <preset.h>
#include <startspec.h>
#include <stdlib.h>
#include <time.h>

#include <stdio.h>
int preset_create(preset_t **rpreset)
{
	preset_t *preset;

	preset = calloc(1, sizeof(preset_t));
	if (preset == NULL)
		return ENOMEM;

	preset->pre_buf_secs = 10*60;
	preset->post_buf_secs = 15*60;

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
	time_t tstart, tstop = 0;
	struct tm tm;
	ds_date_t date;

	switch (preset->start.dsk) {
	case ds_date:
		tstart = preset_start(preset);
		tstop = preset_stop(preset);

		if (tstart >= t) {
			printf("**START**\n");
			/* Start */
			revent->etype = re_start;
			revent->preset = preset;
			revent->t = tstart;
		} else if (tstop >= t) {
			printf("**STOP**\n");
			/* Stop */
			revent->etype = re_stop;
			revent->preset = preset;
			revent->t = tstop;
		} else {
			printf("**NOTHING**\n");
			return ENOENT;
		}
		break;
	case ds_dow:
		/* Get date from t */
		(void) localtime_r(&t, &tm);
		date.y = tm.tm_year + 1900;
		date.m = tm.tm_mon + 1;
		date.d = tm.tm_mday;

		printf("Cur date: %d-%02d-%02d\n", date.y, date.m, date.d);
		while (preset_stop_with_date(preset, &date) >= t) {
			printf("Go to previous day.\n");
			/* Previous day */
			startspec_date_decr(&date);
			printf("Cur date: %d-%02d-%02d\n", date.y, date.m, date.d);
		}

		printf("Cur date: %d-%02d-%02d\n", date.y, date.m, date.d);
		while (true) {
			printf("Cur date: %d-%02d-%02d\n", date.y, date.m, date.d);
			printf("Check DOW\n");
			/* Is it the correct DOW? */
			if (startspec_date_get_dow(&date) ==
			    preset->start.ds.dow.dow) {

				tstart = preset_start_with_date(preset, &date);
				tstop = preset_stop_with_date(preset, &date);

				if (tstart >= t) {
					printf("**START**\n");
					/* Start */
					revent->etype = re_start;
					revent->preset = preset;
					revent->t = tstart;
					return 0;
				} else if (tstop >= t) {
					printf("**STOP**\n");
		    			/* Stop */
					revent->etype = re_stop;
					revent->preset = preset;
					revent->t = tstop;
					return 0;
				}
			}

			printf("Go to next day.\n");
			/* Next day */
			startspec_date_incr(&date);
		}

		return ENOENT;
	}

	return 0;
}
