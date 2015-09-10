#include <startspec.h>
#include <time.h>

time_t startspec_date_tod_ts(ds_date_t *date, tod_t *tod)
{
	struct tm t;

	t.tm_sec = 0;
	t.tm_min = tod->m;
	t.tm_hour = tod->h;
	t.tm_mday = date->d;
	t.tm_mon = date->m - 1;
	t.tm_year = date->y - 1900;
	t.tm_isdst = -1;

        return mktime(&t);
}

