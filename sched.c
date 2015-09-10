#include <adt/list.h>
#include <errno.h>
#include <preset.h>
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
		if (c != ' ' && c != '\t') {
			printf("ungetc('%c')\n", c);
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
			printf("End of file reached.\n");
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

	printf("Dow=%d\n", d);
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

	printf("Time: %02d:%02d\n", h, m);
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
		printf("read quantity\n");
		nread = fscanf(f, "%d", &val);
		if (nread != 1)
			break;

		printf("q=%d skip ws\n", val);
		rc = sched_skip_ws(f);
		if (rc != 0)
			return EIO;

		c = fgetc(f);
		printf("read unit '%c'\n", c);
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

	printf("Duration=%ds\n", duration);
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
	printf("Source = '%s'\n", name_buf);
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
	printf("Name = '%s'\n", name_buf);
	p->recname = strdup(name_buf);
	if (p->recname == NULL)
		return ENOMEM;

	return 0;
}

static int sched_parse_field(FILE *f, char *fldname, preset_t *p)
{
	printf("Parse field '%s'.\n", fldname);
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
		printf("parse field name\n");
		rc = sched_parse_fld_name(f, &fldname);
		if (rc != 0)
			goto error;

		if (fldname == NULL)
			break;

		printf("skip ws\n");
		rc = sched_skip_ws(f);
		if (rc != 0)
			goto error;

		printf("Field name is '%s'\n", fldname);
		++nf;

		c = fgetc(f);
		if (c != '=') {
			printf("'=' expected, got '%c'.\n", c);
			rc = EIO;
			goto error;
		}

		printf("parse fields\n");
		rc = sched_parse_field(f, fldname, p);
		if (rc != 0)
			goto error;

		rc = sched_skip_ws(f);
		if (rc != 0)
			goto error;

		c = fgetc(f);
		if (c == '\n')
			break;

		if (c != ';') {
			printf("';' expected, got '%c'.\n", c);
			rc = EIO;
			goto error;
		}
	}

	if (nf > 0) {
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
		rc = EIO;
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

int sched_get_next_event(sched_t *sched, time_t t, revent_t *revent)
{
	int rc;
	revent_t beste;
	bool have_beste = false;
	revent_t e;

	list_foreach(sched->presets, lsched, preset_t, p) {
		rc = preset_get_next_event(p, t, &e);
		printf("preset_get_next_event -> %d\n", rc);
		if (rc == 0) {
			printf("Got event.\n");
			if (!have_beste || e.t < beste.t) {
				beste = e;
				have_beste = true;
			}
		}
	}

	if (!have_beste)
		return ENOENT;

	*revent = beste;
	return 0;
}

int sched_get_cur_start_events(sched_t *sched, time_t t, revents_t *revents)
{
	return 0;
}

void sched_free_events(revents_t *revents)
{
}
