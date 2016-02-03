#ifndef TYPES_PRESET_H
#define TYPES_PRESET_H

#include <adt/list.h>
#include <types/source.h>
#include <types/startspec.h>

/** Recording preset
 *
 * Single recording preset, describes a (posibly regularly occuring)
 * broadcast of interest.
 */
typedef struct preset {
	link_t lsched;
	/** Start time specification */
	startspec_t start;
	/** Nominal length in seconds */
	unsigned long duration_secs;
	/** Seconds to start in advance */
	unsigned long pre_buf_secs;
	/** Seconds to extend beyond nominal end */
	unsigned long post_buf_secs;
	/** Recording source */
	source_t *source;
	/** Recording name */
	char *recname;
} preset_t;

#endif
