#include <adt/list.h>
#include <errno.h>
#include <preset.h>
#include <revent.h>
#include <sched.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NAME_BUF_SIZE 64
static char name_buf[NAME_BUF_SIZE];

static int sched_skip_ws(FILE *f)
{
	int c;

	while (!feof(f)) {
		c = fgetc(f);
		if (c == 0)
			return EIO;
		if (c == '#') {
			do {
				c = fgetc(f);
			} while (c != EOF && c != '\n');
			if (c == '\n')
				ungetc(c, f);
			break;
		}
		if (c != ' ' && c != '\t') {
			ungetc(c, f);
			break;
		}
	}

	return 0;
}

static int sched_parse_fld_name(FILE *f, char **fldname)
{
	int c;
	int rc;
	int i;

	rc = sched_skip_ws(f);
	if (rc != 0)
		return rc;

	i = 0;
	while (true) {
		c = fgetc(f);
		if (c == EOF) {
			*fldname = NULL;
			return 0;
		}

		if (c == ';') {
			printf("';' unexpected.\n");
			return EIO;
		}

		if (c == '=')
			break;

		if (c == ' ' || c == '\t')
			break;

		if (i >= NAME_BUF_SIZE - 1) {
			printf("Field name too long.\n");
			return EIO;
		}

		name_buf[i++] = c;
	}

	ungetc(c, f);
	name_buf[i] = '\0';
	*fldname = strdup(name_buf);
	if (*fldname == NULL)
		return ENOMEM;

	return 0;
}

static int sched_parse_field_dow(FILE *f, preset_t *p)
{
	int c;
	char dow[3];
	int d;

	c = fgetc(f);
	if (c < 'a' || c > 'z')
		return EIO;

	dow[0] = c;

	c = fgetc(f);
	if (c < 'a' || c > 'z')
		return EIO;

	dow[1] = c;
	dow[2] = '\0';

	if (strcmp(dow, "po") == 0)
		d = 0;
	else if (strcmp(dow, "ut") == 0)
		d = 1;
	else if (strcmp(dow, "st") == 0)
		d = 2;
	else if (strcmp(dow, "ct") == 0)
		d = 3;
	else if (strcmp(dow, "pa") == 0)
		d = 4;
	else if (strcmp(dow, "so") == 0)
		d = 5;
	else if (strcmp(dow, "ne") == 0)
		d = 6;
	else {
		printf("Unknown day of week '%s'.\n", dow);
	}

	p->start.dsk = ds_dow;
	p->start.ds.dow.dow = d;

	return 0;
}

static int sched_parse_field_date(FILE *f, preset_t *p)
{
	int nread;
	int y, m, d;

	nread = fscanf(f, "%d-%d-%d", &y, &m, &d);
	if (nread != 3) {
		printf("Invalid date format.\n");
		return EIO;
	}

	p->start.dsk = ds_date;
	p->start.ds.date.y = y;
	p->start.ds.date.m = m;
	p->start.ds.date.d = d;

	return 0;
}

static int sched_parse_field_time(FILE *f, preset_t *p)
{
	int nread;
	int h, m;

	nread = fscanf(f, "%d:%02d", &h, &m);
	if (nread != 2) {
		printf("Invalid time format.\n");
		return EIO;
	}

	p->start.tod.h = h;
	p->start.tod.m = m;
	p->start.tod_valid = true;

	return 0;
}

static int sched_parse_field_duration(FILE *f, preset_t *p)
{
	int nread;
	int val;
	int nfields;
	int scale;
	int lscale;
	int c;
	int duration;
	int rc;

	duration = 0;

	nfields = 0; lscale = 3600 * 24 + 1;
	while (true) {
		nread = fscanf(f, "%d", &val);
		if (nread != 1)
			break;

		rc = sched_skip_ws(f);
		if (rc != 0)
			return EIO;

		c = fgetc(f);
		switch (c) {
		case 'd':
			scale = 3600 * 24;
		case 'h':
			scale = 3600;
			break;
		case 'm':
			scale = 60;
			break;
		case 's':
			scale = 1;
			break;
		}

		if (scale >= lscale) {
			printf("Invalid duration format.\n");
			return EIO;
		}

		duration += val * scale;
		nfields++;
		lscale = scale;
	}

	if (nfields == 0) {
		printf("Duration expected.\n");
		return EIO;
	}

	if (duration <= 0) {
		printf("Duration must be positive.\n");
		return EIO;
	}

	p->duration_secs = duration;

	return 0;
}

static int sched_parse_field_source(FILE *f, preset_t *p)
{
	int rc;
	int i;
	int c;

	rc = sched_skip_ws(f);
	if (rc != 0)
		return EIO;

	i = 0;
	while (true) {
		c = fgetc(f);
		if (c == EOF)
			break;

		if (c == ';') {
			ungetc(c, f);
			break;
		}

		if (c == ' ' || c == '\t')
			break;

		if (i >= NAME_BUF_SIZE - 1) {
			printf("Source name too long.\n");
			return EIO;
		}

		name_buf[i++] = c;
	}

	if (strlen(name_buf) == 0) {
		printf("Error: empty source name\n");
		return EIO;
	}

	name_buf[i] = '\0';
	return 0;
}

static int sched_parse_field_name(FILE *f, preset_t *p)
{
	int rc;
	int i;
	int c;

	rc = sched_skip_ws(f);
	if (rc != 0)
		return EIO;

	i = 0;
	while (true) {
		c = fgetc(f);
		if (c == EOF)
			break;

		if (c == ';' || c == '\n') {
			ungetc(c, f);
			break;
		}

		if (c == ' ' || c == '\t')
			break;

		if (i >= NAME_BUF_SIZE - 1) {
			printf("Recording name too long.\n");
			return EIO;
		}

		name_buf[i++] = c;
	}

	name_buf[i] = '\0';
	p->recname = strdup(name_buf);
	if (p->recname == NULL)
		return ENOMEM;

	if (!revent_rn_valid(p->recname)) {
		printf("Invalid recording name pattern '%s'.\n", p->recname);
		return EIO;
	}

	return 0;
}

