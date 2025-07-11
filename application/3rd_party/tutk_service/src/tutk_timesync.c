#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

#include "tutk_define.h"
#include "log_define.h"

#define cmd_time_sync_start "/etc/init.d/factory/S50sntp start"

static char tzone[64];
static char enable = -1;
extern int gTime_;

int TUTK_setTimeZone(const char *tz)
{
	printf("%s\n", __func__);

	if (tz != NULL && strlen(tz) && strlen(tz) < 64 - 1) {
		strncpy(tzone, tz, strlen(tz));
		tzone[63] = '\0';
		enable = 0;
	}
	return enable;
}

int TUTK_timeSync(void)
{
	char buf[128];
	int retry = 3;
	if (enable == 0) {
		while (retry--) {
			sprintf(buf, "echo %s > /etc/TZ", tzone);
			TUTK_exeSystemCmd(buf);
			TUTK_exeSystemCmd(cmd_time_sync_start);
			/* check time*/
			time_t timep;
			struct tm *p;
			time(&timep);
			gTime_ = (int)time(NULL);
			p = localtime(&timep);
			if ((p->tm_year + 1900) > 1970) {
				break;
			}
		}
		if (retry <= 0) {
			tutkservice_log_err("TUTK_timesync retry 3 times failed");
			return -1;
		}
	}
	return 0;
}

