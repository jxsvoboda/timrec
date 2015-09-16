#include <errno.h>
#include <rclass/wget.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

static int wget_start(char *, char *, rsession_t *);
static int wget_stop(rsession_t *);

rclass_t rclass_wget = {
	wget_start,
	wget_stop
};

#include <stdio.h>
static int wget_start(char *src, char *recname, rsession_t *rs)
{
	pid_t pid;
	char *rfname = NULL;
	char *lfname = NULL;
	wget_session_t *ws = NULL;
	FILE *lf;
	int rc;

	printf("wget_start('%s', '%s', rrs)\n", src, recname);

	ws = calloc(1, sizeof(wget_session_t));
	if (rs == NULL) {
		printf("Out of memory.\n");
		rc = ENOMEM;
		goto error;
	}

	rs->priv = ws;

	rc = asprintf(&rfname, "/var/opt/timrec/%s", recname);
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

	lf = fopen(lfname, "wt");
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

		(void) execlp("wget", "wget", "-O", rfname, src, NULL);
		/* If we get here, execlp() has failed */
		exit(1);
	}

	/* Parent */
	fclose(lf);
	ws->pid = pid;
	return 0;
error:
	free(ws);
	free(rfname);
	return rc;
}

static int wget_stop(rsession_t *rs)
{
	wget_session_t *ws = (wget_session_t *)rs->priv;
	int rc;
	int stat;

	printf("wget_stop()\n");
	rc = kill(ws->pid, SIGINT);
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

