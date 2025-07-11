#define _GNU_SOURCE

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "secure_openssl_demo.h"
#include "log.h"

#ifdef DEBUG
#define DBG(...) printf(__VA_ARGS__)
#else
#define DBG(...)
#endif

/**
 * @brief help messages
 * @details
 * @param NULL
 * @return NULL
 * @retval NULL
 * @see
 */
static void help(const char *program_name)
{
	printf("USAGE: %s [options] ...\n"
	       "Options:\n"
	       " -D <I2C bus ID> 		I2C bus ID. (Default = 0)\n"
	       " -H <Host name>			Host IP.\n"
	       " -P <Port no.>			Host port no.\n"
	       " -C <Client certificate> 	Client certificate.\n"
	       " -K <Client private key>	Client private key.\n"
	       " -R <Root CA certificate> 	Root CA certificate.\n"
	       "\n"
	       "Example:\n"
	       " $ %s -D 1 -H 123.321.00.000 -P 1234 -C client.crt -K client.key -R rootCA.crt\n",
	       program_name, program_name);
}

int main(int argc, char **argv)
{
	int ret = 0;
	int opt = 0;
	int i2c_bus_id = 0;
	int port = 0;
	const char *hostname = NULL;
	const char *cert_file = NULL;
	const char *key_file = NULL;
	const char *ca_file = NULL;

	/* Get parameters */
	while ((opt = getopt(argc, argv, "D:H:P:C:K:R:h")) != -1) {
		switch (opt) {
		case 'D':
			i2c_bus_id = atoi(optarg);
			break;
		case 'H':
			hostname = optarg;
			break;
		case 'P':
			port = atoi(optarg);
			break;
		case 'C':
			cert_file = optarg;
			break;
		case 'K':
			key_file = optarg;
			break;
		case 'R':
			ca_file = optarg;
			break;
		case 'h':
			help(argv[0]);
			return EXIT_SUCCESS;
		default:
			help(argv[0]);
			return EXIT_FAILURE;
		}
	}

	/* Execute demonstration */
	printf("[%s] L%d Secure OpenSSL demo start.\n", __func__, __LINE__);

	ret = SEOPENSSL_executeDemo(i2c_bus_id, hostname, port, cert_file, key_file, ca_file);

	if (ret) {
		log_err("Secure OpenSSL demo error.\n");
	}

	printf("[%s] L%d Secure OpenSSL demo end.\n", __func__, __LINE__);

	return 0;
}
