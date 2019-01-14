/*
 * Copyright 2019 Jiri Svoboda
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

/*
 * DVB recording class
 *
 * Record DVB stream using dvb-tools (dvbv5-zap and dd)
 *
 * param: station name (as appears in channels.conf)
 */

#include <errno.h>
#include <rclass/dvb.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <source.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <stdio.h>
#include <unistd.h>

static int dvb_start(source_t *, char *, rsession_t *);
static int dvb_stop(rsession_t *);

rclass_t rclass_dvb = {
	dvb_start,
	dvb_stop
};

static int dvb_start(source_t *source, char *recname, rsession_t *rs)
{
	pid_t pid;
	char *rfname = NULL;
	char *rfarg = NULL;
	char *lfname = NULL;
	dvb_session_t *dvbs = NULL;
	struct stat sbuf;
	int serno;
	char *url;
	FILE *lf = NULL;
	int rc;

	url = source->param;
	printf("dvb url=%s\n", url);

	dvbs = calloc(1, sizeof(dvb_session_t));
	if (rs == NULL) {
		printf("Out of memory.\n");
		rc = ENOMEM;
		goto error;
	}

	rs->priv = dvbs;

	rc = asprintf(&rfname, "/var/opt/timrec/%s.mpg", recname);
	if (rc < 0) {
		printf("Out of memory.\n");
		rc = ENOMEM;
		goto error;
	}

	rc = stat(rfname, &sbuf);
	serno = 1;
	if (rc == 0) {
		/* File already exists */
		free(rfname);

		while (rc == 0) {
			rc = asprintf(&rfname, "/var/opt/timrec/%s.%d.mpg", recname,
			    serno++);
			if (rc < 0) {
				printf("Out of memory.\n");
				rc = ENOMEM;
				goto error;
			}

			rc = stat(rfname, &sbuf);
		}
	}

	rc = asprintf(&rfarg, "of=%s", rfname);
	if (rc < 0) {
		printf("Out of memory.\n");
		rc = ENOMEM;
		goto error;
	}

	rc = asprintf(&lfname, "/var/opt/timrec/%s.log", recname);
	if (rc < 0) {
		printf("Out of memory.\n");
		rc = ENOMEM;
		goto error;
	}

	lf = fopen(lfname, "at");
	if (lf == NULL) {
		printf("Error opening log file '%s'.\n", lfname);
		rc = EIO;
		goto error;
	}

	pid = fork();
	if (pid < 0) {
		printf("Error forking.\n");
		rc = ENOMEM;
		goto error;
	} else if (pid == 0) {
		/* Child */
		close(2);

		/* Redirect standard error to log file */
		rc = dup2(fileno(lf), 2);
		if (rc < 0)
			exit(1);

		(void) execlp("dvbv5-zap", "dvbv5-zap", "-r", url, NULL);
		/* If we get here, execlp() has failed */
		exit(1);
	}

	/* Parent */
	fclose(lf);
	dvbs->zap_pid = pid;

	pid = fork();
	if (pid < 0) {
		printf("Error forking.\n");
		rc = ENOMEM;
		goto error;
	} else if (pid == 0) {
		/* Child */
		(void) execlp("dd", "dd", "if=/dev/dvb/adapter0/dvr0",
		    rfarg, NULL);
		/* If we get here, execlp() has failed */
		exit(1);
	}

	/* Parent */
	fclose(lf);
	free(rfname);
	free(rfarg);
	free(lfname);
	dvbs->dd_pid = pid;
	return 0;
error:
	if (dvbs != NULL)
		free(dvbs);
	if (rfname != NULL)
		free(rfname);
	if (rfarg != NULL)
		free(rfarg);
	if (lfname != NULL)
		free(lfname);
	return rc;
}

static int dvb_stop(rsession_t *rs)
{
	dvb_session_t *dvbs = (dvb_session_t *)rs->priv;
	int rc;
	int stat;

	printf("dvb_stop()\n");
	rc = kill(dvbs->zap_pid, SIGTERM);
	if (rc < 0) {
		printf("Failed killing process %u\n", dvbs->zap_pid);
		return EIO;
	}

	rc = waitpid(dvbs->zap_pid, &stat, 0);
	if (rc != dvbs->zap_pid) {
		printf("Failed waiting for process status.\n");
		return EIO;
	}

	printf("dvb_stop()\n");
	rc = kill(dvbs->dd_pid, SIGTERM);
	if (rc < 0) {
		printf("Failed killing process %u\n", dvbs->dd_pid);
		return EIO;
	}

	rc = waitpid(dvbs->dd_pid, &stat, 0);
	if (rc != dvbs->dd_pid) {
		printf("Failed waiting for process status.\n");
		return EIO;
	}

	free(dvbs);
	rs->priv = NULL;
	return 0;
}
