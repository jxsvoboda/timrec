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
	wget_session_t *ws = NULL;
	int rc;

	printf("wget_start('%s', '%s', rrs)\n", src, recname);

	ws = calloc(1, sizeof(wget_session_t));
	if (rs == NULL) {
		printf("Out of memory.\n");
		rc = ENOMEM;
		goto error;
	}

	rs->priv = ws;

	rc = asprintf(&rfname, "/tmp/%s.ogg", recname);
	if (rc < 0) {
		printf("Out of memory.\n");
		rc = ENOMEM;
		goto error;
	}

	pid = fork();
	if (pid < 0) {
		printf("Error forking.\n");
		rc = ENOMEM;
		goto error;
	} else if (pid == 0) {
		/* Child */
		(void) execlp("wget", "wget", "-O", rfname, src, NULL);
		/* If we get here, execlp() has failed */
		exit(1);
	}

	printf("child pid=%u\n", pid);
	/* Parent */
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

