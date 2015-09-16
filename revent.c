#include <errno.h>
#include <rclass/wget.h>
#include <revent.h>
#include <rsession.h>
#include <stdio.h>

#define CRO2_SRC "http://amp1.cesnet.cz:8000/cro2-256.ogg"

static rsessions_t rsessions;

void revent_init(void)
{
	rsessions_init(&rsessions);
}

int revent_execute(revent_t *revent)
{
	rclass_t *rcls = &rclass_wget;
	rsession_t *rsess;
	int rc;

	switch (revent->etype) {
	case re_start:
		printf("Start recording.\n");
		rc = rsession_create(&rsessions, revent, &rsess);
		if (rc != 0) {
			printf("Error creating recording sesion.\n");
			return rc;
		}

		rc = rcls->rec_start(CRO2_SRC, revent->preset->recname, rsess);
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
