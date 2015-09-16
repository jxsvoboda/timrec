#ifndef TYPES_RCLASS_H
#define TYPES_RCLASS_H

#include <types/rsession.h>

/** Recording class
 */
typedef struct {
	int (*rec_start)(char *, char *, rsession_t *);
	int (*rec_stop)(rsession_t *);
} rclass_t;

#endif
