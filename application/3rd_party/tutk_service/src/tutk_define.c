#include "tutk_define.h"
#include <stdbool.h>

extern unsigned int gPushNotificationAbort;
extern NebulaDeviceCtx *gDeviceCtx;

inline void TUTK_runSystemCmdWithRetry(char *excmd)
{
	pid_t status;
	int isOK = -1;
	int retry = 3;

	do {
		status = system(excmd);

		if (-1 == status) {
			isOK = -1;
			tutkservice_log_info("system error!");
		} else {
			tutkservice_log_info("exit status value = [0x%x]\n", status);

			if (WIFEXITED(status)) // shell is executed
			{
				if (0 == WEXITSTATUS(status)) //shell executed successfully
				{
					isOK = 0;
					tutkservice_log_info("run shell script successfully.\n");
				} else {
					//shell executed fail
					isOK = -1;
					tutkservice_log_info("run shell script fail, script exit code: %d\n",
					                     WEXITSTATUS(status));
				}
			} else {
				//shell is non-executed
				isOK = -1;
				tutkservice_log_info("exit status = [%d]\n", WEXITSTATUS(status));
			}
		}

		if (isOK == 0) {
			retry = 0;
		} else {
			retry--;
		}
	} while (retry != 0);
}

inline int TUTK_exeSystemCmd(char *excmd)
{
	FILE *pf = NULL;
	char buff[255];
	int ret = 0;

	pf = popen(excmd, "r");
	if (pf == NULL) {
		fprintf(stderr, "Failed to execute command:\n %s\n", excmd);
		return ret;
	}
	while (!feof(pf)) {
		if (fgets(buff, sizeof(buff), pf) != NULL) {
			printf("%s", buff);
		}
	}
	ret = pclose(pf);

	return ret;
}

inline int TUTK_forkIndependentProc(char *prog, char **arg_list)
{
	pid_t child;

	if ((child = fork()) < 0) {
		/* parent: check if fork failed */
		//tutkservice_log_err("fork error");
	} else if (child == 0) {
		/* 1st level child: fork again */
		if ((child = fork()) < 0) {
			//tutkservice_log_err("fork error");
		} else if (child > 0) {
			/* 1st level child: terminate itself to make init process the parent of 2nd level child */
			exit(0);
		} else {
			/* 2nd level child: execute program and will become child of init once 1st level child exits */
			execvp(prog, arg_list);
			//tutkservice_log_err("execvp error");
			exit(0);
		}
	}

	/* parent: wait for 1st level child ends */
	waitpid(child, NULL, 0);

	return child;
}

//########################################################
//# Start Nebula Device Push Notification
//########################################################
int nebula_device_push_notification(const char *push_data, int event)
{
	int ret;

	FILE *fp;
	//get notification settings
	fp = fopen(DEFAULT_DEVICE_SETTING, "r");
	if (fp == NULL) {
		tutkservice_log_err("device_settings.txt not exist\n");
		return -1;
	}
	fseek(fp, 0, SEEK_END);
	long fsize = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	char *settings = malloc(fsize + 1);
	if (settings == NULL) {
		tutkservice_log_err("fail to malloc buffer for notification settings\n");
		return -1;
	}

	fread(settings, 1, fsize, fp);
	fclose(fp);
	fp = NULL;
	settings[fsize] = 0;

	ret = Nebula_Device_Load_Settings(gDeviceCtx, settings);
	free(settings);
	if (ret == NEBULA_ER_NoERROR) {
		printf("Nebula_Device_Load_Settings------ok event %d\n", event);
	} else {
		tutkservice_log_err("Nebula_Device_Load_Settings ret[%d]------fail\n", ret);
		return -1;
	}

	char push_notify[64];
	sprintf(push_notify, "{\"event\":\"%d\"}", event);
	NebulaJsonObject *notification_obj = NULL;
	if (push_data)
		ret = Nebula_Json_Obj_Create_From_String(push_data, &notification_obj);
	else
		ret = Nebula_Json_Obj_Create_From_String(push_notify, &notification_obj);
	if (ret != NEBULA_ER_NoERROR) {
		tutkservice_log_err("Create notification_obj ret[%d]------fail\n", ret);
		return -1;
	}

	ret = Nebula_Device_Push_Notification(gDeviceCtx, notification_obj, 10000, &gPushNotificationAbort);
	if (ret != NEBULA_ER_NoERROR) {
		tutkservice_log_err("Nebula_Device_Push_Notification ret[%d]------fail\n", ret);
		return -1;
	}
	return 0;
}