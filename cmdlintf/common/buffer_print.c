/*
 */

#ifndef __BUFFER_PRINT_H__
#define __BUFFER_PRINT_H__

#define MAX_TRANSFER_BUFFER	10240

static char	bp_buffer[MAX_TRANSFER_BUFFER];
static int	bp_index = 0;

static void bpinit (void) { bp_index = 0; }
static int bplen (void) { return bp_index; }
static char *bpbptr (void) { return bp_buffer; }

static int bprint (const char *fmt, ...) {
	va_list	ap;
	int	len;

	va_start (ap, fmt);
	len = vsprintf (&bp_buffer[bp_index], fmt, ap);
	va_end (ap);

	bp_index += len;

	return len;
}

#endif
