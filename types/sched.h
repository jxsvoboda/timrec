#ifndef TYPES_SCHED_H
#define TYPES_SCHED_H

#include <types/adt/list.h>
#include <types/source.h>
#include <types/preset.h>

typedef struct {
	sources_t *sources;
	list_t presets; /* of preset_t */
} sched_t;

#endif
