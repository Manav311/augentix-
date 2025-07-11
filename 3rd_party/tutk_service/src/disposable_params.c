#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>
#include "NebulaAPIs.h"
#include "disposable_params.h"

void getRandomStr(char *buf, int length)
{
	char elements[63] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
	int i = 0;
	struct timeval tv;
	gettimeofday(&tv, NULL);

	srand((unsigned int)(tv.tv_usec));
	for (i = 0; i < length; i++) {
		int j = rand() % 62;
		buf[i] = elements[j];
	}
}

int createDisposableParams(char *pin_code, char *secret_id, char *psk)
{
	char tmp_pin_code[DISPOSABLE_PIN_CODE_LENGTH + 1] = { 0 };
	char tmp_secret_id[DISPOSABLE_SECRET_ID_LENGTH + 1] = { 0 };
	char tmp_psk[DISPOSABLE_PSK_LENGTH + 1] = { 0 };

	FILE *fp = fopen(DISPOSABLE_PARAMS_PATH, "w");
	if (fp == NULL) {
		printf("[%s] create file fail\n", __func__);
		return -1;
	}

	printf("===== Create disposable params =====\n");

	getRandomStr(tmp_pin_code, DISPOSABLE_PIN_CODE_LENGTH);
	fprintf(fp, "nebula_pin_code:%s\n", tmp_pin_code);
	printf("nebula_pin_code[%s]\n", tmp_pin_code);

	getRandomStr(tmp_secret_id, DISPOSABLE_SECRET_ID_LENGTH);
	fprintf(fp, "nebula_secret_id:%s\n", tmp_secret_id);
	printf("nebula_secret_id[%s]\n", tmp_secret_id);

	getRandomStr(tmp_psk, DISPOSABLE_PSK_LENGTH);
	fprintf(fp, "nebula_psk:%s\n", tmp_psk);
	printf("nebula_psk[%s]\n", tmp_psk);

	fclose(fp);
	printf("==========\n");

	if (pin_code != NULL) {
		strcpy(pin_code, tmp_pin_code);
	}

	if (secret_id != NULL) {
		strcpy(secret_id, tmp_secret_id);
	}

	if (psk != NULL) {
		strcpy(psk, tmp_psk);
	}

	return 0;
}

int loadDisposableParams(char *pin_code, char *secret_id, char *psk)
{
	char str_line[1024] = { 0 };
	char symbol[64] = { 0 };
	char param[1024] = { 0 };
	FILE *fp = NULL;

	fp = fopen(DISPOSABLE_PARAMS_PATH, "r");
	if (fp == NULL) {
		return createDisposableParams(pin_code, secret_id, psk);
	}
	printf("===== Load disposable params =====\n");

	while (fgets(str_line, 1024, fp) != NULL) {
		sscanf(str_line, "%[^:]:%s", symbol, param);
		if (!strcmp(symbol, "nebula_pin_code")) {
			if (pin_code != NULL) {
				strncpy(pin_code, param, MAX_PIN_CODE_LENGTH);
			}
			printf("%s", str_line);
		} else if (!strcmp(symbol, "nebula_secret_id")) {
			if (secret_id != NULL) {
				strncpy(secret_id, param, MAX_NEBULA_SECRETID_LENGTH);
			}
			printf("%s", str_line);
		} else if (!strcmp(symbol, "nebula_psk")) {
			if (psk != NULL) {
				strncpy(psk, param, MAX_NEBULA_PSK_LENGTH);
			}
			printf("%s", str_line);
		} else {
			// unknow params
		}
	}
	printf("==========\n");
	fclose(fp);
	return 0;
}