#define _GNU_SOURCE

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/timeb.h>

#include "lpw_fw_upg.h"
#include "log.h"

//#define DEBUG

#ifdef DEBUG
#define DBG(...) printf(__VA_ARGS__)
#else
#define DBG(...)
#endif

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/**
 * @brief help messages
 * @details
 * @param 
 * @return 
 * @retval
 * @see
 */
static void help(const char *program_name)
{
	printf("USAGE: %s -i [options] ...\n"
	       "Options:\n"
	       " -i <new firmware upgrade file>	Firmware upgrade file.\n"
	       "\n"
	       "Example:\n"
	       " $ %s -i /etc/fw_upg.bin\n",
	       program_name, program_name);
}

int main(int argc, char **argv)
{
	log_info("LPW firmware upgrade start ...\n");

	/* initlize variables */
	int opt;
	int ret = 0;
	unsigned char *new_fw_path = NULL;
	struct timeb s_time;
	struct timeb e_time;

	/* get start time */
	ftime(&s_time);

	/* get parameters */
	while ((opt = getopt(argc, argv, "i:h")) != -1) {
		switch (opt) {
		case 'i':
			new_fw_path = (unsigned char *)optarg;
			if (new_fw_path == NULL) {
				log_err("new firmware file path is null.\n");
				return EXIT_FAILURE;
			}
			log_info("new_fw_path: %s\n", new_fw_path);
			break;
		case 'h':
			help(argv[0]);
			return EXIT_SUCCESS;
		default:
			help(argv[0]);
			return EXIT_FAILURE;
		}
	}

	/* start firmware upgrade */
	ret = LPW_appUpgFw(new_fw_path);
	if (ret != 0) {
		log_err("APP fails to upgrade firmware. Error no: %d.\n", ret);
		return EXIT_FAILURE;
	}

	/* get end time */
	ftime(&e_time);
	log_info("LPW firmware upgrade done. Using %ld ms.\n",
	         ((e_time.time - s_time.time) * 1000 + (e_time.millitm - s_time.millitm)));

	return 0;
}
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
