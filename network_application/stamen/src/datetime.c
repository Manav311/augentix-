#include "datetime.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <unistd.h>

int set_date_time(const char *datetime)
{
	// Only allow DateTime format: YYYY.MM.DD-hh:mm:ss
	if (strlen(datetime) != 19 || // total length should be 19
	    !isdigit(datetime[0]) || !isdigit(datetime[1]) || !isdigit(datetime[2]) || !isdigit(datetime[3]) || // Year
	    datetime[4] != '.' || !isdigit(datetime[5]) || !isdigit(datetime[6]) || // Month
	    datetime[7] != '.' || !isdigit(datetime[8]) || !isdigit(datetime[9]) || // Day of Month
	    datetime[10] != '-' || !isdigit(datetime[11]) || !isdigit(datetime[12]) || // Hour
	    datetime[13] != ':' || !isdigit(datetime[14]) || !isdigit(datetime[15]) || // Minute
	    datetime[16] != ':' || !isdigit(datetime[17]) || !isdigit(datetime[18])) { // Second
		return -1;
	}

	char cmd[28] = { 0 };
	sprintf(cmd, "date -s %s", datetime);
	int err = system(cmd);
	if (err != 0) {
		syslog(LOG_ERR, "Cannot set DateTime.");
	}

	return err;
}
