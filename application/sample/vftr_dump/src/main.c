#include <stdio.h>
#include "rtsp_shm.h"
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <errno.h>

#include "vftr_dump.h"
#include "rtsp_shm.h"

static int g_iva_pid = 0;

static void handleSigInt(int signo)
{
	if (signo == SIGINT) {
		fprintf(stderr, "Caught SIGINT!\n");
	} else if (signo == SIGTERM) {
		fprintf(stderr, "Caught SIGTERM!\n");
	} else if (signo == SIGKILL) {
		fprintf(stderr, "Caught SIGKILL\n");
	} else if (signo == SIGQUIT) {
		fprintf(stderr, "Caught SIGQUIT!\n");
	} else {
		fprintf(stderr, "Unexpected signal!\n");
	}

	DUMP_stop();
}

void help(const char *name)
{
	printf("Usage: %s pid [options]...\n"
	       "       %s -i file [options]...\n"
	       "       %s sub-command\n"
	       "\n"
	       "Options:\n"
	       "  -i             Read from binary files.\n"
	       "  -B             Print objects in binary format with headers. Ignored if read from binary files.\n"
#ifdef CONFIG_APP_VFTR_DUMP_SUPPORT_SEI
	       "  -s val         Support RTSP SEI out. val = [%s].\n"
	       "  -w chn,win     Target window idx. (Default: 0,0)\n"
#endif
	       "  -g g0,g1...    Show objects with specific group. Maximum support %d elements.\n"
	       "  -n g0,g1...    Show objects without specific group. Maximum support %d elements.\n"
	       "  -d id0,id1...  Show objects with specific id. Maximum support %d elements.\n"
	       "  -t t0,t1...    Show objects with specific type. Maximum support %d elements.\n"
	       "  -c c0,c1...    Show objects with specific category. Maximum support %d elements.\n"
	       "  -p tid         Show objects with specific tid.\n"
	       "  -T second      Interval. Ignored if read from binary files.\n"
	       "  -f path        Image folder path. Image data is ignored if -f is not specified.\n"
	       "\n"
	       "Help sub-commands:\n"
	       "  list           Show all possible ids, types, and categories.\n"
	       "\n"
	       "For example:\n"
	       "  $ vftr_dump list\n"
	       "  $ vftr_dump <PID> -g 4 -T 5 > /tmp/iva_dump.log\n"
	       "  $ vftr_dump <PID> -t 1,4 -T 5 > /tmp/iva_dump.log\n"
	       "  $ vftr_dump <PID> -B -T 5 > /tmp/iva_dump.hex\n"
	       "  $ vftr_dump <PID> -B -s 'hd' > /tmp/iva_dump.hex\n"
	       "  $ vftr_dump <PID> -f /tmp/img_dir > /tmp/iva_dump.log\n"
	       "  $ vftr_dump -i /tmp/iva_dump.hex > /tmp/iva_dump.log\n",
	       name, name, name, RSHM_getAppCbString(), MAX_IVA_DUMP_NUM, MAX_IVA_DUMP_NUM, MAX_IVA_DUMP_NUM,
	       FLAG_LENGTH_TYPE, FLAG_MAX_CAT_VALUE);
}

