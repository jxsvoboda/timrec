#ifndef FIELD_H
#define FIELD_H

#include <stdio.h>
#include <types/field.h>

extern void field_read_init(field_read_t *, FILE *);
extern int field_read_getc(field_read_t *);

#endif
