#include <errno.h>
#include <preset.h>
#include <startspec.h>
#include <stdlib.h>

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

int preset_get_next_event(preset_t *preset, time_t t, revent_t *revent)
{
	time_t ns = 0;

	switch (preset->start.dsk) {
	case ds_date:
		ns = startspec_date_tod_ts(&preset->start.ds.date,
		    &preset->start.tod);

		if (ns - preset->pre_buf_secs >= t) {
			printf("**START**\n");
			/* Start */
			revent->etype = re_start;
			revent->preset = preset;
			revent->t = ns - preset->pre_buf_secs;
		} else if (ns + preset->duration_secs + preset->post_buf_secs >= t) {
			printf("**STOP**\n");
			/* Stop */
			revent->etype = re_stop;
			revent->preset = preset;
			revent->t = ns + preset->duration_secs +
			    preset->post_buf_secs;
		} else {
			printf("**NOTHING**\n");
			return ENOENT;
		}
		break;
	case ds_dow:
		return ENOTSUP;
		break;
	}

	return 0;
}
