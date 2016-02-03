#include <adt/list.h>
#include <errno.h>
#include <rclass/wget.h>
#include <source.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NAME_BUF_SIZE 64
static char name_buf[NAME_BUF_SIZE];

static int source_create(source_t **rs)
{
	source_t *s;

	s = calloc(1, sizeof(source_t));
	if (s == NULL)
		return ENOMEM;

	*rs = s;
	return 0;
}

static void source_destroy(source_t *s)
{
	free(s);
}

static int source_skip_ws(FILE *f)
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

static int source_parse_fld_name(FILE *f, char **fldname)
{
	int c;
	int rc;
	int i;

	rc = source_skip_ws(f);
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

static int source_parse_name(FILE *f, source_t *s)
{
	int rc;
	int i;
	int c;

	rc = source_skip_ws(f);
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
			printf("Source name too long.\n");
			return EIO;
		}

		name_buf[i++] = c;
	}

	name_buf[i] = '\0';
	s->name = strdup(name_buf);
	if (s->name == NULL)
		return ENOMEM;

	return 0;
}

static int source_parse_rclass(FILE *f, source_t *s)
{
	int rc;
	int i;
	int c;

	rc = source_skip_ws(f);
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
			printf("Source name too long.\n");
			return EIO;
		}

		name_buf[i++] = c;
	}

	name_buf[i] = '\0';

	if (strcmp(name_buf, "wget") != 0) {
		printf("Unknown recording class '%s'.\n", name_buf);
		return EIO;
	}

	s->rclass = &rclass_wget;

	return 0;
}

static int source_parse_param(FILE *f, source_t *s)
{
	int rc;
	int i;
	int c;

	rc = source_skip_ws(f);
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
			printf("Source name too long.\n");
			return EIO;
		}

		name_buf[i++] = c;
	}

	name_buf[i] = '\0';
	s->param = strdup(name_buf);
	if (s->param == NULL)
		return ENOMEM;

	return 0;
}

static int source_parse_field(FILE *f, char *fldname, source_t *s)
{
	if (strcmp(fldname, "name") == 0)
		return source_parse_name(f, s);
	else if (strcmp(fldname, "rclass") == 0)
		return source_parse_rclass(f, s);
	else if (strcmp(fldname, "param") == 0)
		return source_parse_param(f, s);
	else {
		printf("Unexpected field name '%s'.\n", fldname);
		return EIO;
	}

	return 0;
}

/** Verify that all required fields are present. */
static int source_validate(source_t *s)
{
	int rc;

	rc = 0;

	if (s->name == NULL) {
		printf("Source must have 'name' specified.\n");
		rc = EINVAL;
	}

	if (s->rclass == NULL) {
		printf("Source must have 'rclass' specified.\n");
		rc = EINVAL;
	}

	if (s->param == NULL) {
		printf("Source must have 'param' specified.\n");
		rc = EINVAL;
	}

	return rc;
}

static int sources_parse_source(sources_t *sources, FILE *f)
{
	int rc;
	char *fldname = NULL;
	source_t *s;
	int nf;
	int c;

	nf = 0;

	rc = source_create(&s);
	if (rc != 0)
		return rc;

	while (true) {
		rc = source_skip_ws(f);
		if (rc != 0)
			goto error;

		c = fgetc(f);
		if (c == '\n')
			break;
		ungetc(c, f);

		rc = source_parse_fld_name(f, &fldname);
		if (rc != 0)
			goto error;

		if (fldname == NULL)
			break;

		rc = source_skip_ws(f);
		if (rc != 0)
			goto error;

		++nf;

		c = fgetc(f);
		if (c != '=') {
			printf("'=' expected, got '%c'.\n", c);
			rc = EIO;
			goto error;
		}

		rc = source_parse_field(f, fldname, s);
		if (rc != 0)
			goto error;

		rc = source_skip_ws(f);
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
		rc = source_validate(s);
		if (rc != 0) {
			source_destroy(s);
			return rc;
		}

		printf("Loaded source %p name='%s'\n", s, s->name);
		list_append(&s->lsources, &sources->sources);
	} else {
		source_destroy(s);
	}

	return 0;
error:
	if (fldname != NULL)
		free(fldname);
	return rc;
}

int sources_load(const char *fname, sources_t **rsources)
{
	sources_t *sources = NULL;
	FILE *f = NULL;
	int rc;

	sources = calloc(1, sizeof(sources_t));
	if (sources == NULL)
		return ENOMEM;

	f = fopen(fname, "rt");
	if (f == NULL) {
		rc = ENOENT;
		goto error;
	}

	list_initialize(&sources->sources);

	while (!feof(f)) {
		rc = sources_parse_source(sources, f);
		if (rc != 0)
			goto error;
	}

	*rsources = sources;
	return 0;
error:
	if (f != NULL)
		fclose(f);
	if (sources != NULL)
		free(sources);
	return rc;
}

void sources_destroy(sources_t *sources)
{
	free(sources);
}

int sources_get_source(sources_t *sources, char *name, source_t **rs)
{
	list_foreach(sources->sources, lsources, source_t, s) {
		if (strcmp(s->name, name) == 0) {
			*rs = s;
			return 0;
		}
	}

	return ENOENT;
}
