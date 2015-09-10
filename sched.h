#ifndef SCHED_H
#define SCHED_H

#include <types/adt/list.h>
#include <types/revent.h>
#include <types/sched.h>

extern int sched_load(const char *, sched_t **);
extern void sched_destroy(sched_t *);
extern int sched_get_next_event(sched_t *, time_t, revent_t *);
extern int sched_get_cur_start_events(sched_t *, time_t, revents_t *);
extern void sched_free_events(revents_t *);

#endif
