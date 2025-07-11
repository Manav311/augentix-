#include <stdio.h>
#include <stdint.h>
#include <unistd.h>

#include "spi_dbg.h"

#define EVENT_ID 0x01
#define DATA_ID 0x02

int main(void)
{
	int i = 0;
	int j = 0;

	uint8_t data[2] = { 0x55, 0xAA };

	for (; i < 3; i++) {
		spi_dbg_log_event(EVENT_ID);
		usleep(1000);
	}

	for (; j < 3; j++) {
		spi_dbg_log_data(DATA_ID, (sizeof(data) / sizeof(data[0])), data);
		usleep(1000);
	}
}
