#ifndef TYPES_REVENT_H
#define TYPES_REVENT_H

#include <time.h>
#include <types/preset.h>

typedef enum {
	/** Start recording */
	re_start,
	/** Stop recording */
	re_stop
} revent_type_t;

/** Recording event
 */
typedef struct {
	/** Event type */
	revent_type_t etype;
	/** Preset where this event originates */
	preset_t *preset;
	/** Time of this event */
	time_t t;
} revent_t;

typedef struct {
	list_t revents;
} revents_t;

#endif
