#ifndef SOURCE_H
#define SOURCE_H

#include <types/adt/list.h>
#include <types/source.h>

extern int sources_load(const char *, sources_t **);
extern void sources_destroy(sources_t *);
extern int sources_get_source(sources_t *, char *name, source_t **);

#endif
