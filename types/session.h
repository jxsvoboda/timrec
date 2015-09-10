#ifndef TYPES_SESSION_H
#define TYPES_SESSION_H

#include <types/preset.h>

/** Active recording session
 */
typedef struct {
	/** Preset from which the session originates */
	preset_t *preset;
} session_t;

typedef struct {
	list_t sessions; /* of session_t */
} sessions_t;

#endif
