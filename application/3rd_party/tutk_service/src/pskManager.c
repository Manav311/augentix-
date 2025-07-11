#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "NebulaDevice.h"
#include "disposable_params.h"
#include "Nebula_User_PSK_Manager/pskManager.h"
#include "log_define.h"

//########################################################
//# Credential management
//########################################################
int GetPskFromFile(const char *identity, const char *file_path, char *psk_buf, int buf_size)
{
	char line_buf[MAX_NEBULA_IDENTITY_LENGTH + MAX_NEBULA_PSK_LENGTH + 2] = { 0 };
	char tmp_identity[MAX_NEBULA_IDENTITY_LENGTH + 1] = { 0 };
	char tmp_psk[MAX_NEBULA_PSK_LENGTH + 1] = { 0 };
	FILE *file_ptr = fopen(file_path, "r+");
	if (file_ptr == NULL) {
		tutkservice_log_err("[%s] open %s error, return 500(Internal Server Error)\n", __func__, file_path);
		return 500;
	}

	while (fgets(line_buf, sizeof(line_buf), file_ptr)) {
		// read line & find target identity
		sscanf(line_buf, "%[^:]:%s", tmp_identity, tmp_psk);
		if (strcmp(tmp_identity, identity) == 0) {
			if (strlen(tmp_psk) > (unsigned int)buf_size) {
				tutkservice_log_err("[%s] psk buf is too small, return 400(Bad Request)\n", __func__);
				fclose(file_ptr);
				return 400;
			}
			strcpy(psk_buf, tmp_psk);
			break;
		}
		memset(line_buf, 0, sizeof(line_buf));
	}
	fclose(file_ptr);
	return 200;
}

int AppendPskToFile(const char *identity, const char *file_path, char *psk_buf, int buf_size)
{
	FILE *file_ptr = fopen(file_path, "a+");
	if (file_ptr == NULL) {
		tutkservice_log_err("[%s] open %s error, return 500(Internal Server Error)\n", __func__, file_path);
		return 500;
	}

	if (DISPOSABLE_PSK_LENGTH + 1 > buf_size) {
		tutkservice_log_err("[%s] psk buf is too small, return 400(Bad Request)\n", __func__);
		fclose(file_ptr);
		return 400;
	}

	getRandomStr(psk_buf, DISPOSABLE_PSK_LENGTH);
	fprintf(file_ptr, "\n%s:%s", identity, psk_buf);
	fclose(file_ptr);

	return 200;
}

static int DeletePskfromFile(const char *identity, const char *file_path)
{
	char tmp_file_path[] = "./tmp_identity";
	char line_buf[MAX_NEBULA_IDENTITY_LENGTH + MAX_NEBULA_PSK_LENGTH + 2] = { 0 };
	char tmp_identity[MAX_NEBULA_IDENTITY_LENGTH + 1] = { 0 };
	FILE *origin_file_ptr = fopen(file_path, "r");
	if (origin_file_ptr == NULL) {
		tutkservice_log_err("[%s] open %s error, return 500(Internal Server Error)\n", __func__, file_path);
		return 500;
	}

	FILE *tmp_file_ptr = fopen(tmp_file_path, "w");
	if (origin_file_ptr == NULL) {
		tutkservice_log_err("[%s] create tmp file error, return 500(Internal Server Error)\n", __func__);
		fclose(origin_file_ptr);
		return 500;
	}

	while (fgets(line_buf, sizeof(line_buf), origin_file_ptr)) {
		sscanf(line_buf, "%[^:]", tmp_identity);
		if (strcmp(tmp_identity, identity) != 0) {
			fprintf(tmp_file_ptr, "%s", line_buf);
		}
		memset(line_buf, 0, sizeof(line_buf));
	}

	fclose(origin_file_ptr);
	fclose(tmp_file_ptr);

	remove(file_path);
	rename(tmp_file_path, file_path);

	return 200;
}

