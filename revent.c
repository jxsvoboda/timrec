#include <revent.h>
#include <stdio.h>

int revent_execute(revent_t *revent)
{
	switch (revent->etype) {
	case re_start:
		printf("Start recording. ");
		break;
	case re_stop:
		printf("Stop recording. ");
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
