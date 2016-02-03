#ifndef TYPES_RCLASS_H
#define TYPES_RCLASS_H

#include <types/source.h>
#include <types/rsession.h>

/** Recording class
 */
typedef struct rclass {
	int (*rec_start)(source_t *, char *, rsession_t *);
	int (*rec_stop)(rsession_t *);
} rclass_t;

#endif
