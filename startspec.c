#include <startspec.h>
#include <time.h>

#define MONTHS_IN_YEAR 12

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

int startspec_days_in_month(int year, int month)
{
	bool isleap;

	isleap = (year % 4) == 0;
	if ((year % 100) == 0) isleap = false;
	if ((year % 400) == 0) isleap = true;

	switch (month) {
	case 1: return 31;
	case 2: return 28 + isleap;
	case 3: return 31;
	case 4: return 30;
	case 5: return 31;
	case 6: return 30;
	case 7: return 31;
	case 8: return 31;
	case 9: return 30;
	case 10: return 31;
	case 11: return 30;
	case 12: return 31;
	}

	return 0;
}

/** Previous day */
void startspec_date_decr(ds_date_t *date)
{
	if (date->d > 1) {
		--date->d;
	} else {
		if (date->m > 1) {
			--date->m;
		} else {
			--date->y;
			date->m = MONTHS_IN_YEAR;
		}

		date->d = startspec_days_in_month(date->y, date->m);
	}
}

/** Next day */
void startspec_date_incr(ds_date_t *date)
{
	if (date->d < startspec_days_in_month(date->y, date->m)) {
		++date->d;
	} else {
		date->d = 1;

		if (date->m < MONTHS_IN_YEAR) {
			++date->m;
		} else {
			date->m = 1;
			++date->y;
		}
	}
}

int startspec_date_get_dow(ds_date_t *date)
{
	struct tm tm;
	time_t t;
	tod_t tod;

	tod.h = tod.m = 0;

	/* Go to timestamp (at 00:00) and back to get DOW */
	t = startspec_date_tod_ts(date, &tod);
	(void) localtime_r(&t, &tm);

	/* Convert from Sunday-start of week to Monday start of week */
	if (tm.tm_wday == 0) /* sunday */
		return 6;
	else
		return tm.tm_wday - 1;
}
