#ifndef PRESET_H
#define PRESET_H

#include <time.h>
#include <types/preset.h>
#include <types/revent.h>

extern int preset_create(preset_t **);
extern void preset_destroy(preset_t *);
extern int preset_get_next_event(preset_t *, time_t, revent_t *);

#endif
