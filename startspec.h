#ifndef STARTSPEC_H
#define STARTSPEC_H

#include <time.h>
#include <types/startspec.h>

extern time_t startspec_date_tod_ts(ds_date_t *, tod_t *);
extern void startspec_date_decr(ds_date_t *);
extern void startspec_date_incr(ds_date_t *);
extern int startspec_date_get_dow(ds_date_t *);

#endif
