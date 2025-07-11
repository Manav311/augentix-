#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "ledevt.h"
#include "agtx_types.h"

int main(int argc, char **argv)
{
	AGTX_UNUSED(argc);

	char *client;
	int enabled = 0;

	if (atoi(argv[1]) < 0) {
		return -1;
	} else {
		client = argv[1];
		enabled = atoi(argv[2]);
		if (enabled >= 0) {
			setLEDInform(client, enabled);
		} else {
			printf(" /system/bin/ledapp [LED_Client] [LED_enabled]\n");
		}
	}
	return 0;
}
