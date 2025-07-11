#include "ampi.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void get_random_data(char *d, int len)
{
	int i;
	for (i = 0; i < len; i++) {
		d[i] = rand() & 0xFF;
	}
}

void show_data(char *d, int len)
{
	int i;
	for (i = 0; i < len; i++) {
		printf("%x, ", d[i]);
		if (i % 16 == 15)
			printf("\n");
	}
}

int main(void)
{
	ampi_dev _dev;
	ampi_svc _svc;
	int send_len;
	int rcv_len;
	int ma_len;
	char send_data[512];
	char rvc_data[512];
	int i;
	int ret = 0;
	char *tmp = NULL;

	_dev = ampi_init(0);
	if (_dev == AMPI_FAILURE) {
		printf("ampi_init fail\n");
		return -1;
	}
	_svc = ampi_link_service(_dev, "tx_check", 10000);
	if (_svc == AMPI_FAILURE) {
		printf("link fail\n");
		ampi_deinit(_dev);
		return -1;
	}

	for (i = 0; i < 10000; i++) {
		//send_len = 1 + rand() % 480;
		send_len = (i & 0xFF) + 1; //1~256
		printf("-%d-test %d -- %d byte\n", (int)getpid(), i, send_len);

		memset(send_data, send_len & 0xff, send_len);
		//get_random_data(send_data, send_len);

		ma_len = 1 + rand() % 512;
		tmp = ampi_malloc(_dev, ma_len);
		if (tmp == NULL) {
			printf("AMPI_MALLOC fail\n");
			ret = -1;
			break;
		}
		memset(tmp, send_len, ma_len);

		if (AMPI_FAILURE == ampi_send(_svc, send_data, send_len, 300)) {
			printf("send %d byte fail\n", send_len);
			ret = -1;
			break;
		}

		rcv_len = ampi_receive(_svc, rvc_data, send_len, 10000);
		if (AMPI_FAILURE == rcv_len) {
			printf("receive %d byte fail\n", send_len);
			ret = -1;
			break;
		}
		if (rcv_len != send_len)
			printf("Incorrect length, send(%d), receive(%d)\n", send_len, rcv_len);

		if (memcmp(send_data, rvc_data, send_len)) {
			printf("Incorrect response\n");
			printf("===============================\n");
			show_data(send_data, send_len);
			printf("===============================\n");
			show_data(rvc_data, send_len);
			printf("===============================\n");
			ret = -1;
			break;
		}

		ampi_free(_dev, tmp);
		tmp = NULL;
	}
	if (tmp)
		ampi_free(_dev, tmp);
	ampi_unlink_service(_svc);
	ampi_deinit(_dev);
	return ret;
}