int GetAllIdentitiesFromFile(const char *file_path, char **identities_json_str)
{
	char line_buf[MAX_NEBULA_IDENTITY_LENGTH + MAX_NEBULA_PSK_LENGTH + 2] = { 0 };
	char *identities_str = calloc(1, MAX_IDENTITIES_LIST_SIZE + 1);
	char tmp_identity[MAX_NEBULA_IDENTITY_LENGTH + 1] = { 0 };
	FILE *file_ptr = fopen(file_path, "r+");
	if (file_ptr == NULL) {
		tutkservice_log_err("[%s] open %s error!, return 500(Internal Server Error)\n", __func__, file_path);
		free(identities_str);
		return 500;
	}

	strcpy(identities_str, "{\"identities\":[\"admin\",");

	while (fgets(line_buf, sizeof(line_buf), file_ptr)) {
		// read line & find target identity
		sscanf(line_buf, "%[^:]", tmp_identity);
		// 3 = '\"' + '\"' + ','   2 = ']' + '}'
		if (strlen(identities_str) + strlen(tmp_identity) + 3 < MAX_IDENTITIES_LIST_SIZE - 2) {
			strcat(identities_str, "\"");
			strcat(identities_str, tmp_identity);
			strcat(identities_str, "\",");
		} else {
			tutkservice_log_err("[%s] buf is too small, not list all identlities\n", __func__);
			break;
		}
		memset(line_buf, 0, sizeof(line_buf));
	}

	fclose(file_ptr);

	identities_str[strlen(identities_str) - 1] = ']'; // replace last ','
	identities_str[strlen(identities_str)] = '}';
	//Nebula_Json_Obj_Create_From_String(identities_str, identities_json);
	*identities_json_str = identities_str;

	return 200;
}

int GetCredential(const char *udid, const char *secret_id, const char *identity, const char *mode,
                  const char *file_path, char **credential)
{
	int ret = 0;
	char tmp_psk[MAX_NEBULA_PSK_LENGTH + 1] = { 0 };
	char *tmp_credential = NULL;

	if (strcmp(mode, "createIfNotExist") == 0) {
		ret = GetPskFromFile(identity, file_path, tmp_psk, MAX_NEBULA_PSK_LENGTH);
		if (ret != 200) {
			ret = AppendPskToFile(identity, file_path, tmp_psk, MAX_NEBULA_PSK_LENGTH);
			if (ret != 200) {
				tutkservice_log_err("[%s] create psk fail, return 500(Internal Server Error)\n",
				                    __func__);
				return 500;
			}
		}
	} else if (strcmp(mode, "updateIfExist") == 0) {
		ret = GetPskFromFile(identity, file_path, tmp_psk, MAX_NEBULA_PSK_LENGTH);
		if (ret == 200) {
			DeletePskfromFile(identity, file_path);
			AppendPskToFile(identity, file_path, tmp_psk, MAX_NEBULA_PSK_LENGTH);
		} else {
			tutkservice_log_err("[%s] cant find identity, return 404(Not Found)\n", __func__);
			return 404;
		}
	} else {
		tutkservice_log_err("[%s] unknow createMode: %s ,return 400\n", __func__, mode);
		return 400;
	}

	ret = Nebula_Device_New_Credential(udid, identity, tmp_psk, secret_id, &tmp_credential);
	if (ret != NEBULA_ER_NoERROR) {
		tutkservice_log_err("[%s] Nebula_Device_New_Credential()=%d, return 500(Internal Server Error)\n",
		                    __func__, ret);
		return 500;
	}
	*credential = tmp_credential;

	return 200;
}

int DeleteCredential(const char *identity, const char *file_path)
{
	char tmp_psk[MAX_NEBULA_PSK_LENGTH + 1] = { 0 };
	int ret = GetPskFromFile(identity, file_path, tmp_psk, MAX_NEBULA_PSK_LENGTH);
	if (ret == 200) {
		DeletePskfromFile(identity, file_path);
	} else {
		tutkservice_log_err("[%s] cant find identity, return 404(Not Found)\n", __func__);
		return 404;
	}
	return 200;
}
