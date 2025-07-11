#include <stdio.h>
#include <stdint.h>
#include <unistd.h>

#include "uart_dbg.h"

int main(void)
{
	int i = 0;

	uint8_t event_id = 1;
	uint8_t data_id = 1;
	uint8_t data[] = { 0x99, 0xAA, 0xBB, 0xCC };

	for (i = 0; i < 1000; i++) {
		uart_dbg_log_event(event_id);
		if (event_id < 127) {
			event_id += 0x1;
		}
		uart_dbg_log_data(data_id, sizeof(data) / sizeof(data[0]), data);
		if (data_id < 31) {
			data_id += 0x1;
		}
	}
}
