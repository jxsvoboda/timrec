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
