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
