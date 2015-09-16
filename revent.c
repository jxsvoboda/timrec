#include <errno.h>
#include <rclass/wget.h>
#include <revent.h>
#include <rsession.h>
#include <startspec.h>
#include <stdio.h>
#include <string.h>

#define CRO2_SRC "http://amp1.cesnet.cz:8000/cro2-256.ogg"
#define RECNAME_MAX_LEN 1024

static rsessions_t rsessions;

void revent_init(void)
{
	rsessions_init(&rsessions);
}

static int revent_rn_subst_name(revent_t *revent, const char *recname, char **sn)
{
	const char *p;
	char buf[RECNAME_MAX_LEN + 1];
	unsigned long di;
	int rc;
	ds_date_t date;

	p = recname;
	di = 0;

	while (*p != '\0') {
		if (di >= RECNAME_MAX_LEN) {
			/* Too long */
			return ENOSPC;
		}

		if (*p == '%') {
			++p;
			switch (*p) {
			case '%':
				buf[di++] = '%';
				break;
			case 'D':
				if (revent != NULL) {
					startspec_ts_date_tod(revent->nst, &date, NULL);
				} else {
					date.d = date.m = date.y = 0;
				}

				rc = snprintf(buf + di, RECNAME_MAX_LEN + 1 - di,
				    "%u-%02u-%02u", date.y, date.m, date.d);
				if (rc < 0) {
					/* Error */
					return EIO;
				}
				if (rc >= RECNAME_MAX_LEN + 1 - di) {
					/* Too long */
					return ENOSPC;
				}
				di += rc;
				break;
			default:
				return EINVAL;
			}
			++p;
		} else {
			buf[di++] = *p++;
		}
	}

	buf[di] = '\0';
	if (sn != NULL) {
		*sn = strdup(buf);
		if (*sn == NULL)
			return ENOMEM;
	}

	return 0;
}

bool revent_rn_valid(const char *recname)
{
	return revent_rn_subst_name(NULL, recname, NULL) == 0;
}

int revent_rn_subst(revent_t *revent, char **sn)
{
	return revent_rn_subst_name(revent, revent->preset->recname, sn);
}

int revent_execute(revent_t *revent)
{
	rclass_t *rcls = &rclass_wget;
	rsession_t *rsess;
	char *rname;
	int rc;

	switch (revent->etype) {
	case re_start:
		printf("Start recording.\n");
		rc = rsession_create(&rsessions, revent, &rsess);
		if (rc != 0) {
			printf("Error creating recording sesion.\n");
			return rc;
		}

		rc = revent_rn_subst(revent, &rname);
		if (rc != 0) {
			printf("Error determining recording name.\n");
			return ENOMEM;
		}

		rc = rcls->rec_start(CRO2_SRC, rname, rsess);
		if (rc != 0) {
			printf("Error starting recording.\n");
			return rc;
		}

		break;
	case re_stop:
		printf("Stop recording.\n");
		rsess = rsession_find(&rsessions, revent);
		if (rsess == NULL) {
			printf("Error finding session to stop.\n");
			return ENOENT;
		}

		rc = rcls->rec_stop(rsess);
		if (rc != 0) {
			printf("Error stopping recording.\n");
			return rc;
		}

		rsession_destroy(rsess);
		break;
	}

	printf("Nominal time: %ld, name: '%s'\n", revent->t,
	    revent->preset->recname);

	return 0;
}

revent_t *revent_first(revents_t *revents)
{
	link_t *link;

	link = list_first(&revents->revents);
	if (link == NULL)
		return NULL;

	return list_get_instance(link, revent_t, lrevents);
}

revent_t *revent_next(revent_t *revent)
{
	link_t *link;

	link = list_next(&revent->lrevents, &revent->revents->revents);
	if (link == NULL)
		return NULL;

	return list_get_instance(link, revent_t, lrevents);
}
