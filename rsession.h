#ifndef RSESSION_H
#define RSESSION_H

#include <time.h>
#include <types/preset.h>
#include <types/revent.h>
#include <types/rsession.h>

extern void rsessions_init(rsessions_t *);
extern int rsession_create(rsessions_t *, revent_t *, rsession_t **);
extern rsession_t *rsession_find(rsessions_t *, revent_t *);
extern void rsession_destroy(rsession_t *);

#endif
