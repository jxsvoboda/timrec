/*
 * Copyright 2017 Jiri Svoboda
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include <field.h>
#include <stdio.h>

/** Start reading a configuration field.
 *
 * @param field Field structure to initialize
 * @param f File to read from
 */
void field_read_init(field_read_t *field, FILE *f)
{
	int c;

	field->f = f;
	c = fgetc(f);
	if (c == '"') {
		field->quotes = true;
	} else {
		field->quotes = false;
		ungetc(c, f);
	}
}

/** Decode next character from configuration field.
 *
 * @param field Initialized field structure
 * @return Decoded character
 */
int field_read_getc(field_read_t *field)
{
	int c;

	c = fgetc(field->f);
	if (field->quotes) {
		/* End of quoted field */
		if (c == '"') {
			/* We've skipped over the terminating double quote */
			return EOF;
		}

		/* Escaped characters */
		if (c == '\\') {
			/* Only backslash and double quote can be escape-encoded */
			c = fgetc(field->f);
			if (c != '\\' && c != '"')
				return EOF;
		}
	} else {
		/* Characters that cannot appear in an unquoted field */
		if (c == ' ' || c == '\t' || c == ';' || c == '"' || c == '\n') {
			ungetc(c, field->f);
			return EOF;
		}
	}

	return c;
}
