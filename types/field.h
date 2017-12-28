#ifndef TYPES_FIELD_H
#define TYPES_FIELD_H

#include <stdbool.h>
#include <stdio.h>

/** Config field reader */
typedef struct {
	/** Input file */
	FILE *f;
	/** @c true if there are quotes around the field */
	bool quotes;
} field_read_t;

#endif
