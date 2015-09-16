#include <errno.h>
#include <rsession.h>
#include <stdlib.h>

void rsessions_init(rsessions_t *rss)
{
	list_initialize(&rss->rsess);
}

int rsession_create(rsessions_t *rss, revent_t *start_ev, rsession_t **rrs)
{
	rsession_t *rs;

	rs = calloc(1, sizeof(rsession_t));
	if (rs == NULL)
		return ENOMEM;

	rs->preset = start_ev->preset;
	rs->nst = start_ev->nst;

	list_append(&rs->lrsess, &rss->rsess);
	*rrs = rs;
	return 0;
}

rsession_t *rsession_find(rsessions_t *rss, revent_t *stop_ev)
{
	list_foreach(rss->rsess, lrsess, rsession_t, rs) {
		if (rs->preset == stop_ev->preset &&
		    rs->nst == stop_ev->nst) {
			    /* Match */
			    return rs;
		}
	}

	return NULL;
}

void rsession_destroy(rsession_t *rs)
{
	list_remove(&rs->lrsess);
	free(rs);
}
