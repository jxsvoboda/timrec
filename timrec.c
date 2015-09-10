#include <errno.h>
#include <revent.h>
#include <sched.h>
#include <stdio.h>
#include <unistd.h>

#define IDLE_SLEEP (60 * 60)

const char *sched_fname = "sched.txt";

int main(void)
{
	sched_t *sched;
	revent_t revent;
	struct timespec ts;
	time_t curtime;
	time_t vtime;
	unsigned int s;
	int rc;

	printf("Timrec starting\n");
	rc = sched_load(sched_fname, &sched);
	if (rc != 0) {
		printf("Error loading schedule.\n");
		return 1;
	}

	printf("Loaded schedule '%s'.\n", sched_fname);
	rc = clock_gettime(CLOCK_REALTIME, &ts);
	if (rc != 0) {
		printf("Error getting time.\n");
		return 1;
	}

	vtime = ts.tv_sec;

	while (true) {
		rc = sched_get_next_event(sched, vtime, &revent);
		if (rc != 0) {
			if (rc == ENOENT) {
				printf("No planned recording.\n");
				sleep(IDLE_SLEEP);
				continue;
			} else {
				printf("Error getting next event.\n");
				return 1;
			}
		}

		printf("Next event: t=%ld type=%d recname='%s'\n",
		    revent.t, revent.etype, revent.preset->recname);

		while (true) {
			rc = clock_gettime(CLOCK_REALTIME, &ts);
			if (rc != 0) {
				printf("Error getting time.\n");
				return 1;
			}

			curtime = ts.tv_sec;

			printf("curtime=%ld\n", curtime);
			if (curtime >= revent.t)
				break;

			if (revent.t - curtime > IDLE_SLEEP)
				s = IDLE_SLEEP;
			else
				s = revent.t - curtime;

			printf("Sleeping for %u seconds.\n", s);
			sleep(s);
		}

		rc = revent_execute(&revent);
		if (rc != 0) {
			printf("Error executing event.\n");
		}

		vtime = revent.t + 1;
	}

	printf("Timrec exiting\n");
	return 0;
}
