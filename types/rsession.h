#ifndef TYPES_RSESSION_H
#define TYPES_RSESSION_H

#include <adt/list.h>
#include <time.h>

typedef struct {
	list_t rsess;
} rsessions_t;

typedef struct {
	/** Rclass-private data */
	void *priv;
	/** Preset that generated this session */
	struct preset *preset;
	/** Nominal start time */
	time_t nst;
	link_t lrsess;
} rsession_t;

#endif
