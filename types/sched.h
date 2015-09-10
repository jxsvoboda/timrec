#ifndef TYPES_SCHED_H
#define TYPES_SCHED_H

#include <types/adt/list.h>
#include <types/preset.h>

typedef struct {
	list_t presets; /* of preset_t */
} sched_t;

#endif
