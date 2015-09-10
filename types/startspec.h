#ifndef TYPES_STARTSPEC_H
#define TYPES_STARTSPEC_H

#include <stdbool.h>

typedef enum {
	/** Specific date (d.m.y) */
	ds_date = 1,
	/** Every week on the specified day-of-week */
	ds_dow = 2
} day_spec_kind_t;

typedef struct {
	int d;
	int m;
	int y;
} ds_date_t;

typedef struct {
	int dow;
} ds_dow_t;

typedef struct {
	int h;
	int m;
} tod_t;

/** Start time specification */
typedef struct {
	day_spec_kind_t dsk;
	union {
		ds_date_t date;
		ds_dow_t dow;
	} ds;
	tod_t tod;
	bool tod_valid;
} startspec_t;

#endif