int parse(int argc, char **argv, DumpConfig *config)
{
	int c;
	char *token;

	if (argc <= 1) {
		help(argv[0]);
		return -1;
	}

	g_iva_pid = atoi(argv[1]);
	if (g_iva_pid <= 20) {
		if (strcmp(argv[1], "list") == 0) {
			DUMP_list();
			return -1;
		} else if (strstr(argv[1], "-i") != NULL) {
			/* This means read from files */
		} else {
			help(argv[0]);
			return -1;
		}
	}

	size_t cnt = 0;
	int chn, win;
	const char *s = ",";
	DIR *dir;

	while ((c = getopt(argc, argv, "i:g:Bd:n:s:t:c:f:p:T:hw:")) != -1) {
		switch (c) {
		case 'i':
			config->in_binary_path = optarg;
			DBG("path = %s\n", config->in_binary_path);
			break;
		case 'g':
			cnt = 0;
			token = strtok(optarg, s);
			while ((token != NULL) && (cnt < MAX_IVA_DUMP_NUM)) {
				DBG(" %s\n", token);
				config->filter.group[cnt] = atoi(token);
				token = strtok(NULL, s);
				++cnt;
			}

			config->filter.group_num = cnt;
			DBG("cnt = %d\n", config->filter.group_num);
			break;
		case 'B':
			config->is_binary = 1;
			DBG("is_binary = %d\n", config->is_binary);
			break;
#ifdef CONFIG_APP_VFTR_DUMP_SUPPORT_SEI
		case 's':
			config->rtsp_app = optarg;
			break;
		case 'w':
			cnt = 0;
			token = strtok(optarg, s);
			chn = atoi(token);
			token = strtok(NULL, s);
			win = atoi(token);
			config->win = MPI_VIDEO_WIN(0, chn, win);
			break;
#endif
		case 'd':
			cnt = 0;
			token = strtok(optarg, s);
			while ((token != NULL) && (cnt < MAX_IVA_DUMP_NUM)) {
				DBG(" %s\n", token);
				config->filter.id[cnt] = atoi(token);
				++cnt;
				token = strtok(NULL, s);
			}

			config->filter.id_num = cnt;
			DBG("cnt = %d\n", config->filter.id_num);
			break;

		case 'f':
			dir = opendir(optarg);
			if (dir) {
				closedir(dir);
				config->img_directory = optarg;
			} else if (ENOENT == errno) {
				fprintf(stderr, "directory does not exist !\n");
				return -1;
			} else {
				fprintf(stderr, "something wrong with directory !\n");
			}

			break;

		case 'n':
			cnt = 0;
			token = strtok(optarg, s);
			while ((token != NULL) && (cnt < MAX_IVA_DUMP_NUM)) {
				DBG(" %s\n", token);
				config->filter.no_group[cnt] = atoi(token);
				token = strtok(NULL, s);
				++cnt;
			}

			config->filter.no_group_num = cnt;
			DBG("cnt = %d\n", config->filter.no_group_num);
			break;
		case 't':
			cnt = 0;
			token = strtok(optarg, s);
			while ((token != NULL) && (cnt < MAX_IVA_DUMP_NUM)) {
				DBG(" %s\n", token);
				config->filter.type[cnt] = atoi(token);
				token = strtok(NULL, s);
				++cnt;
			}

			config->filter.type_num = cnt;
			DBG("cnt = %d\n", config->filter.type_num);
			break;
		case 'c':
			cnt = 0;
			token = strtok(optarg, s);
			while ((token != NULL) && (cnt < MAX_IVA_DUMP_NUM)) {
				DBG(" %s\n", token);
				config->filter.cat[cnt] = atoi(token);
				token = strtok(NULL, s);
				++cnt;
			}

			config->filter.cat_num = cnt;
			DBG("cnt = %d\n", config->filter.cat_num);
			break;
		case 'p':
			config->filter.tid = atoi(optarg);
			break;

		case 'T':
			config->interval_in_sec = atof(optarg);
			DBG("interval = %.1f\n", config->interval_in_sec);
			break;
		case 'h':
		default:
			help(argv[0]);
			return -1;
		}
	}

	if (config->in_binary_path) {
		config->is_binary = 0;
		config->interval_in_sec = 0;
	}

	return 0;
}

int main(int argc, char **argv)
{
	DumpConfig config = { 0 };

	if (signal(SIGINT, handleSigInt) == SIG_ERR) {
		perror("Cannot handle SIGINT!\n");
		return 1;
	}

	if (parse(argc, argv, &config) == -1) {
		return 0;
	}

#ifdef CONFIG_APP_VFTR_DUMP_SUPPORT_SEI
	// init avftr serv
	if (config.rtsp_app) {
		int ret = RSHM_init(config.rtsp_app, config.win);
		if (ret) {
			fprintf(stderr, "[ERROR] init avftr server fail\n");
			return -EINVAL;
		}
	}
#endif

	/* start dumping */
	DUMP_start(g_iva_pid, &config);

#ifdef CONFIG_APP_VFTR_DUMP_SUPPORT_SEI
	// exit avftr serv
	if (config.rtsp_app) {
		RSHM_exit();
	}
#endif

	return 0;
}
