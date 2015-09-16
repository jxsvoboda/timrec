#include <errno.h>
#include <revent.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define IDLE_SLEEP (60 * 60)

const char *sched_fname = "sched.txt";

static void timrec_revents_execute(revents_t *revents)
{
	revent_t *e;
	int rc;

	printf("Executing %lu events.\n", list_count(&revents->revents));
	e = revent_first(revents);
	while (e != NULL) {
		rc = revent_execute(e);
		if (rc != 0) {
			printf("Error executing event.\n");
		}

		e = revent_next(e);
	}
}

int main(void)
{
	sched_t *sched;
	struct timespec ts;
	revents_t nevents;
	time_t curtime;
	time_t vtime;
	revent_t *e0, *e;
	unsigned int s;
	char *rname;
	int rc;

	printf("Timrec starting\n");

	setvbuf(stdout, NULL, _IOLBF, 0);
	revent_init();

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

	rc = sched_get_cur_start_events(sched, vtime, &nevents);
	if (rc != 0) {
		printf("Error getting event list.\n");
		return 1;
	}

	if (!list_empty(&nevents.revents)) {
		printf("WARNING: Late-starting %lu sessions which should "
		    "already be in progress.\n", list_count(&nevents.revents));

		timrec_revents_execute(&nevents);
	}

	while (true) {
		rc = sched_get_next_events(sched, vtime, &nevents);
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

		e0 = revent_first(&nevents);

		printf("Next event(s) at t=%ld:\n", e0->t);
		e = e0;
		while (e != NULL) {
			rc = revent_rn_subst(e, &rname);
			if (rc != 0) {
				printf("Error substituting recording name.\n");
				return 1;
			}
			printf("\ttype=%d recname='%s'\n",
			    e->etype, rname);
			free(rname);
			e = revent_next(e);
		}

		while (true) {
			rc = clock_gettime(CLOCK_REALTIME, &ts);
			if (rc != 0) {
				printf("Error getting time.\n");
				return 1;
			}

			curtime = ts.tv_sec;

			printf("curtime=%ld\n", curtime);
			if (curtime >= e0->t)
				break;

			printf("Next event happening in %ld seconds.\n",
			    e0->t - curtime);

			if (e0->t - curtime > IDLE_SLEEP)
				s = IDLE_SLEEP;
			else
				s = e0->t - curtime;

			printf("Sleeping for %u seconds.\n", s);
			sleep(s);
		}

		printf("Executing %lu events.\n", list_count(&nevents.revents));
		timrec_revents_execute(&nevents);

		vtime = e0->t + 1;
		sched_free_events(&nevents);
	}

	printf("Timrec exiting\n");
	return 0;
}
