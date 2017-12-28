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

/*
 * Wget recording class
 *
 * Record a web stream using wget
 *
 * param: Stream URL
 */
 
#include <errno.h>
#include <rclass/wget.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <source.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <stdio.h>
#include <unistd.h>

static int wget_start(source_t *, char *, rsession_t *);
static int wget_stop(rsession_t *);

rclass_t rclass_wget = {
	wget_start,
	wget_stop
};

static int wget_start(source_t *source, char *recname, rsession_t *rs)
{
	pid_t pid;
	char *rfname = NULL;
	char *lfname = NULL;
	wget_session_t *ws = NULL;
	struct stat sbuf;
	int serno;
	char *url;
	FILE *lf = NULL;
	int rc;

	url = source->param;
	printf("wget url=%s\n", url);

	ws = calloc(1, sizeof(wget_session_t));
	if (rs == NULL) {
		printf("Out of memory.\n");
		rc = ENOMEM;
		goto error;
	}

	rs->priv = ws;

	/* XXX Extension should be provided by recording source */
	rc = asprintf(&rfname, "/var/opt/timrec/%s.ogg", recname);
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
			rc = asprintf(&rfname, "/var/opt/timrec/%s.%d.ogg", recname,
			    serno++);
			if (rc < 0) {
				printf("Out of memory.\n");
				rc = ENOMEM;
				goto error;
			}

			rc = stat(rfname, &sbuf);
		}
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

		(void) execlp("wget", "wget", "-O", rfname, url, NULL);
		/* If we get here, execlp() has failed */
		exit(1);
	}

	/* Parent */
	free(rfname);
	free(lfname);
	fclose(lf);
	ws->pid = pid;
	return 0;
error:
	free(ws);
	if (rfname != NULL)
		free(rfname);
	if (lfname != NULL)
		free(lfname);
	if (lf != NULL)
		fclose(lf);
	return rc;
}

static int wget_stop(rsession_t *rs)
{
	wget_session_t *ws = (wget_session_t *)rs->priv;
	int rc;
	int stat;

	printf("wget_stop()\n");
	rc = kill(ws->pid, SIGTERM);
	if (rc < 0) {
		printf("Failed killing process %u\n", ws->pid);
		return EIO;
	}

	rc = waitpid(ws->pid, &stat, 0);
	if (rc != ws->pid) {
		printf("Failed waiting for process status.\n");
		return EIO;
	}

	free(ws);
	rs->priv = NULL;
	return 0;
}

