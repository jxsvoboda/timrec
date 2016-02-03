#ifndef TYPES_SOURCE_H
#define TYPES_SOURCE_H

#include <types/adt/list.h>

/** Recording source
 *
 * Single recording source, describes a recording source, such as a network
 * audio stream.
 */
typedef struct source {
	/** Link to @c sources_t.presets */
	link_t lsources;
	/** Source name */
	char *name;
	/** Recording class */
	struct rclass *rclass;
	/** Instance parameter (e.g. URI) */
	char *param;
} source_t;

typedef struct {
	list_t sources; /* of source_t */
} sources_t;

#endif
