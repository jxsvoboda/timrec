#ifndef TYPES_RCLASS_DVB_H
#define TYPES_RCLASS_DVB_H

#include <unistd.h>

typedef struct {
	pid_t zap_pid;
	pid_t dd_pid;
} dvb_session_t;

#endif