static int sched_parse_field(FILE *f, char *fldname, preset_t *p)
{
	if (strcmp(fldname, "dow") == 0)
		return sched_parse_field_dow(f, p);
	else if (strcmp(fldname, "date") == 0)
		return sched_parse_field_date(f, p);
	else if (strcmp(fldname, "time") == 0)
		return sched_parse_field_time(f, p);
	else if (strcmp(fldname, "duration") == 0)
		return sched_parse_field_duration(f, p);
	else if (strcmp(fldname, "source") == 0)
		return sched_parse_field_source(f, p);
	else if (strcmp(fldname, "name") == 0)
		return sched_parse_field_name(f, p);
	else {
		printf("Unexpected field name '%s'.\n", fldname);
		return EIO;
	}

	return 0;
}

/** Verify that all required fields are present. */
static int sched_preset_validate(preset_t *p)
{
	int rc;

	rc = 0;

	if (p->start.dsk == 0) {
		printf("Preset must have either 'dow' or 'date' specified.\n");
		rc = EINVAL;
	}

	if (!p->start.tod_valid) {
		printf("Preset must have 'time' specified.\n");
		rc = EINVAL;
	}

	if (p->duration_secs == 0) {
		printf("Preset must have 'duration' specified.\n");
		rc = EINVAL;
	}

	if (p->recname == NULL) {
		printf("Preset must have 'name' specified.\n");
		rc = EINVAL;
	}

	return rc;
}

static int sched_parse_preset(sched_t *sched, FILE *f)
{
	int rc;
	char *fldname = NULL;
	preset_t *p;
	int nf;
	int c;

	nf = 0;

	rc = preset_create(&p);
	if (rc != 0)
		return rc;

	while (true) {
		rc = sched_skip_ws(f);
		if (rc != 0)
			goto error;

		c = fgetc(f);
		if (c == '\n')
			break;
		ungetc(c, f);

		rc = sched_parse_fld_name(f, &fldname);
		if (rc != 0)
			goto error;

		if (fldname == NULL)
			break;

		rc = sched_skip_ws(f);
		if (rc != 0)
			goto error;

		++nf;

		c = fgetc(f);
		if (c != '=') {
			printf("'=' expected, got '%c'.\n", c);
			rc = EIO;
			goto error;
		}

		rc = sched_parse_field(f, fldname, p);
		if (rc != 0)
			goto error;

		rc = sched_skip_ws(f);
		if (rc != 0)
			goto error;

		c = fgetc(f);
		if (c == EOF || c == '\n')
			break;

		if (c != ';') {
			printf("';' expected, got '%c'.\n", c);
			rc = EIO;
			goto error;
		}
	}

	if (nf > 0) {
		rc = sched_preset_validate(p);
		if (rc != 0) {
			preset_destroy(p);
			return rc;
		}

		printf("Loaded preset %p recname='%s'\n", p, p->recname);
		list_append(&p->lsched, &sched->presets);
	} else {
		preset_destroy(p);
	}
	return 0;
error:
	if (fldname != NULL)
		free(fldname);
	return rc;
}

int sched_load(const char *fname, sched_t **rsched)
{
	sched_t *sched = NULL;
	FILE *f = NULL;
	int rc;

	sched = calloc(1, sizeof(sched_t));
	if (sched == NULL)
		return ENOMEM;

	f = fopen(fname, "rt");
	if (f == NULL) {
		rc = ENOENT;
		goto error;
	}

	list_initialize(&sched->presets);

	while (!feof(f)) {
		rc = sched_parse_preset(sched, f);
		if (rc != 0)
			goto error;
	}

	*rsched = sched;
	return 0;
error:
	if (f != NULL)
		fclose(f);
	if (sched != NULL)
		free(sched);
	return rc;
}

void sched_destroy(sched_t *sched)
{
	free(sched);
}

int sched_get_next_events(sched_t *sched, time_t t, revents_t *revents)
{
	int rc;
	time_t best_t;
	revent_t e, *le;

	sched_init_events(revents);

	list_foreach(sched->presets, lsched, preset_t, p) {
		rc = preset_get_next_event(p, t, &e);
		if (rc == 0) {
			if (list_empty(&revents->revents) || e.t < best_t) {
				sched_free_events(revents);
				best_t = e.t;

				le = calloc(1, sizeof(revent_t));
				if (le == NULL)
					return ENOMEM;
				*le = e;
				list_append(&le->lrevents, &revents->revents);
				le->revents = revents;
			} else if (e.t == best_t) {
				le = calloc(1, sizeof(revent_t));
				if (le == NULL)
					return ENOMEM;
				*le = e;
				list_append(&le->lrevents, &revents->revents);
				le->revents = revents;
			}
		}
	}

	if (list_empty(&revents->revents))
		return ENOENT;

	return 0;
}

int sched_get_cur_start_events(sched_t *sched, time_t t, revents_t *revents)
{
	int rc;

	sched_init_events(revents);

	list_foreach(sched->presets, lsched, preset_t, p) {
		rc = preset_append_cur_start_events(p, t, revents);
		if (rc != 0)
			return rc;
	}

	return 0;
}

void sched_init_events(revents_t *revents)
{
	list_initialize(&revents->revents);
}

void sched_free_events(revents_t *revents)
{
	revent_t *e;

	while (!list_empty(&revents->revents)) {
		e = revent_first(revents);
		list_remove(&e->lrevents);
		free(e);
	}
}
