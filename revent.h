#ifndef REVENT_H
#define REVENT_H

#include <types/revent.h>

extern int revent_execute(revent_t *);
extern revent_t *revent_first(revents_t *);
extern revent_t *revent_next(revent_t *);

#endif
